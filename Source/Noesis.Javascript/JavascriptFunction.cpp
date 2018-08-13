#include "JavascriptFunction.h"
#include "JavascriptInterop.h"
#include "JavascriptContext.h"
#include "JavascriptException.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptFunction::JavascriptFunction(v8::Handle<v8::Object> iFunction, JavascriptContext^ context)
{
	if (!iFunction->IsFunction())
		throw gcnew System::ArgumentException("Trying to use non-function as function");
	
	if(!context)
		throw gcnew System::ArgumentException("Must provide a JavascriptContext");

	mFuncHandle = new Persistent<Function>(context->GetCurrentIsolate(), Handle<Function>::Cast(iFunction));
	mContext = context;

	mContext->RegisterFunction(this);
}

JavascriptFunction::~JavascriptFunction()
{
	if(mFuncHandle) 
	{
		if (mContext)
		{
			JavascriptScope scope(mContext);
			mFuncHandle->Reset();
		}
		delete mFuncHandle;
		mFuncHandle = nullptr;
	}
}

System::Object^ JavascriptFunction::Call(... cli::array<System::Object^>^ args)
{
	JavascriptScope scope(mContext);
	v8::Isolate* isolate = mContext->GetCurrentIsolate();
	HandleScope handleScope(isolate);

	Handle<v8::Object> global = mContext->GetGlobal();

	int argc = args->Length;
	Handle<v8::Value> *argv = new Handle<v8::Value>[argc];
	for (int i = 0; i < argc; i++)
	{
		argv[i] = JavascriptInterop::ConvertToV8(args[i]);
	}

	TryCatch tryCatch(isolate);
	Local<Value> retVal = mFuncHandle->Get(isolate)->Call(global, argc, argv);
	if (retVal.IsEmpty())
		throw gcnew JavascriptException(tryCatch);

	delete [] argv;
	return JavascriptInterop::ConvertFromV8(retVal);
}

bool JavascriptFunction::operator==(JavascriptFunction^ func1, JavascriptFunction^ func2)
{
	if(ReferenceEquals(func2, nullptr)) {
		return false;
	}
	Handle<Function> jsFuncPtr1 = func1->mFuncHandle->Get(func1->mContext->GetCurrentIsolate());
	Handle<Function> jsFuncPtr2 = func2->mFuncHandle->Get(func2->mContext->GetCurrentIsolate());

	return jsFuncPtr1->Equals(jsFuncPtr2);
}

bool JavascriptFunction::Equals(JavascriptFunction^ other)
{
	return this == other;
}

bool JavascriptFunction::Equals(Object^ other)
{
	JavascriptFunction^ otherFunc = dynamic_cast<JavascriptFunction^>(other);
	return (otherFunc && this->Equals(otherFunc));
}

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////