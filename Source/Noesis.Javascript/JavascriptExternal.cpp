////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptExternal.cpp
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

#include "JavascriptExternal.h"

#include "JavascriptContext.h"
#include "JavascriptInterop.h"
#include "JavascriptException.h"
#include "SystemInterop.h"

#include <stdio.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace System::Reflection;

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptExternal::JavascriptExternal(System::Object^ iObject)
{
    System::Runtime::InteropServices::GCHandle handle = System::Runtime::InteropServices::GCHandle::Alloc(iObject, System::Runtime::InteropServices::GCHandleType::Normal);
    mObjectHandle = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle);
    mOptions = SetParameterOptions::None;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptExternal::~JavascriptExternal()
{
    if (mObjectHandle != System::IntPtr::Zero) {
        System::Runtime::InteropServices::GCHandle handle = System::Runtime::InteropServices::GCHandle::FromIntPtr(mObjectHandle);
        handle.Free();
        mObjectHandle = System::IntPtr::Zero;
    }
    if (!mPersistent.IsEmpty()) {
        mPersistent.ClearWeak<void>();
        mPersistent.Reset();
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GCCallback(const WeakCallbackInfo<JavascriptExternal>& data)
{
    auto context = JavascriptContext::GetCurrent();
    auto external = data.GetParameter();
    auto object = external->GetObject();

    if (object != nullptr) {
        if (context->mExternals->ContainsKey(object)) {
            context->mExternals->Remove(object);
        }
    }
    delete external;
}

void
JavascriptExternal::InitializePersistent(Isolate* isolate, Local<Object> object)
{
    object->SetInternalField(0, External::New(isolate, this));
    mPersistent.Reset(isolate, object);
    mPersistent.SetWeak(this, &GCCallback, WeakCallbackType::kParameter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Object>
JavascriptExternal::ToLocal(Isolate* isolate)
{
    if (!mPersistent.IsEmpty())
        return Local<Object>::New(isolate, mPersistent);
    
    auto context = JavascriptContext::GetCurrent();

    EscapableHandleScope scope(isolate);

    Local<FunctionTemplate> templ = context->GetObjectWrapperConstructorTemplate(GetObject()->GetType());
    Local<ObjectTemplate> instanceTemplate = templ->InstanceTemplate();
    Local<Object> object = instanceTemplate->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    InitializePersistent(isolate, object);

    return scope.Escape(object);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptExternal::GetObject()
{
    // .NET 8: Convert IntPtr back to GCHandle to access Target
    if (mObjectHandle == System::IntPtr::Zero)
        return nullptr;
    System::Runtime::InteropServices::GCHandle handle = System::Runtime::InteropServices::GCHandle::FromIntPtr(mObjectHandle);
    if (!handle.IsAllocated)
        return nullptr;
    return handle.Target;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Function>
JavascriptExternal::GetMethod(wstring iName)
{
    auto context = JavascriptContext::GetCurrent();
    auto isolate = JavascriptContext::GetCurrentIsolate();

    auto type = GetObject()->GetType();
    auto memberName = gcnew System::String(iName.c_str());
    auto uniqueMethodName = type->AssemblyQualifiedName + L"." + memberName;

    if (context->mMethods->ContainsKey(uniqueMethodName))
        return Local<Function>::New(isolate, *context->mMethods[uniqueMethodName].Pointer);
	
    // Verification if it is a method
    auto members = type->GetMember(memberName);
    if (members->Length > 0 && members[0]->MemberType == MemberTypes::Method)
    {
        // Store both the method name AND the target object wrapper in function data
        // This ensures we can find the correct object even when called from different contexts
        auto externalPtr = External::New(isolate, this);
        auto dataArray = v8::Array::New(isolate, 2);
        dataArray->Set(isolate->GetCurrentContext(), 0, JavascriptInterop::ConvertToV8(memberName)).ToChecked();
        dataArray->Set(isolate->GetCurrentContext(), 1, externalPtr).ToChecked();
        
        auto functionTemplate = FunctionTemplate::New(isolate, JavascriptInterop::Invoker, dataArray);
        auto function = functionTemplate->GetFunction(isolate->GetCurrentContext()).ToLocalChecked();
        context->mMethods[uniqueMethodName] = WrappedMethod(new Persistent<Function>(isolate, function));
        return function;
    }
	
	// Wasn't an method
	return Local<Function>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Function>
JavascriptExternal::GetMethod(Local<String> iName)
{
	return GetMethod((wchar_t*) *String::Value(JavascriptContext::GetCurrentIsolate(), iName));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns false is no such property exists, otherwise check 'result'
// for an empty value (exception) or the value (including null)
bool
JavascriptExternal::GetProperty(wstring iName, Local<Value> &result)
{
	System::Object^ self = GetObject();
	System::Type^ type = self->GetType();
	PropertyInfo^ propertyInfo = type->GetProperty(gcnew System::String(iName.c_str()));

	v8::Isolate *isolate = JavascriptContext::GetCurrentIsolate();
	try
	{
		if (propertyInfo == nullptr)
		{
			//may have an indexer
			PropertyInfo^ indexerInfo = type->GetProperty("Item", System::Object::typeid, gcnew cli::array<System::Type^> { System::String::typeid });
			if (indexerInfo == nullptr)
			{
				return false;
			}
			if (!indexerInfo->CanRead)
			{
				result = isolate->ThrowException(JavascriptInterop::ConvertToV8("Property " + gcnew System::String(iName.c_str()) + " may not be read."));
			}
			else
			{
				result = JavascriptInterop::ConvertToV8(indexerInfo->GetValue(self, gcnew cli::array<System::String^> { gcnew System::String(iName.c_str()) }));
			}
			return true;
		}

		if (!propertyInfo->CanRead)
		{
			result = isolate->ThrowException(JavascriptInterop::ConvertToV8("Property " + gcnew System::String(iName.c_str()) + " may not be read."));
		}
		else
		{
			result = JavascriptInterop::ConvertToV8(propertyInfo->GetValue(self, nullptr));
		}
	}
	catch (System::Reflection::TargetInvocationException^ exception)
	{
		result = JavascriptInterop::HandleTargetInvocationException(exception);
	}
	catch (System::Exception^ exception)
	{
		result = isolate->ThrowException(JavascriptInterop::ConvertToV8(exception));
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Value>
JavascriptExternal::GetProperty(uint32_t iIndex)
{
	System::Object^ self = GetObject();
	System::Type^ type = self->GetType();
	cli::array<PropertyInfo^>^ propertyInfo = type->GetProperties();
	int index = iIndex;

	// Check if it an array
	if(type->IsArray)
	{
		System::Array^ objectArray = (System::Array^)self;
		return JavascriptInterop::ConvertToV8(objectArray->GetValue(index));
	} 
	
	// Check if it an object
	if(type->IsClass)
	{
		try
		{
			cli::array<int^>^ args = gcnew cli::array<int^>(1);
			args[0] = index;
			System::Reflection::PropertyInfo^ item_info = type->GetProperty("Item", gcnew cli::array<System::Type^> { int::typeid });
			if (item_info == nullptr || item_info->GetIndexParameters()->Length != 1)
				// No indexed property.
				return Local<Value>();  // v8 will return null
			System::Object^ object = type->InvokeMember("Item", System::Reflection::BindingFlags::GetProperty, nullptr, self, args,  nullptr);
			return JavascriptInterop::ConvertToV8(object);
		}
		catch(System::Reflection::TargetInvocationException^ exception)
		{
			return JavascriptInterop::HandleTargetInvocationException(exception);
		}
		catch(System::Exception^ Exception)
		{
			return JavascriptContext::GetCurrentIsolate()->ThrowException(JavascriptInterop::ConvertToV8(Exception));
		}
	}

	// No array or indexer, return null and throw an exception
	return Local<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Value>
JavascriptExternal::SetProperty(wstring iName, Local<Value> iValue)
{
	System::Object^ self = GetObject();
	System::Type^ type = self->GetType();
	PropertyInfo^ propertyInfo = type->GetProperty(gcnew System::String(iName.c_str()));

	v8::Isolate *isolate = JavascriptContext::GetCurrentIsolate();

	try
	{
		if (propertyInfo == nullptr)
		{
			//may have an indexer
			PropertyInfo^ indexerInfo = type->GetProperty("Item", System::Object::typeid, gcnew cli::array<System::Type^> { System::String::typeid });
			if (indexerInfo == nullptr)
			{
				if ((mOptions & SetParameterOptions::RejectUnknownProperties) == SetParameterOptions::RejectUnknownProperties)
					return isolate->ThrowException(JavascriptInterop::ConvertToV8("Unknown member: " + gcnew System::String(iName.c_str())));
				return Local<Value>();
			}
			if (!indexerInfo->CanWrite)
			{
				return isolate->ThrowException(JavascriptInterop::ConvertToV8("Property " + gcnew System::String(iName.c_str()) + " may not be set."));
			}
			else
			{
				indexerInfo->SetValue(self, JavascriptInterop::ConvertFromV8(iValue), gcnew cli::array<System::String^> { gcnew System::String(iName.c_str()) });
			}
			return iValue;
		}

		System::Object^ value = JavascriptInterop::ConvertFromV8(iValue);
		if (value != nullptr) {
			System::Type^ valueType = value->GetType();
			System::Type^ propertyType = propertyInfo->PropertyType;

			// attempt conversion if assigned value is of wrong type
			if (propertyType != valueType && !propertyType->IsAssignableFrom(valueType))
				value = SystemInterop::ConvertToType(value, propertyType);
		}

		if (!propertyInfo->CanWrite)
		{
			return isolate->ThrowException(JavascriptInterop::ConvertToV8("Property " + gcnew System::String(iName.c_str()) + " may not be set."));
		}
		else
		{
			propertyInfo->SetValue(self, value, nullptr);
			// We used to convert and return propertyInfo->GetValue() here.
			// I don't know why we did, but I stopped it because CanRead
			// might be false, which should not stop us _setting_.
			// Also it wastes precious CPU time.
			return iValue;
		}
	}
	catch (System::Reflection::TargetInvocationException^ exception)
	{
		return JavascriptInterop::HandleTargetInvocationException(exception);
	}
	catch (System::Exception^ exception)
	{
		return isolate->ThrowException(JavascriptInterop::ConvertToV8(exception));
	}

	return Local<Value>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Local<Value>
JavascriptExternal::SetProperty(uint32_t iIndex, Local<Value> iValue)
{
	System::Object^ self = GetObject();
	System::Type^ type = self->GetType();
	cli::array<PropertyInfo^>^ propertyInfo = type->GetProperties();
	int index = iIndex;

	// Check if it an array or an indexer
	if(type->IsArray)
	{
		System::Array^ objectArray = (System::Array^)self;
		objectArray->SetValue(JavascriptInterop::ConvertFromV8(iValue), index);
		return JavascriptInterop::ConvertToV8(objectArray->GetValue(index));
	} 
	else
	{
		v8::Isolate *isolate = JavascriptContext::GetCurrentIsolate();
		try
		{
			System::Reflection::PropertyInfo^ item_info = type->GetProperty("Item", gcnew cli::array<System::Type^> { int::typeid });
			if (item_info == nullptr || item_info->GetIndexParameters()->Length != 1) {
				return isolate->ThrowException(JavascriptInterop::ConvertToV8("No public integer-indexed property."));
			} else {
				cli::array<System::Object^>^ index_args = gcnew cli::array<System::Object^>(1);
				index_args[0] = index;
				item_info->SetValue(self, JavascriptInterop::ConvertFromV8(iValue), index_args);
			}
		}
		catch(System::Reflection::TargetInvocationException^ exception)
		{
			return JavascriptInterop::HandleTargetInvocationException(exception);
		}
		catch(System::Exception^ exception)
		{
			return isolate->ThrowException(JavascriptInterop::ConvertToV8(exception));
		}
	}

	return Local<Value>();
}

Local<Function> JavascriptExternal::GetIterator()
{
    auto context = JavascriptContext::GetCurrent();
    auto type = GetObject()->GetType();
    auto uniqueMethodName = type->AssemblyQualifiedName + L".$$Iterator";

    auto isolate = JavascriptContext::GetCurrentIsolate();
    if (context->mMethods->ContainsKey(uniqueMethodName))
        return Local<Function>::New(isolate, *context->mMethods[uniqueMethodName].Pointer);

    auto functionTemplate = FunctionTemplate::New(isolate, JavascriptExternal::IteratorCallback);
    auto function = functionTemplate->GetFunction(isolate->GetCurrentContext()).ToLocalChecked();
    context->mMethods[uniqueMethodName] = WrappedMethod(new Persistent<Function>(isolate, function));
    return function;
}

void JavascriptExternal::IteratorCallback(const v8::FunctionCallbackInfo<Value>& iArgs)
{
    auto isolate = iArgs.GetIsolate();

    auto iterator = ObjectTemplate::New(isolate);
    iterator->SetInternalFieldCount(1);
    auto functionTemplate = FunctionTemplate::New(isolate, JavascriptExternal::IteratorNextCallback);
    iterator->Set(String::NewFromUtf8(isolate, "next").ToLocalChecked(), functionTemplate);
    auto iteratorInstance = iterator->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();

    auto internalField = iArgs.This()->GetInternalField(0).As<Value>().As<External>();
    auto external = (JavascriptExternal*)internalField->Value();
    auto enumerable = (System::Collections::IEnumerable^)external->GetObject();
    auto enumerator = enumerable->GetEnumerator();

    auto context = JavascriptContext::GetCurrent();
    auto enumeratorExternal = context->WrapObject(enumerator);
    enumeratorExternal->InitializePersistent(isolate, iteratorInstance);

    iArgs.GetReturnValue().Set(iteratorInstance);
}

void JavascriptExternal::IteratorNextCallback(const v8::FunctionCallbackInfo<Value>& iArgs)
{
    auto isolate = iArgs.GetIsolate();

    auto internalField = iArgs.This()->GetInternalField(0).As<Value>().As<External>();
    auto external = (JavascriptExternal*)internalField->Value();
    auto enumerator = (System::Collections::IEnumerator^) external->GetObject();

    try
    {
        // MoveNext might throw in which case we need to schedule a JS exception with the isolate otherwise the JS code
        // can't catch that error and the script execution terminates immediately instead.
        auto done = !enumerator->MoveNext();
        auto resultTemplate = ObjectTemplate::New(isolate);
        auto result = resultTemplate->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
        result->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "done").ToLocalChecked(), JavascriptInterop::ConvertToV8(done));
        if (!done)
            result->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "value").ToLocalChecked(), JavascriptInterop::ConvertToV8(enumerator->Current));
        iArgs.GetReturnValue().Set(result);
    }
    catch (System::Exception ^exception)
    {
        // If we catch a .NET exception we schedule it with the isolate and set the resulting object as the return value
        // of the callback. This automatically causes the iterator protocol to be fulfilled correctly. We must set the
        // JS exception object as the return value to get correct source and line information in addition to the
        // stacktrace.
        auto result = isolate->ThrowException(JavascriptInterop::ConvertToV8(exception));
        iArgs.GetReturnValue().Set(result);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////
