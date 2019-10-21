////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptException.cpp
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

#include "JavascriptException.h"
#include "JavascriptContext.h"
#include "JavascriptInterop.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace v8;

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptException::JavascriptException(TryCatch& iTryCatch): System::Exception(GetExceptionMessage(iTryCatch), GetSystemException(iTryCatch))
{
	v8::Local<v8::Message> message = iTryCatch.Message();
	if (!message.IsEmpty())
	{
		mSource = gcnew System::String((wchar_t*) *String::Value(JavascriptContext::GetCurrentIsolate(), message->GetScriptResourceName()));
		mLine = message->GetLineNumber(JavascriptContext::GetCurrentIsolate()->GetCurrentContext()).ToChecked();
		mStartColumn = message->GetStartColumn();
		mEndColumn = message->GetEndColumn();
	}

	// This causes an "Data is not serializable" exception sometimes, I think
	// when it contains an InnerException.
	//v8::Local<v8::Value> ex = iTryCatch.Exception();
	//this->Data->Add("V8Exception", JavascriptInterop::ConvertFromV8(ex));
	if (!message.IsEmpty())
	{
		v8::String::Utf8Value sourceline(JavascriptContext::GetCurrentIsolate(), message->GetSourceLine(JavascriptContext::GetCurrentIsolate()->GetCurrentContext()).ToLocalChecked());
		System::String^ sourceLineStr = gcnew System::String((const char*)*sourceline);
		this->Data->Add("V8SourceLine", sourceLineStr);
	}
	v8::MaybeLocal<v8::Value> stackTrace = iTryCatch.StackTrace(JavascriptContext::GetCurrentIsolate()->GetCurrentContext());
	if (!stackTrace.IsEmpty())
	{
		this->Data->Add("V8StackTrace", JavascriptInterop::ConvertFromV8(stackTrace.ToLocalChecked()));
	}
}

JavascriptException::JavascriptException(wchar_t const *complaint): System::Exception(gcnew System::String(complaint))
{
	mSource = System::String::Empty;
	mLine = -1;
	mStartColumn = -1;
	mEndColumn = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^
JavascriptException::Source::get()
{
	return mSource;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
JavascriptException::Line::get()
{
	return mLine;
}

int
JavascriptException::StartColumn::get()
{
	return mStartColumn;
}

int
JavascriptException::EndColumn::get()
{
	return mEndColumn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^
JavascriptException::GetExceptionMessage(TryCatch& iTryCatch)
{
	System::Exception^ exception = GetSystemException(iTryCatch);
	if (exception != nullptr)
	{
		return gcnew System::String(exception->Message);
	}
	else
	{
		if (iTryCatch.HasTerminated())
			return gcnew System::String(L"Execution Terminated");
		
        String::Value stringValue(JavascriptContext::GetCurrentIsolate(), iTryCatch.Exception());
        // Using a constructor which takes a length makes sure that we don't discard zero bytes in the middle of the string
        return gcnew System::String((wchar_t*)* stringValue, 0, stringValue.length());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Exception^
JavascriptException::GetSystemException(TryCatch& iTryCatch)
{
	// If an exception was thrown by C# code that we previously invoked
	// then we will have wrapped the original Exception object and
	// stuck it in the InnerException property.  Let's get it out
	// again.
	v8::Local<v8::Value> v8exception = iTryCatch.Exception();
	if (v8exception->IsObject()) {
		v8::Local<v8::Object> exception_o = v8::Local<v8::Object>::Cast(v8exception);
        auto isolate = JavascriptContext::GetCurrentIsolate();
        auto context = isolate->GetCurrentContext();
		v8::Local<v8::String> inner_exception_str = v8::String::NewFromUtf8(isolate, "InnerException", v8::NewStringType::kNormal).ToLocalChecked();
		if (exception_o->HasOwnProperty(context, inner_exception_str).FromMaybe(false)) {
			v8::Local<v8::Value> inner = exception_o->Get(context, inner_exception_str).ToLocalChecked();
			System::Object^ object = JavascriptInterop::UnwrapObject(inner);
			return dynamic_cast<System::Exception^>(object);
		}
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////