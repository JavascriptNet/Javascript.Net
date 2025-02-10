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
using namespace System::Collections::Generic;

// Forward declaration
ref class JavascriptFunction;


////////////////////////////////////////////////////////////////////////////////////////////////////
// Remembers which objects have been just converted, to avoid stack overflows when we are 
// converting self-referential objects.
////////////////////////////////////////////////////////////////////////////////////////////////////
class ConvertedObjects
{
	//std::unordered_map<v8::Local<v8::Object>, int> mConvertedHandles = new std::unordered_map<v8::Local<v8::Object>, int>();
	//gcroot<System::Collections::Generic::Dictionary<System::Int32, System::Object^> ^> intToConverted = gcnew System::Collections::Generic::Dictionary<System::Int32, System::Object^>();
public:
	ConvertedObjects();
	~ConvertedObjects();
	System::Object^ GetConverted(v8::Local<v8::Object> o);
	void AddConverted(v8::Local<v8::Object> o, System::Object^ converted);

private:
	v8::Local<v8::Map> objectToConversion;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// JavascriptInterop
////////////////////////////////////////////////////////////////////////////////////////////////////
class JavascriptInterop
{
	////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////
public:

	static void InitObjectWrapperTemplate(Local<ObjectTemplate> &object);

	static System::Object^ ConvertFromV8(Local<Value> iValue);

	static Local<Value> ConvertToV8(System::Object^ iObject);

	static System::Object^ UnwrapObject(Local<Value> iValue);

	static void Invoker(const v8::FunctionCallbackInfo<Value>& iArgs);

	static Local<Value> HandleTargetInvocationException(System::Reflection::TargetInvocationException^ exception);

    static v8::Local<v8::FunctionTemplate> GetFunctionTemplateFromSystemDelegate(System::Delegate^ iDelegate);

private:
	static System::Object^ ConvertFromV8(Local<Value> iValue, ConvertedObjects &already_converted);

	static JavascriptFunction^ ConvertFunctionFromV8(Local<Value> iValue);

	static System::Object^ ConvertObjectFromV8(Local<Object> iObject, ConvertedObjects &already_converted);

	static System::DateTime^ ConvertDateFromV8(Local<Date> iValue);

    static Local<Date> ConvertDateTimeToV8(System::DateTime^ dateTime);

    static System::Text::RegularExpressions::Regex^ ConvertRegexFromV8(Local<Value> iValue);

	static v8::Local<v8::Value> ConvertFromSystemArray(System::Array^ iArray);

    static v8::Local<v8::Value> ConvertFromSystemRegex(System::Text::RegularExpressions::Regex^ iRegex);

	static v8::Local<v8::Value> ConvertFromSystemDictionary(System::Object^ iObject);

	static v8::Local<v8::Value> ConvertFromSystemList(System::Object^ iObject);

	static v8::Local<v8::Value> ConvertFromSystemDelegate(System::Delegate^ iDelegate);

	static void DelegateInvoker(const FunctionCallbackInfo<Value>& info);

	static bool IsSystemObject(Local<Value> iValue);

	static Local<Object> WrapObject(System::Object^ iObject);

	static System::Object^ ConvertArrayFromV8(Local<Value> iValue, ConvertedObjects &already_converted);

	static Intercepted Getter(Local<Name> iName, const PropertyCallbackInfo<Value>& iInfo);

	static Intercepted Setter(Local<String> iName, Local<Value> iValue, const PropertyCallbackInfo<Value>& iInfo);

	static Intercepted IndexGetter(uint32_t iIndex, const PropertyCallbackInfo<Value>& iInfo);

	static Intercepted IndexSetter(uint32_t iIndex, Local<Value> iValue, const PropertyCallbackInfo<Value>& iInfo);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////