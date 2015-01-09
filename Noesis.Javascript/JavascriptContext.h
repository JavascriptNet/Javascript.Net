////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptContext.h
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

#include <v8.h>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace v8;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

class JavascriptExternal;

[System::Flags]
public enum class SetParameterOptions : int
{
    None = 0,
    RejectUnknownProperties = 1
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// WrappedJavascriptExternal
//
// Type-safely wraps a native pointer for inclusion in managed code as an IntPtr.  I thought
// there would already be something for this, but I couldn't find it.
////////////////////////////////////////////////////////////////////////////////////////////////////
public value struct WrappedMethod
{
private:
	System::IntPtr pointer;

internal:
	WrappedMethod(Persistent<Function> *value)
	{
		System::IntPtr value_pointer(value);
        pointer = value_pointer;
	}

	property Persistent<Function> *Pointer
    {
        Persistent<Function> *get()
        {
            return (Persistent<Function> *)(void *)pointer;
        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// WrappedJavascriptExternal
//
// See comment in WrappedMethod.
////////////////////////////////////////////////////////////////////////////////////////////////////
public value struct WrappedJavascriptExternal
{
private:
	System::IntPtr pointer;

internal:
	WrappedJavascriptExternal(JavascriptExternal *value)
	{
		System::IntPtr value_pointer(value);
        pointer = value_pointer;
	}

	property JavascriptExternal *Pointer
    {
        JavascriptExternal *get()
        {
            return (JavascriptExternal *)(void *)pointer;
        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptContext
//
// This is the interface provided to our C# code.
//
// Being a CLR data structure, this cannot be seen by x86 code.
////////////////////////////////////////////////////////////////////////////////////////////////////
public ref class JavascriptContext: public System::IDisposable
{
	////////////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////////////
public:
    static JavascriptContext();

	JavascriptContext();

	~JavascriptContext();


	////////////////////////////////////////////////////////////
	// Public methods
	////////////////////////////////////////////////////////////
public:

	void SetParameter(System::String^ iName, System::Object^ iObject);

	void SetParameter(System::String^ iName, System::Object^ iObject, SetParameterOptions options);

	System::Object^ GetParameter(System::String^ iName);

	virtual System::Object^ Run(System::String^ iSourceCode);

	virtual System::Object^ Run(System::String^ iScript, System::String^ iScriptResourceName);
		
	property static System::String^ V8Version { System::String^ get(); }

	void TerminateExecution();

    bool IsExecutionTerminating();

	static void Collect();

	// Fatal errors can occur when v8 runs out of memory.  Your process
	// will exit immediately after this handler is called, because
	// that's just how v8 works.
	// (http://stackoverflow.com/questions/16797423/how-to-handle-v8-engine-crash-when-process-runs-out-of-memory)
	//
	// Call this just once for the whole library.
	delegate void FatalErrorHandler(System::String^ location, System::String^ message);
	static void SetFatalErrorHandler(FatalErrorHandler^ handler);

	////////////////////////////////////////////////////////////
	// Internal methods
	////////////////////////////////////////////////////////////
internal:
	void SetStackLimit();

	static JavascriptContext^ GetCurrent();
	
	static v8::Isolate *GetCurrentIsolate();

    v8::Locker *Enter([System::Runtime::InteropServices::Out] JavascriptContext^% old_context);

	void Exit(v8::Locker *locker, JavascriptContext^ old_context);

	JavascriptExternal* WrapObject(System::Object^ iObject);

	Handle<ObjectTemplate> GetObjectWrapperTemplate();
		
	static void FatalErrorCallbackMember(const char* location, const char* message);

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
protected:
	// By entering an isolate before using a context, we can have multiple
	// contexts used simultaneously in different threads.
	v8::Isolate *isolate;

	// v8 context required to be active for all v8 operations.
	Persistent<Context>* mContext;

	// Avoids us recreating these too often.
	Persistent<ObjectTemplate> *objectWrapperTemplate;

	// Stores every JavascriptExternal we create.  This saves time if the same
	// objects are recreated frequently, and stops us building up a huge
	// collection of JavascriptExternal objects that won't be freed until
	// the context is destroyed.
	System::Collections::Generic::Dictionary<System::Object ^, WrappedJavascriptExternal> ^mExternals;

	// Keeping track of recursion.
	[System::ThreadStaticAttribute] static JavascriptContext ^sCurrentContext;

	static FatalErrorHandler^ fatalErrorHandler;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptScope
//
// This must be constructed before any use of handles or calling of v8 
// functions.  It protects against simultaneous multithreaded use of v8.
////////////////////////////////////////////////////////////////////////////////////////////////////
ref class JavascriptScope
{
	// It is OK to nest v8::Lockers in one thread.
	v8::Locker *v8ThreadLock;
    JavascriptContext^ oldContext;

public:
	JavascriptScope(JavascriptContext^ iContext)
	{
	    // We store the old context so that JavascriptContexts can be created and run
	    // recursively.
        v8ThreadLock = iContext->Enter(oldContext);
    }
	
	~JavascriptScope()
	{
        JavascriptContext::GetCurrent()->Exit(v8ThreadLock, oldContext);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Standalone functions - can be called from unmanaged code too
////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Script> CompileScript(wchar_t const *source_code, wchar_t const *resource_name = NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
