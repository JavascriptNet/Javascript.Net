////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptContext.cpp
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

#include <atomic>
#include <msclr\lock.h>
#include <vcclr.h>
#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>
#include <signal.h>
#include "libplatform/libplatform.h"

#include "JavascriptContext.h"

#include "SystemInterop.h"
#include "JavascriptException.h"
#include "JavascriptExternal.h"
#include "JavascriptFunction.h"
#include "JavascriptInterop.h"
#include "JavascriptStackFrame.h"

using namespace msclr;
using namespace v8::platform;

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma managed(push, off)
	void GetPathsForInitialisation(char dll_path[MAX_PATH], char icudtl_dat_path[MAX_PATH])
	{
		HMODULE hm = NULL;
		if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCSTR)&GetPathsForInitialisation,  // any address in the module we're caring about
			&hm)) {
			int ret = GetLastError();
			fprintf(stderr, "GetModuleHandle error: %d\n", ret);
			raise(SIGABRT);  // Exit immediately.
		}
		int nchars = GetModuleFileNameA(hm, dll_path, MAX_PATH);
		if (nchars == 0 || nchars >= MAX_PATH) {
			int ret = GetLastError();
			fprintf(stderr, "GetModuleFileNameA error: %d\n", ret);
			raise(SIGABRT);  // Exit immediately.
		}
		// Because they can conflict with differently-versioned .bin/.dat files from Chromium/CefSharp,
		// we'll prefer .bin files prefixed by "v8_", if present.
		strcpy_s(icudtl_dat_path, MAX_PATH, dll_path);
		if (strlen(dll_path) > MAX_PATH - 20) {
			fprintf(stderr, "Path is too long - don't want to overflow our buffers.");
			raise(SIGABRT);  // Exit immediately.
		}
		strcpy_s(strrchr(icudtl_dat_path, '\\'), 15, "\\v8_icudtl.dat");
		FILE *file;
		if (fopen_s(&file, icudtl_dat_path, "r") == 0)
			fclose(file);
		else
			strcpy_s(strrchr(icudtl_dat_path, '\\'), 12, "\\icudtl.dat");
	}

    std::atomic_flag initalized = ATOMIC_FLAG_INIT;
	// This code didn't work in managed code, probably due to too-clever smart pointers.
	void UnmanagedInitialisation()
	{
        if (!initalized.test_and_set(std::memory_order_acquire)) {
            // Get location of DLL so that v8 can use it to find its .dat files.
            char dll_path[MAX_PATH], icudtl_dat_path[MAX_PATH];
            GetPathsForInitialisation(dll_path, icudtl_dat_path);
            v8::V8::InitializeICUDefaultLocation(dll_path, icudtl_dat_path);
            v8::Platform* platform = v8::platform::NewDefaultPlatform().release();
            v8::V8::InitializePlatform(platform);
            v8::V8::Initialize();
        }
	}
#pragma managed(pop)

v8::Local<v8::String> ToV8String(Isolate* isolate, System::String^ value) {
    if (value == nullptr)
        throw gcnew System::ArgumentNullException("value");
    pin_ptr<const wchar_t> namePtr = PtrToStringChars(value);
    wchar_t* name = (wchar_t*)namePtr;

    return String::NewFromTwoByte(isolate, (uint16_t*)name, v8::NewStringType::kNormal).ToLocalChecked();
}

