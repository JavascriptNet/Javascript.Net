////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptExternal.h
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
#include <gcroot.h>
#include "JavascriptContext.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace v8;

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptExternal
//
// Wraps around a CLI object and serves it up to v8.  This object is itself
// stored within the 0th internal field of a JavaScript object.
////////////////////////////////////////////////////////////////////////////////////////////////////
class JavascriptExternal
{
	////////////////////////////////////////////////////////////
	// Constructor
	////////////////////////////////////////////////////////////
public:

	JavascriptExternal(System::Object^ iObject);

	~JavascriptExternal();

	
	////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////
public:

	SetParameterOptions GetOptions() { return mOptions; }

	void SetOptions(SetParameterOptions options) { mOptions = options; }

	System::Object^ GetObject();

	Local<Function> GetMethod(wstring iName);

	Local<Function> GetMethod(Local<String> iName);

	bool GetProperty(wstring iName, Local<Value> &result);

	Local<Value> GetProperty(uint32_t iIndex);

	Local<Value> SetProperty(wstring iName, Local<Value> iValue);

	Local<Value> SetProperty(uint32_t iIndex, Local<Value> iValue);

    Local<Function> GetIterator();

    Local<Object> ToLocal(Isolate* isolate);

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
public:
    Persistent<Object> mPersistent;
private:
	
	// Handle to the .Net object being wrapped.  It takes this
	// form so that the garbage collector won't try to move it.
	System::Runtime::InteropServices::GCHandle mObjectHandle;

	SetParameterOptions mOptions;

    void InitializePersistent(Isolate* isolate, Local<Object> object);

    static void IteratorCallback(const v8::FunctionCallbackInfo<Value>& iArgs);
    static void IteratorNextCallback(const v8::FunctionCallbackInfo<Value>& iArgs);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////