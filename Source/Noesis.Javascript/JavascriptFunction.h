#pragma once

//////////////////////////////////////////////////////////////////////////

#include <v8.h>

#include "JavascriptContext.h"

using namespace v8;

//////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// JavascriptFunction
//
// Wraps around a JS function when passed back to C#, allowing it to be
// called from C#.  Callers must not dispose of their JavascriptContext
// while they still have references to JavascriptFunctions.
//////////////////////////////////////////////////////////////////////////
public ref class JavascriptFunction
{
public:
	JavascriptFunction(v8::Local<v8::Object> iFunction, JavascriptContext^ context);
	~JavascriptFunction();
    !JavascriptFunction();

	System::Object^ Call(... cli::array<System::Object^>^ args);

	static bool operator== (JavascriptFunction^ func1, JavascriptFunction^ func2);
	bool Equals(JavascriptFunction^ other);
	virtual bool Equals(Object^ other) override;
	
    virtual System::String^ ToString() override;

private:
	v8::Persistent<v8::Function>* mFuncHandle;
	JavascriptContext^ mContext;
};

//////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

//////////////////////////////////////////////////////////////////////////