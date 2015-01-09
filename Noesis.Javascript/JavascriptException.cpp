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
		mSource = gcnew System::String((wchar_t*) *String::Value(message->GetScriptResourceName()));
		mLine = message->GetLineNumber();
		mStartColumn = message->GetStartColumn();
		mEndColumn = message->GetEndColumn();
	}

	// This causes an "Data is not serializable" exception sometimes, I think
	// when it contains an InnerException.
	//v8::Local<v8::Value> ex = iTryCatch.Exception();
	//this->Data->Add("V8Exception", JavascriptInterop::ConvertFromV8(ex));
	if (!message.IsEmpty())
	{
		v8::String::Utf8Value sourceline(message->GetSourceLine());
		System::String^ sourceLineStr = gcnew System::String((const char*)*sourceline);
		this->Data->Add("V8SourceLine", sourceLineStr);
	}
	v8::Local<v8::Value> stackTrace = iTryCatch.StackTrace();
	if (!stackTrace.IsEmpty())
	{
		this->Data->Add("V8StackTrace", JavascriptInterop::ConvertFromV8(stackTrace));
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
	// Adding this location information is unhelpful because the library user can
	// get the information from Line/StartColumn/EndColumn, and may not want it
	// entangled with the message, or may want to localise it, or whatever.
	//System::String^ location;
    //
	//v8::Local<v8::Message> message = iTryCatch.Message();
	//if (!message.IsEmpty())
	//	location = gcnew System::String((wchar_t*) *String::Value(message->GetScriptResourceName())) + ", line " + message->GetLineNumber();
	//else
	//	location = gcnew System::String("Unknown location");
	
	System::Exception^ exception = GetSystemException(iTryCatch);
	if (exception != nullptr)
	{
		return gcnew System::String(exception->Message /*+ " (" + location + ")."*/);
	}
	else
	{
		if (iTryCatch.Exception()->IsNull())
			// We get a null exception here when execution is terminated.
			// Documentation shows a HasTerminated() method on TryCatch,
			// but perhaps our copy of v8 is too old.
			return gcnew System::String(L"Execution Terminated");
		else
			return gcnew System::String((wchar_t*) *String::Value(iTryCatch.Exception())) /*+ " (" + location + ")"*/;
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
		v8::Handle<v8::Object> exception_o = v8::Handle<v8::Object>::Cast(v8exception);
		v8::Handle<v8::String> inner_exception_str = v8::String::NewFromUtf8(JavascriptContext::GetCurrentIsolate(), "InnerException");
		if (exception_o->HasOwnProperty(inner_exception_str)) {
			v8::Handle<v8::Value> inner = exception_o->Get(inner_exception_str);
			System::Object^ object = JavascriptInterop::UnwrapObject(inner);
			return dynamic_cast<System::Exception^>(object);
		}
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////