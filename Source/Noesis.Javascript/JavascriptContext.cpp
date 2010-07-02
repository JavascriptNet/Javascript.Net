////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptContext.cpp
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

#include <msclr\lock.h>

#include "JavascriptContext.h"

#include "SystemInterop.h"
#include "JavascriptException.h"
#include "JavascriptExternal.h"
#include "JavascriptInterop.h"

using namespace msclr;

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptContext::JavascriptContext()
{
	mExternals = new vector<JavascriptExternal*>();
	mContext = new Persistent<Context>(Context::New());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptContext::~JavascriptContext()
{
	mContext->Dispose();
	Clear();
	delete mContext;
	delete mExternals;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::SetParameter(System::String^ iName, System::Object^ iObject)
{
	HandleScope handleScope;
	Handle<Value> value;
	string key;
	
	{
		JSScope scope(this);
		key = SystemInterop::ConvertFromSystemString(iName);
		value = JavascriptInterop::ConvertToV8(iObject);
		(*mContext)->Global()->Set(String::New(key.c_str()), value);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptContext::GetParameter(System::String^ iName)
{
	HandleScope handleScope;
	Handle<Value> value;
	string key;
	
	{
		JSScope scope(this);
		key = SystemInterop::ConvertFromSystemString(iName);
		value = (*mContext)->Global()->Get(String::New(key.c_str()));
	}

	return JavascriptInterop::ConvertFromV8(value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Persistent<Script>
JavascriptContext::Compile(System::String^ iSourceCode)
{
	Persistent<Script> script;
	Handle<String> source;

	// convert source
	source = String::New(SystemInterop::ConvertFromSystemString(iSourceCode).c_str());

	// compile
	{
		JSScope scope(this);
		TryCatch tryCatch;

		script = Persistent<Script>::New(Script::Compile(source));

		if (script.IsEmpty())
			throw gcnew JavascriptException(tryCatch);		
		else
			return script;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
JavascriptContext::Run(System::String^ iSourceCode)
{
	HandleScope handleScope;
	Local<Value> ret;
	Local<Script> script;

	{
		lock l(mLock);
		// Compile
		Handle<String> source;

		// convert source
		source = String::New(SystemInterop::ConvertFromSystemString(iSourceCode).c_str());

		// compile
		{
			JSScope scope(this);
			TryCatch tryCatch;

			script = Script::Compile(source);

			if (script.IsEmpty())
				throw gcnew JavascriptException(tryCatch);		
		}
	}
	
	{
		JSScope scope(this);
		TryCatch tryCatch;
		ret = (*script)->Run();

		if (ret.IsEmpty())
			throw gcnew JavascriptException(tryCatch);
	}
	
	return JavascriptInterop::ConvertFromV8(ret);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptContext^
JavascriptContext::GetCurrent()
{
	return sCurrentContext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::Enter()
{
	(*mContext)->Enter();
	sCurrentContext = this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::Exit()
{
	sCurrentContext = nullptr;
	(*mContext)->Exit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JavascriptContext::Clear()
{
	while (mExternals->size())
	{
		delete mExternals->back();
		mExternals->pop_back();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptExternal*
JavascriptContext::WrapObject(System::Object^ iObject)
{
	JavascriptExternal* external = new JavascriptExternal(iObject);
	mExternals->push_back(external);
	return external;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////