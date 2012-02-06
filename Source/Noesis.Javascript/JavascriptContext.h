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


////////////////////////////////////////////////////////////////////////////////////////////////////
// CliV8Locker
//
// This is a separate class to ensure that the lock is always disposed.
// If we included the pointer in JavascriptContext then an exception in
// its constructor would stop the lock being released.
////////////////////////////////////////////////////////////////////////////////////////////////////
public ref class CliV8Locker
{
internal:
	CliV8Locker() { locker = new v8::Locker(); }
	~CliV8Locker() { delete locker; }

private:
	v8::Locker *locker;
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

	System::Object^ GetParameter(System::String^ iName);

	virtual System::Object^ Run(System::String^ iSourceCode);

	virtual System::Object^ Run(System::String^ iScript, System::String^ iScriptResourceName);

	static void Collect();

	////////////////////////////////////////////////////////////
	// Internal methods
	////////////////////////////////////////////////////////////
internal:
	static JavascriptContext^ GetCurrent();

	void Enter();

	void Exit();

	void Clear();

	JavascriptExternal* WrapObject(System::Object^ iObject);

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
internal:
	// v8 context required to be active for all v8 operations.
	Persistent<Context>* mContext;

	// v8 objects we hang onto for the duration.
	vector<JavascriptExternal*>* mExternals;

	// v8 can be used on multiple threads, but not at the same time.  This is
	// ensures it is not.  It is OK to nest v8::Lockers in one thread.
	CliV8Locker v8ThreadLock;

	// Keeping track of recursion.
	[System::ThreadStaticAttribute] static JavascriptContext ^sCurrentContext;
	JavascriptContext^ oldContext;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptScope
////////////////////////////////////////////////////////////////////////////////////////////////////
class JavascriptScope
{
public:
	JavascriptScope(JavascriptContext^ iContext)
	{ iContext->Enter(); }
	
	~JavascriptScope()
	{ JavascriptContext::GetCurrent()->Exit(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Standalone functions - can be called from unmanaged code too
////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Script> CompileScript(wchar_t const *source_code);

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
