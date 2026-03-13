////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptDebugger.cpp
//
// Copyright 2010 Noesis Innovation Inc. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <v8-inspector.h>
#include <vcclr.h>
#include <string>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#include "JavascriptDebugger.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////
// String conversion helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

static System::String^ StringViewToManagedString(const v8_inspector::StringView& view)
{
    if (view.length() == 0)
        return System::String::Empty;

    if (view.is8Bit())
    {
        // V8 inspector 8-bit strings are UTF-8 encoded (CDP JSON is ASCII-safe)
        return System::Text::Encoding::UTF8->GetString((System::Byte*)view.characters8(), (int)view.length());
    }
    else
    {
        // 16-bit UTF-16
        return gcnew System::String((const wchar_t*)view.characters16(), 0, (int)view.length());
    }
}

static std::string ManagedStringToUtf8(System::String^ str)
{
    cli::array<System::Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(str);
    std::string result(bytes->Length, '\0');
    if (bytes->Length > 0)
    {
        pin_ptr<System::Byte> pinned = &bytes[0];
        memcpy(result.data(), pinned, bytes->Length);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// InspectorImpl
//
// Native struct that implements both V8InspectorClient (V8 -> us callbacks) and
// V8Inspector::Channel (V8 -> us protocol messages).
//
// Threading model:
//   - V8 callbacks (runMessageLoopOnPause, sendResponse, sendNotification) are
//     always called on the V8 thread (the thread holding the V8 lock).
//   - EnqueueCommand() is thread-safe and may be called from any thread.
//   - ProcessPending() must be called on the V8 thread.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct InspectorImpl : v8_inspector::V8InspectorClient, v8_inspector::V8Inspector::Channel
{
    static const int kContextGroupId = 1;

    v8::Isolate* isolate;
    v8::Persistent<v8::Context>* contextPersistent;
    std::unique_ptr<v8_inspector::V8Inspector> inspector;
    std::unique_ptr<v8_inspector::V8InspectorSession> session;

    // Command queue: commands enqueued from any thread, dispatched on V8 thread.
    std::mutex queueMutex;
    std::condition_variable cv;
    std::deque<std::string> pendingCommands;
    std::atomic<bool> paused;
    std::atomic<bool> waitingForDebugger;
    std::atomic<bool> disconnecting;

    // GC root keeping the managed JavascriptDebugger alive while this struct exists.
    gcroot<JavascriptDebugger^> managedDebugger;

    InspectorImpl(v8::Isolate* iso, v8::Persistent<v8::Context>* ctx, bool waitForDebugger)
        : isolate(iso)
        , contextPersistent(ctx)
        , paused(waitForDebugger)
        , waitingForDebugger(waitForDebugger)
        , disconnecting(false)
    {
        inspector = v8_inspector::V8Inspector::create(isolate, this);
    }

    ~InspectorImpl()
    {
        // If still connected, perform a minimal disconnect without V8 access
        // (safe path when called from JavascriptDebugger destructor after
        // OnContextDisposing has already done V8 cleanup).
        SignalStop(true);
    }

    // Connect the inspector session. Must be called with the V8 lock held.
    void Connect()
    {
        v8::HandleScope scope(isolate);

        v8_inspector::StringView emptyState;
        session = inspector->connect(
            kContextGroupId,
            this,
            emptyState,
            v8_inspector::V8Inspector::kFullyTrusted,
            waitingForDebugger ? v8_inspector::V8Inspector::kWaitingForDebugger : v8_inspector::V8Inspector::kNotWaitingForDebugger);

        v8::Local<v8::Context> ctx = contextPersistent->Get(isolate);
        const uint8_t name[] = "JavaScript.Net";
        v8_inspector::StringView contextName(name, sizeof(name) - 1);
        inspector->contextCreated(v8_inspector::V8ContextInfo(ctx, kContextGroupId, contextName));
    }

    // Disconnect the inspector session. Must be called with the V8 lock held.
    // Resets both session and inspector to null while the lock is held, so
    // their V8 destructors never run without the lock (e.g. from ~InspectorImpl
    // via delete mImpl after the JavascriptScope has already released the lock).
    void Disconnect()
    {
        if (!inspector) return;  // Already disconnected

        // Unblock any thread waiting in runMessageLoopOnPause.
        SignalStop(true);

        if (session)
        {
            session->stop();
            session.reset();
        }

        v8::HandleScope scope(isolate);
        v8::Local<v8::Context> ctx = contextPersistent->Get(isolate);
        inspector->contextDestroyed(ctx);
        inspector.reset();  // Must be reset here, with the V8 lock held.
                            // The unique_ptr destructor in ~InspectorImpl would
                            // otherwise call ~V8Inspector() without the lock.
    }

    // Signal the pause loop to exit without V8 operations.
    // Pass isDisconnecting=true when tearing down so that runMessageLoopOnPause
    // will not re-enter after returning. Use the default (false) for transient
    // unblocks such as runIfWaitingForDebugger.
    // Safe to call without the V8 lock.
    void SignalStop(bool isDisconnecting = false)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (isDisconnecting) disconnecting = true;
        paused = false;
        waitingForDebugger = false;
        cv.notify_all();
    }

    bool IsConnected() const { return session != nullptr; }

    bool IsPaused() const { return paused; }

    // Enqueue a command for dispatch on the V8 thread. Thread-safe.
    void EnqueueCommand(std::string cmd)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        pendingCommands.push_back(std::move(cmd));
        cv.notify_one();
    }

    // Dispatch all currently queued commands. Must be called on the V8 thread.
    void ProcessPending()
    {
        if (!session) return;

        // Flush any already-queued commands first (e.g. pre-queued before Run()).
        DispatchQueued();

        // If waiting for the debugger, block here on the V8 thread processing
        // incoming commands until Runtime.runIfWaitingForDebugger is received.
        // This mirrors how Chrome/Node.js implement --inspect-brk: the host
        // blocks before any script runs, letting the client connect, send
        // Debugger.enable / setBreakpoint, etc., then release via
        // Runtime.runIfWaitingForDebugger.
        while (waitingForDebugger)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return !pendingCommands.empty() || !waitingForDebugger.load() || disconnecting.load();
            });

            if (!waitingForDebugger || disconnecting) break;

            std::string cmd = std::move(pendingCommands.front());
            pendingCommands.pop_front();
            lock.unlock();

            if (session)
            {
                v8_inspector::StringView sv(
                    reinterpret_cast<const uint8_t*>(cmd.data()), cmd.size());
                session->dispatchProtocolMessage(sv);
            }
        }
    }

    // Dispatch all currently queued commands without blocking.
    void DispatchQueued()
    {
        while (true)
        {
            std::string cmd;
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (pendingCommands.empty()) break;
                cmd = std::move(pendingCommands.front());
                pendingCommands.pop_front();
            }
            v8_inspector::StringView sv(
                reinterpret_cast<const uint8_t*>(cmd.data()), cmd.size());
            session->dispatchProtocolMessage(sv);
        }
    }

    ////////////////////////////////////////////////////////////
    // V8InspectorClient interface
    ////////////////////////////////////////////////////////////

    // Called by V8 when execution pauses (breakpoint, debugger statement, step).
    // Blocks the V8 thread until a resume/step command is dispatched.
    void runMessageLoopOnPause(int contextGroupId) override
    {
        // Guard against the race where SignalStop() (from Dispose()) fires before
        // this method is entered. Setting paused=true only under the lock ensures
        // that either SignalStop() already set disconnecting=true (and we see it
        // here and bail), or SignalStop() will see paused=true and notify the CV.
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (disconnecting) return;
            paused = true;
        }

        while (true)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return !pendingCommands.empty() || !paused.load() || disconnecting.load();
            });

            if (!paused || disconnecting) break;

            std::string cmd = std::move(pendingCommands.front());
            pendingCommands.pop_front();
            lock.unlock();

            // Dispatch on V8 thread (safe: we hold the V8 lock throughout).
            if (session)
            {
                v8_inspector::StringView sv(
                    reinterpret_cast<const uint8_t*>(cmd.data()), cmd.size());
                session->dispatchProtocolMessage(sv);
            }
        }

        paused = false;
    }

    // Called by V8 when Runtime.runIfWaitingForDebugger is dispatched.
    // Only invoked when the session was connected with kWaitingForDebugger.
    void runIfWaitingForDebugger(int contextGroupId) override
    {
        waitingForDebugger = false;
        SignalStop();
    }

    // Called by V8 when execution should resume from a pause.
    void quitMessageLoopOnPause() override
    {
        SignalStop();
    }

    // Called by V8 to get the default context for a group.
    v8::Local<v8::Context> ensureDefaultContextInGroup(int contextGroupId) override
    {
        return contextPersistent->Get(isolate);
    }

    ////////////////////////////////////////////////////////////
    // V8Inspector::Channel interface
    ////////////////////////////////////////////////////////////

    // Called by V8 to send a CDP response to a command we dispatched.
    void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override
    {
        FireMessage(message->string());
    }

    // Called by V8 to send a CDP notification/event (e.g., "Debugger.paused").
    void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override
    {
        FireMessage(message->string());
    }

    void flushProtocolNotifications() override {}

