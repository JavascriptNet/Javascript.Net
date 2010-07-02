////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptInterop.h
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

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace v8;

////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptInterop
////////////////////////////////////////////////////////////////////////////////////////////////////
class JavascriptInterop
{
	////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////
public:

	static Persistent<ObjectTemplate> GetObjectWrapperTemplate();

	static System::Object^ ConvertFromV8(Handle<Value> iValue);

	static Handle<Value> ConvertToV8(System::Object^ iObject);

	static Handle<Object> WrapObject(System::Object^ iObject);

	static System::Object^ UnwrapObject(Handle<Value> iValue);

	static Handle<Object> WrapFunction(System::Object^ iObject, System::String^ iName);

	static Handle<Value> Getter(Local<String> iName, const AccessorInfo &iInfo);

	static Handle<Value> Setter(Local<String> iName, Local<Value> iValue, const AccessorInfo& iInfo);

	static Handle<Value> IndexGetter(uint32_t iIndex, const AccessorInfo &iInfo);

	static Handle<Value> IndexSetter(uint32_t iIndex, Local<Value> iValue, const AccessorInfo &iInfo);

	static Handle<Value> Invoker(const v8::Arguments& iArgs);
	

	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
private:

	static Persistent<ObjectTemplate> sObjectWrapperTemplate;

};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////