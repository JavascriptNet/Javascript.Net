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

#include <vcclr.h>

#include "JavascriptInterop.h"

#include "SystemInterop.h"
#include "JavascriptException.h"
#include "JavascriptExternal.h"
#include "JavascriptFunction.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace System::Collections;
using namespace System::Collections::Generic;

////////////////////////////////////////////////////////////////////////////////////////////////////

Persistent<ObjectTemplate>
JavascriptInterop::NewObjectWrapperTemplate()
{
	HandleScope handleScope;

	Handle<ObjectTemplate> result = ObjectTemplate::New();
	result->SetInternalFieldCount(1);
	result->SetNamedPropertyHandler(Getter, Setter);
	result->SetIndexedPropertyHandler(IndexGetter, IndexSetter);
	return Persistent<ObjectTemplate>::New(handleScope.Close(result));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptInterop::ConvertFromV8(Handle<Value> iValue)
{
	if (iValue->IsNull() || iValue->IsUndefined())
		return nullptr;
	if (iValue->IsBoolean())
		return gcnew System::Boolean(iValue->BooleanValue());
	if (iValue->IsInt32())
		return gcnew System::Int32(iValue->Int32Value());
	if (iValue->IsNumber())
		return gcnew System::Double(iValue->NumberValue());
	if (iValue->IsString()){
		System::String^ test = gcnew System::String((wchar_t*)*String::Value(iValue->ToString()));
		return gcnew System::String((wchar_t*)*String::Value(iValue->ToString()));
	}
	if (iValue->IsArray())
		return ConvertArrayFromV8(iValue);
	if (iValue->IsDate())
		return ConvertDateFromV8(iValue);
	if (iValue->IsFunction())
		return gcnew JavascriptFunction(iValue->ToObject());
	if (iValue->IsObject())
	{
		Handle<Object> object = iValue->ToObject();

		if (object->InternalFieldCount() > 0)
			return UnwrapObject(iValue);
		else
			return ConvertObjectFromV8(object);
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

		if (type->IsValueType)
		{
			// Common types first.
			if (type == System::Int32::typeid)
				return v8::Int32::New(safe_cast<int>(iObject));
			if (type == System::Double::typeid)
				return v8::Number::New(safe_cast<double>(iObject));
			if (type == System::Boolean::typeid)
				return v8::Boolean::New(safe_cast<bool>(iObject));
			if (type->IsEnum)
			{
				// No equivalent to enum, so convert to a string.
				pin_ptr<const wchar_t> valuePtr = PtrToStringChars(iObject->ToString());
				wchar_t* value = (wchar_t*) valuePtr;
				return v8::String::New((uint16_t*)value);
			}
			else
			{
				if (type == System::Char::typeid)
				{
					uint16_t c = (uint16_t)safe_cast<wchar_t>(iObject);
					return v8::String::New(&c, 1);
				}
				if (type == System::Int64::typeid)
					return v8::Number::New((double)safe_cast<long long>(iObject));
				if (type == System::Int16::typeid)
					return v8::Int32::New(safe_cast<short>(iObject));
				if (type == System::SByte::typeid)
					return v8::Int32::New(safe_cast<signed char>(iObject));
				if (type == System::Byte::typeid)
					return v8::Int32::New(safe_cast<unsigned char>(iObject));
				if (type == System::UInt16::typeid)
					return v8::Uint32::New(safe_cast<unsigned short>(iObject));
				if (type == System::UInt32::typeid)
					return v8::Number::New(safe_cast<unsigned int>(iObject));  // I tried v8::Uint32, but it converted MaxInt to -1.
				if (type == System::UInt64::typeid)
					return v8::Number::New((double)safe_cast<unsigned long long>(iObject));
				if (type == System::Single::typeid)
					return v8::Number::New(safe_cast<float>(iObject));
				if (type == System::Decimal::typeid)
					return v8::Number::New((double)safe_cast<System::Decimal>(iObject));
				if (type == System::DateTime::typeid)
					return v8::Date::New(SystemInterop::ConvertFromSystemDateTime(safe_cast<System::DateTime^>(iObject)));
			}
		}
		if (type == System::String::typeid)
		{
			pin_ptr<const wchar_t> valuePtr = PtrToStringChars(safe_cast<System::String^>(iObject));
			wchar_t* value = (wchar_t*) valuePtr;
			return v8::String::New((uint16_t*)value);
		}
		if (type->IsArray)
			return ConvertFromSystemArray(safe_cast<System::Array^>(iObject));
		if (System::Delegate::typeid->IsAssignableFrom(type))
			return ConvertFromSystemDelegate(safe_cast<System::Delegate^>(iObject));
		
		if (type->IsGenericType)
		{
			if(type->GetGenericTypeDefinition() == System::Collections::Generic::Dictionary::typeid)
			{
				return ConvertFromSystemDictionary(iObject);
			}
			if (type->IsGenericType && (type->GetGenericTypeDefinition() == System::Collections::Generic::List::typeid))
				return ConvertFromSystemList(iObject);
		}

		return WrapObject(iObject);
	}

	return Null();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: should return Handle<External>
Handle<Object>
JavascriptInterop::WrapObject(System::Object^ iObject)
{
	JavascriptContext^ context = JavascriptContext::GetCurrent();

	if (context != nullptr)
	{
		Handle<ObjectTemplate> templ = context->GetObjectWrapperTemplate();
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
	 if (iValue->IsExternal())
	{
		Handle<External> external = Handle<External>::Cast(iValue);
		JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
		return wrapper->GetObject();
	}

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

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptInterop::ConvertArrayFromV8(Handle<Value> iValue)
{
	v8::Handle<v8::Array> object = v8::Handle<v8::Array>::Cast(iValue->ToObject());
	int length = object->Length();
	cli::array<System::Object^>^ results = gcnew cli::array<System::Object^>(length);

	// Populate the .NET Array with the v8 Array
	for(int i = 0; i < length; i++)
	{
		results->SetValue(ConvertFromV8(object->Get(i)), i);
	}

	return results;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptInterop::ConvertObjectFromV8(Handle<Object> iObject)
{
	v8::Local<v8::Array> names = iObject->GetPropertyNames();
	
	unsigned int length = names->Length();
	Dictionary<System::String^, System::Object^>^ results = gcnew Dictionary<System::String^, System::Object^>(length);
	for (unsigned int i = 0; i < length; i++) {
		v8::Handle<v8::Value> nameKey = v8::Uint32::New(i);
		v8::Handle<v8::Value> propName = names->Get(nameKey);
		v8::Handle<v8::Value> propValue = iObject->Get(propName);

		// Property "names" may be integers or other types.  However they will
		// generally be strings so continuing to key this dictionary that way is 
		// probably OK.
		System::String^ key = safe_cast<System::String^>(ConvertFromV8(propName)->ToString());
		results[key] = ConvertFromV8(propValue);
	}

	return results;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::DateTime^
JavascriptInterop::ConvertDateFromV8(Handle<Value> iValue)
{
	System::DateTime^ startDate = gcnew System::DateTime(1970, 1, 1);
	double milliseconds = iValue->NumberValue();
	System::TimeSpan^ timespan = System::TimeSpan::FromMilliseconds(milliseconds);
	return System::DateTime(timespan->Ticks + startDate->Ticks).ToLocalTime();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value>
JavascriptInterop::ConvertFromSystemArray(System::Array^ iArray) 
{
	int lenght = iArray->Length;
	v8::Handle<v8::Array> result = v8::Array::New();
	
	// Transform the .NET array into a Javascript array 
	for (int i = 0; i < lenght; i++) 
	{
		v8::Handle<v8::Value> key = v8::Int32::New(i);
		result->Set(key, ConvertToV8(iArray->GetValue(i)));
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value>
JavascriptInterop::ConvertFromSystemDictionary(System::Object^ iObject) 
{
	v8::Handle<v8::Object> object = v8::Object::New();
	System::Collections::IDictionary^ dictionary =  safe_cast<System::Collections::IDictionary^>(iObject);

	for each(System::Object^ keyValue in dictionary->Keys) 
	{
		v8::Handle<v8::Value> key = ConvertToV8(keyValue);
		v8::Handle<v8::Value> val = ConvertToV8(dictionary[keyValue]);
		object->Set(key, val);
	} 

	return object;
}	

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value>
JavascriptInterop::ConvertFromSystemList(System::Object^ iObject) 
{
	v8::Handle<v8::Array> object = v8::Array::New();
	System::Collections::IList^ list =  safe_cast<System::Collections::IList^>(iObject);

	for(int i = 0; i < list->Count; i++) 
	{
		v8::Handle<v8::Value> key = v8::Int32::New(i);
		v8::Handle<v8::Value> val = ConvertToV8(list[i]);
		object->Set(key, val);
	} 

	return object;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value>
JavascriptInterop::ConvertFromSystemDelegate(System::Delegate^ iDelegate) 
{
	JavascriptContext^ context = JavascriptContext::GetCurrent();
	v8::Handle<v8::External> external = v8::External::New(context->WrapObject(iDelegate));

	v8::Handle<v8::FunctionTemplate> method = v8::FunctionTemplate::New(DelegateInvoker, external);
	return method->GetFunction();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> 
JavascriptInterop::DelegateInvoker(const v8::Arguments& info)
{
	JavascriptExternal* wrapper = (JavascriptExternal*)v8::Handle<v8::External>::Cast(info.Data())->Value();
	System::Object^ object = wrapper->GetObject();

	System::Delegate^ delegat = static_cast<System::Delegate^>(object);
	int nparams = delegat->GetType()->GetMethods()[0]->GetParameters()->Length;

	// As is normal in JavaScript, we ignore excess input parameters, and pad
	// with null if insufficient are supplied.
	int nsupplied = info.Length();
	cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(nparams);
	for (int i = 0; i < nparams; i++) 
	{
		if (i < nsupplied)
			args[i] = ConvertFromV8(info[i]);
		else
			args[i] = nullptr;
	}

	System::Object^ ret;
	try
	{
		// invoke
		ret = delegat->DynamicInvoke(args);
	}
	catch(System::Reflection::TargetInvocationException^ exception)
	{
		v8::ThrowException(JavascriptInterop::ConvertToV8(exception->InnerException));
	}
	catch(System::ArgumentException^)
	{
		// This is what we get when the arguments cannot be converted to match the
		// delegate's requirements.  Its message is all about C# types so I don't
		// pass it on.
		return v8::ThrowException(JavascriptInterop::ConvertToV8("Argument mismatch"));
	}
	catch(System::Exception^ exception)
	{
		v8::ThrowException(JavascriptInterop::ConvertToV8(exception));
	}

	return ConvertToV8(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JavascriptInterop::IsSystemObject(Handle<Value> iValue)
{
	if (iValue->IsObject())
	{
		Local<Object> object = iValue->ToObject();
		return (object->InternalFieldCount() > 0);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::Getter(Local<String> iName, const AccessorInfo &iInfo)
{
	wstring name = (wchar_t*) *String::Value(iName);
	Handle<External> external = Handle<External>::Cast(iInfo.Holder()->GetInternalField(0));
	JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
	Handle<Function> function;
	Handle<Value> value;

	// get method
	function = wrapper->GetMethod(name);
	if (!function.IsEmpty())
		return function;  // good value or exception

	// As for GetMethod().
	if (wrapper->GetProperty(name, value))
		return value;  // good value or exception

	// map toString with ToString
	if (wstring((wchar_t*) *String::Value(iName)) == L"toString")
	{
		function = wrapper->GetMethod(L"ToString");
		if (!function.IsEmpty())
			return function;
	}

	// member not found
	if ((wrapper->GetOptions() & SetParameterOptions::RejectUnknownProperties) == SetParameterOptions::RejectUnknownProperties)
		return v8::ThrowException(JavascriptInterop::ConvertToV8("Unknown member: " + gcnew System::String((wchar_t*) *String::Value(iName))));
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::Setter(Local<String> iName, Local<Value> iValue, const AccessorInfo& iInfo)
{
	wstring name = (wchar_t*) *String::Value(iName);
	Handle<External> external = Handle<External>::Cast(iInfo.Holder()->GetInternalField(0));
	JavascriptExternal* wrapper = (JavascriptExternal*) external->Value();
	
	// set property
	return wrapper->SetProperty(name, iValue);
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
	return Handle<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Handle<Value>
JavascriptInterop::Invoker(const v8::Arguments& iArgs)
{
	System::Object^ data = UnwrapObject(Handle<External>::Cast(iArgs.Data()));
	System::Reflection::MethodInfo^ bestMethod;
	cli::array<System::Object^>^ suppliedArguments;
	cli::array<System::Object^>^ bestMethodArguments;
	cli::array<System::Object^>^ objectInfo;
	int bestMethodMatchedArgs = -1;
	System::Object^ ret;

	// get target and member's name
	objectInfo = safe_cast<cli::array<System::Object^>^>(data);
	System::Object^ self = objectInfo[0];
	// System::Object^ holder = UnwrapObject(iArgs.Holder());
	System::Type^ holderType = self->GetType(); 
	
	// get members
	System::Type^ type = self->GetType();
	System::String^ memberName = (System::String^)objectInfo[1];
	cli::array<System::Reflection::MemberInfo^>^ members = type->GetMember(memberName);

	if (members->Length > 0 && members[0]->MemberType == System::Reflection::MemberTypes::Method)
	{
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
					System::Type^ paramType = parametersInfo[p]->ParameterType;

					if (suppliedArguments[p] != nullptr)
					{
						System::Type^ suppliedType = suppliedArguments[p]->GetType();

						if (suppliedType == paramType)
						{
							arguments[p] = suppliedArguments[p];
							match++;
						}
						else
						{
							arguments[p] = SystemInterop::ConvertToType(suppliedArguments[p], paramType);
							if (arguments[p] == nullptr)
							{
								failed++;
								break;
							}
						}
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
	}

	if (bestMethod != nullptr)
	{
		try
		{
			// invoke
			ret = bestMethod->Invoke(self, bestMethodArguments);
		}
		catch(System::Reflection::TargetInvocationException^ exception)
		{
			v8::ThrowException(JavascriptInterop::ConvertToV8(exception->InnerException));
		}
		catch(System::Exception^ exception)
		{
			v8::ThrowException(JavascriptInterop::ConvertToV8(exception));
		}
	}
	else
		v8::ThrowException(JavascriptInterop::ConvertToV8("Argument mismatch for method \"" + memberName + "\"."));
	
	// return value
	return ConvertToV8(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////