private:
    void FireMessage(const v8_inspector::StringView& view)
    {
        if (disconnecting) return;
        System::String^ msg = StringViewToManagedString(view);
        managedDebugger->OnMessageReceived(msg);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptDebugger
////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptDebugger::JavascriptDebugger(JavascriptContext^ context, bool waitForDebugger)
{
    if (context == nullptr)
        throw gcnew System::ArgumentNullException("context");
    if (context->IsDisposed())
        throw gcnew System::ObjectDisposedException("context");

    mContext = context;
    mDisposed = false;

    // Construct with native-only parameters (managed types are forbidden in the
    // InspectorImpl constructor due to V8_EXPORT base class constraints), then
    // set the managed back-reference from this managed constructor.
    mImpl = new InspectorImpl(context->GetIsolate(), context->GetContextPersistent(), waitForDebugger);
    mImpl->managedDebugger = this;

    // Connect while holding the V8 lock so the inspector sees the context.
    JavascriptScope scope(context);
    mImpl->Connect();

    // Register with the context so Run() will flush pending commands.
    context->mDebugger = this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptDebugger::~JavascriptDebugger()
{
    if (mDisposed) return;
    mDisposed = true;

    if (mContext != nullptr && !mContext->IsDisposed())
    {
        // Normal disposal: context is still alive, do V8-aware cleanup.
        mContext->mDebugger = nullptr;

        // Signal before acquiring the V8 lock. If the V8 thread is blocked in
        // runMessageLoopOnPause (holding the V8 lock), this unblocks it so it
        // can release the lock — otherwise acquiring JavascriptScope would deadlock.
        mImpl->SignalStop(true);

        JavascriptScope scope(mContext);
        mImpl->Disconnect();
    }
    // else: OnContextDisposing() was already called by the context destructor,
    // which performed V8 cleanup. mImpl->session is already null.

    delete mImpl;
    mImpl = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptDebugger::SendCommand(System::String^ jsonMessage)
{
    if (mDisposed)
        throw gcnew System::ObjectDisposedException("JavascriptDebugger");
    if (jsonMessage == nullptr)
        throw gcnew System::ArgumentNullException("jsonMessage");
    if (!mImpl->IsConnected())
        throw gcnew System::InvalidOperationException("Debugger is not connected.");

    mImpl->EnqueueCommand(ManagedStringToUtf8(jsonMessage));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool JavascriptDebugger::IsConnected::get()
{
    return !mDisposed && mImpl != nullptr && mImpl->IsConnected();
}

bool JavascriptDebugger::IsPaused::get()
{
    return !mDisposed && mImpl != nullptr && mImpl->IsPaused();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptDebugger::OnMessageReceived(System::String^ message)
{
    MessageReceived(this, gcnew DebuggerMessageEventArgs(message));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptDebugger::ProcessPendingCommands()
{
    if (mImpl != nullptr && mImpl->IsConnected())
        mImpl->ProcessPending();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptDebugger::OnContextDisposing()
{
    // Called from JavascriptContext::~JavascriptContext() with the V8 lock held.
    // Perform V8-aware cleanup before the isolate is disposed.
    if (mImpl != nullptr)
        mImpl->Disconnect();
    mContext = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
