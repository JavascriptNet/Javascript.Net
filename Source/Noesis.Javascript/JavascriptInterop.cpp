////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptInterop.cpp
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

#include "JavascriptInterop.h"

#include "SystemInterop.h"
#include "JavascriptContext.h"
#include "JavascriptException.h"
#include "JavascriptExternal.h"
#include "JavascriptObject.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

Persistent<ObjectTemplate> JavascriptInterop::sObjectWrapperTemplate;

////////////////////////////////////////////////////////////////////////////////////////////////////

Persistent<ObjectTemplate>
JavascriptInterop::GetObjectWrapperTemplate()
{
	if (sObjectWrapperTemplate.IsEmpty())
	{
		HandleScope handleScope;

		Handle<ObjectTemplate> result = ObjectTemplate::New();
		result->SetInternalFieldCount(1);
		result->SetNamedPropertyHandler(Getter, Setter);
		result->SetIndexedPropertyHandler(IndexGetter, IndexSetter);
		sObjectWrapperTemplate = Persistent<ObjectTemplate>::New(handleScope.Close(result));
	}

	return sObjectWrapperTemplate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptInterop::ConvertFromV8(Handle<Value> iValue)
{
	if (iValue->IsNull())
		return nullptr;
	else if (iValue->IsBoolean())
		return gcnew System::Boolean(iValue->BooleanValue());
	else if (iValue->IsInt32())
		return gcnew System::Int32(iValue->Int32Value());
	else if (iValue->IsNumber())
		return gcnew System::Double(iValue->NumberValue());
	else if (iValue->IsString())
		return gcnew System::String(*String::Utf8Value(iValue->ToString()));
	else if (iValue->IsExternal())
		return UnwrapObject(iValue);
	else if (iValue->IsObject())
	{
		System::Object^ object = UnwrapObject(iValue);
		if (object != nullptr)
			return object;
		else
			gcnew JavascriptObject(iValue->ToObject());
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::ConvertToV8(System::Object^ iObject)
{
	if (iObject != nullptr)
	{
		System::Type^ type = iObject->GetType();

		if (type == System::Boolean::typeid)
			return v8::Boolean::New((bool) iObject);
		if (type == System::Int16::typeid)
			return v8::Int32::New((int) iObject);
		if (type == System::Int32::typeid)
			return v8::Int32::New((int) iObject);
		if (type == System::Single::typeid)
			return v8::Number::New((float) iObject);
		if (type == System::Double::typeid)
			return v8::Number::New((double) iObject);
		if (type == System::String::typeid)
			return v8::String::New(SystemInterop::ConvertFromSystemString((System::String^) iObject).c_str());
		else
			return WrapObject(iObject);
	}

	return Handle<Value>();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: should return Handle<External>
Handle<Object>
JavascriptInterop::WrapObject(System::Object^ iObject)
{
	JavascriptContext^ context = JavascriptContext::GetCurrent();

	if (context != nullptr)
	{
		Handle<ObjectTemplate> templ = GetObjectWrapperTemplate();
		Handle<Object> object = templ->NewInstance();
		object->SetInternalField(0, External::New(context->WrapObject(iObject)));
		return object;
	}

	throw gcnew System::Exception("No context currently active.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: should use Handle<External> iExternal
System::Object^
JavascriptInterop::UnwrapObject(Handle<Value> iValue)
{
	if (iValue->IsObject())
	{
		Handle<Object> object = iValue->ToObject();

		if (object->InternalFieldCount() > 0)
		{
			Handle<External> external = Handle<External>::Cast(object->GetInternalField(0));
			JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
			return wrapper->GetObject();
		}
	}
	else if (iValue->IsExternal())
	{
		Handle<External> external = Handle<External>::Cast(iValue);
		JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
		return wrapper->GetObject();
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::Getter(Local<String> iName, const AccessorInfo &iInfo)
{
	Handle<External> external = Handle<External>::Cast(iInfo.Holder()->GetInternalField(0));
	JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
	Handle<Function> function;
	Handle<Value> value;

	// get method
	function = wrapper->GetMethod(iName);
	if (!function.IsEmpty())
		return function;

	// get property
	value = wrapper->GetProperty(iName);
	if (!value.IsEmpty())
		return value;

	// member not found
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::Setter(Local<String> iName, Local<Value> iValue, const AccessorInfo& iInfo)
{
	Handle<External> external = Handle<External>::Cast(iInfo.Holder()->GetInternalField(0));
	JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
	Handle<Function> function;
	Handle<Value> value;
	
	// set property
	value = wrapper->SetProperty(iName, iValue);

	if (!value.IsEmpty())
		return value;

	// member not found
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::IndexGetter(uint32_t iIndex, const AccessorInfo &iInfo)
{
	Handle<External> external = Handle<External>::Cast(iInfo.Holder()->GetInternalField(0));
	JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
	Handle<Value> value;

	// get property
	value = wrapper->GetProperty(iIndex);
	if (!value.IsEmpty())
		return value;

	// member not found
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::IndexSetter(uint32_t iIndex, Local<Value> iValue, const AccessorInfo &iInfo)
{
	Handle<External> external = Handle<External>::Cast(iInfo.Holder()->GetInternalField(0));
	JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
	Handle<Value> value;

	// get property
	value = wrapper->SetProperty(iIndex, iValue);
	if (!value.IsEmpty())
		return value;

	// member not found
	//throw gcnew JavascriptException(tryCatch);
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::Invoker(const v8::Arguments& iArgs)
{
	System::Object^ holder = UnwrapObject(iArgs.Holder());
	System::Type^ holderType = holder->GetType();
	System::Object^ data = UnwrapObject(Handle<External>::Cast(iArgs.Data()));
	cli::array<System::Reflection::MemberInfo^>^ members;
	System::Reflection::MethodInfo^ bestMethod;
	cli::array<System::Object^>^ suppliedArguments;
	cli::array<System::Object^>^ bestMethodArguments;
	int bestMethodMatchedArgs = -1;
	System::Object^ ret;

	// get members list
	members = (cli::array<System::Reflection::MemberInfo^>^) data;

	// parameters
	suppliedArguments = gcnew cli::array<System::Object^>(iArgs.Length());
	for (int i = 0; i < iArgs.Length(); i++)
		suppliedArguments[i] = ConvertFromV8(iArgs[i]);
	
	// look for best matching method
	for (int i = 0; i < members->Length; i++)
	{
		System::Reflection::MethodInfo^ method = (System::Reflection::MethodInfo^) members[i];
		cli::array<System::Reflection::ParameterInfo^>^ parametersInfo = method->GetParameters();
		cli::array<System::Object^>^ arguments;

		// match arguments & parameters counts
		if (iArgs.Length() == parametersInfo->Length)
		{
			int match = 0;
			int failed = 0;

			// match parameters
			arguments = gcnew cli::array<System::Object^>(iArgs.Length());
			for (int p = 0; p < suppliedArguments->Length; p++)
			{
				System::Type^ type = parametersInfo[p]->ParameterType;
				System::Object^ arg;

				if (suppliedArguments[p] != nullptr)
				{
					if (suppliedArguments[p]->GetType() == type)
						match++;

					arg = SystemInterop::ConvertToType(suppliedArguments[p], type);
					if (arg == nullptr)
					{
						failed++;
						break;
					}

					arguments[p] = arg;
				}
			}

			// skip if a conversion failed
			if (failed > 0)
				continue;

			// remember best match
			if (match > bestMethodMatchedArgs)
			{
				bestMethod = method;
				bestMethodArguments = arguments;
				bestMethodMatchedArgs = match;
			}

			// skip lookup if all args matched
			if (match == arguments->Length)
				break;
		}
	}

	try
	{
		// invoke
		ret = bestMethod->Invoke(holder, bestMethodArguments);
	}
	catch(System::Exception^ Exception)
	{
		v8::ThrowException(JavascriptInterop::ConvertToV8(Exception->ToString()));
	}

	// return value
	return ConvertToV8(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////