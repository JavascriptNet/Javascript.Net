#include "JavascriptFunction.h"
#include "JavascriptInterop.h"
#include "JavascriptContext.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptFunction::JavascriptFunction( v8::Handle<v8::Object> iFunction, JavascriptContext^ context)
{
	if (!iFunction->IsFunction())
		throw gcnew System::ArgumentException("Trying to use non-function as function");
	
	if(!context)
		throw gcnew System::ArgumentException("Must provide a JavascriptContext");

	mFuncHandle = new Persistent<Function>();
	*mFuncHandle = Persistent<Function>::New(Handle<Function>::Cast(iFunction));
	mContext = context;
}

JavascriptFunction::~JavascriptFunction()
{
	if(mFuncHandle) 
	{
		JavascriptScope scope(mContext);
		mFuncHandle->Dispose();
		delete mFuncHandle;
		mFuncHandle = nullptr;
	}
	System::GC::SuppressFinalize(this);
}

JavascriptFunction::!JavascriptFunction() 
{
	if(mFuncHandle) 
	{
		JavascriptScope scope(mContext);
		mFuncHandle->Dispose();
		delete mFuncHandle;
		mFuncHandle = nullptr;
	}
}

System::Object^ JavascriptFunction::Call(... cli::array<System::Object^>^ args)
{	
	JavascriptScope scope(mContext);
	HandleScope handleScope;

	Handle<v8::Object> global = (*mFuncHandle)->CreationContext()->Global();

	int argc = args->Length;
	Handle<v8::Value> *argv = new Handle<v8::Value>[argc];
	for (int i = 0; i < argc; i++)
	{
		argv[i] = JavascriptInterop::ConvertToV8(args[i]);
	}

	Local<Value> retVal = (*mFuncHandle)->Call(global, argc, argv);

	delete [] argv;
	return JavascriptInterop::ConvertFromV8(retVal);
}

bool JavascriptFunction::operator==( JavascriptFunction^ func1, JavascriptFunction^ func2 )
{
	if(ReferenceEquals(func2, nullptr)) {
		return false;
	}
	Handle<Function> jsFuncPtr1 = *(func1->mFuncHandle);
	Handle<Function> jsFuncPtr2 = *(func2->mFuncHandle);

	return jsFuncPtr1->Equals(jsFuncPtr2);
}

bool JavascriptFunction::Equals( JavascriptFunction^ other )
{
	return this == other;
}

bool JavascriptFunction::Equals(Object^ other )
{
	JavascriptFunction^ otherFunc = dynamic_cast<JavascriptFunction^>(other);
	return (otherFunc && this->Equals(otherFunc));
}

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////