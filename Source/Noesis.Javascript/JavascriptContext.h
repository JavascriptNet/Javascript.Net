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

	////////////////////////////////////////////////////////////
	// Internal methods
	////////////////////////////////////////////////////////////
internal:
	static void Collect();

	void SetStackLimit();

	static JavascriptContext^ GetCurrent();

	v8::Locker *Enter();

	void Exit(v8::Locker *locker);

	void Clear();

	JavascriptExternal* WrapObject(System::Object^ iObject);

	Handle<ObjectTemplate> GetObjectWrapperTemplate();

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////

protected:
	// By entering an isolate before using a context, we can have multiple
	// contexts used simultaneously in different threads.
	v8::Isolate *isolate;

	// v8 context required to be active for all v8 operations.
	Persistent<Context>* mContext;

	// v8 objects we hang onto for the duration.
	vector<JavascriptExternal*>* mExternals;

	// Keeping track of recursion.
	[System::ThreadStaticAttribute] static JavascriptContext ^sCurrentContext;
	JavascriptContext^ oldContext;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptScope
//
// This must be constructed before any use of handles or calling of v8 
// functions.  It protects against simultaneous multithreaded use of v8.
////////////////////////////////////////////////////////////////////////////////////////////////////
class JavascriptScope
{
	// It is OK to nest v8::Lockers in one thread.
	v8::Locker *v8ThreadLock;

public:
	JavascriptScope(JavascriptContext^ iContext)
	{ v8ThreadLock = iContext->Enter(); }
	
	~JavascriptScope()
	{ JavascriptContext::GetCurrent()->Exit(v8ThreadLock); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Standalone functions - can be called from unmanaged code too
////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Script> CompileScript(wchar_t const *source_code, wchar_t const *resource_name = NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
