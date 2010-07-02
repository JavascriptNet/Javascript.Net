////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JSContext.h
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

#include "JavascriptObject.h"

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
ref class JSScript;

////////////////////////////////////////////////////////////////////////////////////////////////////
// JSContext
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

	System::Object^ Run(System::String^ iSourceCode);

	////////////////////////////////////////////////////////////
	// Internal methods
	////////////////////////////////////////////////////////////
internal:

	static JavascriptContext^ GetCurrent();

	void Enter();

	void Exit();

	void Clear();

	JavascriptExternal* WrapObject(System::Object^ iObject);

	Persistent<Script> Compile(System::String^ iJSScriptCode);

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
internal:

	Persistent<Context>* mContext;
	vector<JavascriptExternal*>* mExternals;
	// Persistent<Script>* mJSScript;

	[System::ThreadStaticAttribute]
	static JavascriptContext^ sCurrentContext;
	static Object^ mLock = gcnew Object();

};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JSScope
////////////////////////////////////////////////////////////////////////////////////////////////////
class JSScope
{
public:
	JSScope(JavascriptContext^ iContext)
	{ iContext->Enter(); }
	
	~JSScope()
	{ JavascriptContext::GetCurrent()->Exit(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////