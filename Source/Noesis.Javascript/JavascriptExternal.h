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

#include <map>

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace v8;

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptExternal
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
	
	void Clear();

	System::Object^ GetObject();

	Handle<Function> GetMethod(wstring iName);

	Handle<Function> GetMethod(Handle<String> iName);

	Handle<Value> GetProperty(wstring iName);

	Handle<Value> GetProperty(Handle<String> iName);

	Handle<Value> GetProperty(uint32_t iIndex);

	Handle<Value> SetProperty(wstring iName, Handle<Value> iValue);

	Handle<Value> SetProperty(Handle<String> iName, Handle<Value> iValue);

	Handle<Value> SetProperty(uint32_t iIndex, Handle<Value> iValue);

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
private:
	
	System::Runtime::InteropServices::GCHandle mObjectHandle;
	map<wstring, Persistent<Function> > mMethods;

};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////