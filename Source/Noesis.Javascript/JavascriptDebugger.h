////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptDebugger.h
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

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "JavascriptContext.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration of native implementation struct (defined in JavascriptDebugger.cpp)
struct InspectorImpl;

////////////////////////////////////////////////////////////////////////////////////////////////////
// DebuggerMessageEventArgs
//
// Event arguments for messages received from the V8 inspector.
// The Message property contains the raw Chrome DevTools Protocol (CDP) JSON string.
////////////////////////////////////////////////////////////////////////////////////////////////////
public ref class DebuggerMessageEventArgs : System::EventArgs
{
public:
    property System::String^ Message;

    DebuggerMessageEventArgs(System::String^ message)
    {
        Message = message;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptDebugger
//
// Attaches a V8 Inspector session to a JavascriptContext, providing access to the
// Chrome DevTools Protocol (CDP) for debugging JavaScript execution.
//
// Usage:
//   using var context = new JavascriptContext();
//   using var debugger = new JavascriptDebugger(context);
//   debugger.MessageReceived += (s, e) => Console.WriteLine(e.Message);
//
//   // Queue setup commands before running (processed at start of Run())
//   debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
//   debugger.SendCommand("{\"id\":2,\"method\":\"Debugger.setBreakpointByUrl\",...}");
//
//   context.Run(script);
//   // When paused at a breakpoint, MessageReceived fires with "Debugger.paused"
//   // Send resume/step commands from any thread via SendCommand()
//
// Threading:
//   - SendCommand() is thread-safe and may be called from any thread.
//   - Commands sent before Run() are queued and dispatched at the start of Run().
//   - Commands sent while paused at a breakpoint are dispatched on the V8 thread
//     from within the debugger's pause message loop.
//   - MessageReceived events are fired on the V8 thread (the thread calling Run()).
//
// Lifetime:
//   - The debugger must be disposed before the context, or the context disposes
//     the debugger automatically via its own destructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
public ref class JavascriptDebugger : System::IDisposable
{
    ////////////////////////////////////////////////////////////
    // Constructor/Destructor
    ////////////////////////////////////////////////////////////
public:
    /// <summary>
    /// Creates and connects a V8 Inspector session to the given context.
    /// Must be called before context.Run() to enable debugging for subsequent runs.
    /// </summary>
    JavascriptDebugger(JavascriptContext^ context, bool waitForDebugger);

    ~JavascriptDebugger();

    ////////////////////////////////////////////////////////////
    // Public API
    ////////////////////////////////////////////////////////////
public:
    /// <summary>
    /// Fired when the V8 inspector sends a CDP response or notification.
    /// The Message property contains the raw JSON string.
    /// This event is fired on the V8 thread (the thread calling Run()).
    /// </summary>
    event System::EventHandler<DebuggerMessageEventArgs^>^ MessageReceived;

    /// <summary>
    /// Sends a Chrome DevTools Protocol (CDP) command to the V8 inspector.
    /// Thread-safe: may be called from any thread.
    ///
    /// Commands sent before Run() is called are queued and dispatched at the
    /// start of the next Run() call, before script execution begins.
    ///
    /// Commands sent from a background thread while the script is paused at a
    /// breakpoint are dispatched from within the pause message loop on the V8 thread.
    /// </summary>
    void SendCommand(System::String^ jsonMessage);

    /// <summary>
    /// Whether the debugger session is currently active.
    /// </summary>
    property bool IsConnected { bool get(); }

    /// <summary>
    /// Whether the debuger session is currently paused.
    /// </summary>
    property bool IsPaused { bool get(); }

    ////////////////////////////////////////////////////////////
    // Internal methods (used by JavascriptContext and InspectorImpl)
    ////////////////////////////////////////////////////////////
internal:
    /// Called by InspectorImpl to deliver CDP messages from V8 to managed code.
    void OnMessageReceived(System::String^ message);

    /// Dispatches all currently queued commands on the V8 thread.
    /// Called by JavascriptContext::Run() before executing a script.
    /// Must be called while holding the V8 lock.
    void ProcessPendingCommands();

    /// Called by JavascriptContext::~JavascriptContext() before V8 cleanup.
    /// Performs V8-aware disconnect while the V8 lock is still held.
    void OnContextDisposing();

    ////////////////////////////////////////////////////////////
    // Data members
    ////////////////////////////////////////////////////////////
private:
    InspectorImpl* mImpl;
    JavascriptContext^ mContext;
    bool mDisposed;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