static JavascriptContext::JavascriptContext()
{
    System::Threading::Mutex mutex(true, "FA12B681-E968-4D3A-833D-43B25865BEF1");
    UnmanagedInitialisation();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// Static function so it can be called from unmanaged code.
void FatalErrorCallback(const char* location, const char* message)
{
	JavascriptContext::FatalErrorCallbackMember(location, message);
	raise(SIGABRT);  // Exit immediately.
}

void JavascriptContext::FatalErrorCallbackMember(const char* location, const char* message)
{
	// Let's hope Out of Memory doesn't stop us allocating these strings!
	// I guess we can generally count on the garbage collector to find
	// us something, because it didn't have a chance to get involved if v8
	// has just run out.
	System::String ^location_str = gcnew System::String(location);
	System::String ^message_str = gcnew System::String(message);
	if (fatalErrorHandler != nullptr) {
		fatalErrorHandler(location_str, message_str);
	} else {
		System::Console::WriteLine(location_str);
		System::Console::WriteLine(message_str);
	}
}

JavascriptContext::JavascriptContext()
{
	// Unfortunately the fatal error handler is not installed early enough to catch
    // out-of-memory errors while creating new isolates
    // (see my post Catching V8::FatalProcessOutOfMemory while creating an isolate (SetFatalErrorHandler does not work)).
    // Also, HeapStatistics are only fetchable per-isolate, so they will not
    // easily allow us to work out whether we are about to run out (although they
    // would help us determine how much memory a new isolate used).
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	isolate = v8::Isolate::New(create_params);
	v8::Locker v8ThreadLock(isolate);
	v8::Isolate::Scope isolate_scope(isolate);

    isolate->SetFatalErrorHandler(FatalErrorCallback);

	mExternals = gcnew System::Collections::Generic::Dictionary<System::Object ^, WrappedJavascriptExternal>();
	mMethods = gcnew System::Collections::Generic::Dictionary<System::String ^, WrappedMethod>();
    mTypeToConstructorMapping = gcnew System::Collections::Generic::Dictionary<System::Type ^, System::IntPtr>();
	mFunctions = gcnew System::Collections::Generic::List<System::WeakReference ^>();
	HandleScope scope(isolate);
	mContext = new Persistent<Context>(isolate, Context::New(isolate));
    terminateRuns = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptContext::~JavascriptContext()
{
	{
		v8::Locker v8ThreadLock(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		for each (WrappedJavascriptExternal wrapped in mExternals->Values)
			delete wrapped.Pointer;
        for each (WrappedMethod wrapped in mMethods->Values)
        {
            wrapped.Pointer->Reset();
            delete wrapped.Pointer;
        }
        for each (System::WeakReference^ f in mFunctions) {
            JavascriptFunction ^function = safe_cast<JavascriptFunction ^>(f->Target);
            if (function != nullptr)
                delete function;
        }
        for each (System::IntPtr p in mTypeToConstructorMapping->Values) {
            delete (void *)p;
        }
		delete mContext;
		delete mExternals;
        delete mMethods;
        delete mTypeToConstructorMapping;
		delete mFunctions;
	}
	if (isolate != NULL)
		isolate->Dispose();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptContext::SetFatalErrorHandler(FatalErrorHandler^ handler)
{
	if (handler == nullptr)
		throw gcnew System::ArgumentNullException("handler");
	fatalErrorHandler = handler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptContext::SetFlags(System::String^ flags)
{
    std::string convertedFlags = msclr::interop::marshal_as<std::string>(flags);
    v8::V8::SetFlagsFromString(convertedFlags.c_str(), (int)convertedFlags.length());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptContext::TerminateExecution()
{
	// For backwards compatibility.
	TerminateExecution(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void JavascriptContext::TerminateExecution(bool terminate_subsequent_runs)
{
	terminateRuns = terminate_subsequent_runs;
	isolate->TerminateExecution();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool JavascriptContext::IsExecutionTerminating()
{
	return isolate->IsExecutionTerminating();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::SetParameter(System::String^ iName, System::Object^ iObject)
{
	SetParameter(iName, iObject, SetParameterOptions::None);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::SetParameter(System::String^ iName, System::Object^ iObject, SetParameterOptions options)
{
	JavascriptScope scope(this);
	v8::Isolate *isolate = JavascriptContext::GetCurrentIsolate();
	HandleScope handleScope(isolate);
	
	Local<Value> value = JavascriptInterop::ConvertToV8(iObject);

	if (options != SetParameterOptions::None) {
		Local<v8::Object> obj = value.As<v8::Object>();
		if (!obj.IsEmpty()) {
			Local<v8::External> wrap = obj->GetInternalField(0).As<v8::External>();
			if (!wrap.IsEmpty()) {
				JavascriptExternal* external = static_cast<JavascriptExternal*>(wrap->Value());
				external->SetOptions(options);
			}
		}
	}

    v8::Local<v8::String> key = ToV8String(isolate, iName);
	Local<Context>::New(isolate, *mContext)->Global()->Set(isolate->GetCurrentContext(), key, value).ToChecked();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

generic <typename AssociatedType> void JavascriptContext::SetConstructor(System::String^ name, System::Delegate^ constructor)
{
    SetConstructor(name, AssociatedType::typeid, constructor);
}

void JavascriptContext::SetConstructor(System::String^ name, System::Type^ associatedType, System::Delegate^ constructor)
{
    JavascriptScope scope(this);
    Isolate *isolate = JavascriptContext::GetCurrentIsolate();
    HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();

    Local<String> className = ToV8String(isolate, name);
    Local<FunctionTemplate> functionTemplate = JavascriptInterop::GetFunctionTemplateFromSystemDelegate(constructor);
    functionTemplate->SetClassName(className);
    JavascriptInterop::InitObjectWrapperTemplate(functionTemplate->InstanceTemplate());
    mTypeToConstructorMapping[associatedType] = System::IntPtr(new Persistent<FunctionTemplate>(isolate, functionTemplate));
    Local<Context>::New(isolate, *mContext)->Global()->Set(context, className, functionTemplate->GetFunction(context).ToLocalChecked());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptContext::GetParameter(System::String^ iName)
{
	if (iName == nullptr)
		throw gcnew System::ArgumentNullException("iName");
	pin_ptr<const wchar_t> namePtr = PtrToStringChars(iName);
	wchar_t* name = (wchar_t*)namePtr;
	JavascriptScope scope(this);
	v8::Isolate *isolate = JavascriptContext::GetCurrentIsolate();
	HandleScope handleScope(isolate);
	
    auto context = Local<Context>::New(isolate, *mContext);
	Local<Value> value = context->Global()->Get(context, String::NewFromTwoByte(isolate, (uint16_t*)name, v8::NewStringType::kNormal).ToLocalChecked()).ToLocalChecked();
	return JavascriptInterop::ConvertFromV8(value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptContext::Run(System::String^ iScript)
{
	if (iScript == nullptr)
		throw gcnew System::ArgumentNullException("iScript");
	if (terminateRuns)
		throw gcnew JavascriptException(L"Execution terminated");
	pin_ptr<const wchar_t> scriptPtr = PtrToStringChars(iScript);
	wchar_t* script = (wchar_t*)scriptPtr;
	JavascriptScope scope(this);
	//SetStackLimit();
	HandleScope handleScope(isolate);
	MaybeLocal<Value> ret;
	
	Local<Script> compiledScript = CompileScript(isolate, script);

	{
		TryCatch tryCatch(isolate);
		ret = (*compiledScript)->Run(this->GetCurrentIsolate()->GetCurrentContext());

		if (ret.IsEmpty())
			throw gcnew JavascriptException(tryCatch);
	}
	
	return JavascriptInterop::ConvertFromV8(ret.ToLocalChecked());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptContext::Run(System::String^ iScript, System::String^ iScriptResourceName)
{
	if (iScript == nullptr)
		throw gcnew System::ArgumentNullException("iScript");
	if (iScriptResourceName == nullptr)
		throw gcnew System::ArgumentNullException("iScriptResourceName");
	if (terminateRuns)
		throw gcnew JavascriptException(L"Execution terminated");
	pin_ptr<const wchar_t> scriptPtr = PtrToStringChars(iScript);
	wchar_t* script = (wchar_t*)scriptPtr;
	pin_ptr<const wchar_t> scriptResourceNamePtr = PtrToStringChars(iScriptResourceName);
	wchar_t* scriptResourceName = (wchar_t*)scriptResourceNamePtr;
	JavascriptScope scope(this);
	//SetStackLimit();
	HandleScope handleScope(isolate);
	MaybeLocal<Value> ret;	

	Local<Script> compiledScript = CompileScript(isolate, script, scriptResourceName);
	
	{
		TryCatch tryCatch(isolate);
		ret = (*compiledScript)->Run(this->GetCurrentIsolate()->GetCurrentContext());

		if (ret.IsEmpty())
			throw gcnew JavascriptException(tryCatch);
	}
	
	return JavascriptInterop::ConvertFromV8(ret.ToLocalChecked());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static System::String^ v8StringToString(v8::Local<v8::String> handle) {
    if (handle.IsEmpty()) {
        return nullptr;
    }
    return (System::String^)JavascriptInterop::ConvertFromV8(handle);
}

System::Collections::Generic::List<JavascriptStackFrame^>^
JavascriptContext::GetCurrentStack(int maxDepth)
{
    auto stack = gcnew System::Collections::Generic::List<JavascriptStackFrame^>();
    v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(
        this->GetCurrentIsolate(), maxDepth, v8::StackTrace::kScriptName
    );

    for (int i = 0; i < stackTrace->GetFrameCount(); ++i) {
        v8::Local<v8::StackFrame> frame = stackTrace->GetFrame(this->GetCurrentIsolate(), i);
        JavascriptStackFrame^ managedFrame = gcnew JavascriptStackFrame();
        managedFrame->LineNumber = frame->GetLineNumber();
        managedFrame->Column = frame->GetColumn();
        managedFrame->FunctionName = v8StringToString(frame->GetFunctionName());
        managedFrame->ScriptName = v8StringToString(frame->GetScriptName());
        managedFrame->ScriptNameOrSourceURL = v8StringToString(frame->GetScriptNameOrSourceURL());
        managedFrame->IsEval = frame->IsEval();
        managedFrame->IsConstructor = frame->IsConstructor();
        managedFrame->IsWasm = frame->IsWasm();
        stack->Add(managedFrame);
    }
    return stack;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//void
//JavascriptContext::SetStackLimit()
//{
//    // This stack limit needs to be set for each Run because the
//    // stack of the caller could be in completely different spots (e.g.
//    // different threads), or have moved up/down because calls/returns.
//	v8::ResourceConstraints rc;
//
//    // Copied form v8/test/cctest/test-api.cc
//    uint32_t size = 500000;
//    uint32_t* limit = &size - (size / sizeof(size));
//    // If the size is very large and the stack is very near the bottom of
//    // memory then the calculation above may wrap around and give an address
//    // that is above the (downwards-growing) stack.  In that case we return
//    // a very low address.
//    if (limit > &size)
//        limit = reinterpret_cast<uint32_t*>(sizeof(size));
//    
//    int mos = rc.max_old_space_size();
//    
//    rc.set_stack_limit((uint32_t *)(limit));
//    rc.set_max_old_space_size(1700);
//	v8::SetResourceConstraints(isolate, &rc);
//}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptContext^
JavascriptContext::GetCurrent()
{
	return sCurrentContext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Isolate *
JavascriptContext::GetCurrentIsolate()
{
	return sCurrentContext->isolate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<v8::Object> JavascriptContext::GetGlobal()
{
	return mContext->Get(this->GetCurrentIsolate())->Global();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Locker *
JavascriptContext::Enter([System::Runtime::InteropServices::Out] JavascriptContext^% old_context)
{
	v8::Locker *locker = new v8::Locker(isolate);
	isolate->Enter();
    old_context = sCurrentContext;
	sCurrentContext = this;
	HandleScope scope(isolate);
	Local<Context>::New(isolate, *mContext)->Enter();
	return locker;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::Exit(v8::Locker *locker, JavascriptContext^ old_context)
{
	{
		HandleScope scope(isolate);
		Local<Context>::New(isolate, *mContext)->Exit();
	}
	sCurrentContext = old_context;
	isolate->Exit();
	delete locker;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Exposed for the benefit of a regression test.
void
JavascriptContext::Collect()
{
    while(!this->isolate->IdleNotificationDeadline(1)) {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptExternal*
JavascriptContext::WrapObject(System::Object^ iObject)
{
	WrappedJavascriptExternal external_wrapped;
	if (mExternals->TryGetValue(iObject, external_wrapped))
	{
		// We've wrapped this guy before.
		return external_wrapped.Pointer;
	}
	else
	{
		JavascriptExternal* external = new JavascriptExternal(iObject);
		mExternals[iObject] = WrappedJavascriptExternal(external);
		return external;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<FunctionTemplate>
JavascriptContext::GetObjectWrapperConstructorTemplate(System::Type ^type)
{
    System::IntPtr ptrToConstructor;
    if (!mTypeToConstructorMapping->TryGetValue(type, ptrToConstructor)) {
        Local<FunctionTemplate> constructor = FunctionTemplate::New(GetCurrentIsolate());
        JavascriptInterop::InitObjectWrapperTemplate(constructor->InstanceTemplate());
        mTypeToConstructorMapping[type] = System::IntPtr(new Persistent<FunctionTemplate>(isolate, constructor));
        return constructor;
    }
    Persistent<FunctionTemplate> *constructor = (Persistent<FunctionTemplate> *)(void *)ptrToConstructor;
	return constructor->Get(isolate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::RegisterFunction(System::Object^ f)
{
    // Note that while we do store WeakReferences, we never clean up this hashtable,
    // so it will just grow and grow.
	mFunctions->Add(gcnew System::WeakReference(f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^ JavascriptContext::V8Version::get()
{
	return gcnew System::String(v8::V8::GetVersion());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Script>
CompileScript(v8::Isolate *isolate, wchar_t const *source_code, wchar_t const *resource_name)
{
	// convert source
	Local<String> source = String::NewFromTwoByte(isolate, (uint16_t const *)source_code, v8::NewStringType::kNormal).ToLocalChecked();

	// compile
	{
		TryCatch tryCatch(isolate);

		MaybeLocal<Script> script;
		if (resource_name == NULL)
		{
			script = Script::Compile(JavascriptContext::GetCurrentIsolate()->GetCurrentContext(), source);
		}
		else
		{
			Local<String> resource = String::NewFromTwoByte(isolate, (uint16_t const *)resource_name, v8::NewStringType::kNormal).ToLocalChecked();
            ScriptOrigin *origin = new ScriptOrigin(resource);
			script = Script::Compile(JavascriptContext::GetCurrentIsolate()->GetCurrentContext(), source, origin);
		}

		if (script.IsEmpty())
			throw gcnew JavascriptException(tryCatch);

		return script.ToLocalChecked();
	}
}

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
