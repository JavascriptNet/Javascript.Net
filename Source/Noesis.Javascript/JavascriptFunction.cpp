#include "JavascriptFunction.h"
#include "JavascriptInterop.h"
#include "JavascriptContext.h"
#include "JavascriptException.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

JavascriptFunction::JavascriptFunction(v8::Local<v8::Object> iFunction, JavascriptContext^ context)
{
	if (!iFunction->IsFunction())
		throw gcnew System::ArgumentException("Trying to use non-function as function");
	
	if(!context)
		throw gcnew System::ArgumentException("Must provide a JavascriptContext");

	mFuncHandle = new Persistent<Function>(context->GetCurrentIsolate(), Local<Function>::Cast(iFunction));
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

JavascriptFunction::!JavascriptFunction()
{
    delete this;
}

System::Object^ JavascriptFunction::Call(... cli::array<System::Object^>^ args)
{
    if (mFuncHandle == nullptr)
        throw gcnew JavascriptException(L"This function's owning JavascriptContext has been disposed");
    if (!args)
        throw gcnew System::ArgumentNullException("args");
	
    JavascriptScope scope(mContext);
	v8::Isolate* isolate = mContext->GetCurrentIsolate();
	HandleScope handleScope(isolate);

	Local<v8::Object> global = mContext->GetGlobal();

	int argc = args->Length;
	Local<v8::Value> *argv = new Local<v8::Value>[argc];
	for (int i = 0; i < argc; i++)
	{
		argv[i] = JavascriptInterop::ConvertToV8(args[i]);
	}

	TryCatch tryCatch(isolate);
	MaybeLocal<Value> retVal = mFuncHandle->Get(isolate)->Call(isolate->GetCurrentContext(), global, argc, argv);
	if (retVal.IsEmpty())
		throw gcnew JavascriptException(tryCatch);

	delete [] argv;
	return JavascriptInterop::ConvertFromV8(retVal.ToLocalChecked());
}

bool JavascriptFunction::operator==(JavascriptFunction^ func1, JavascriptFunction^ func2)
{
    bool func1_null = ReferenceEquals(func1, nullptr),
         func2_null = ReferenceEquals(func2, nullptr);
    if (func1_null != func2_null)
		return false;
    if (func1_null && func2_null)
        return true;
    if (func1->mFuncHandle == nullptr)
        throw gcnew JavascriptException(L"'func1's owning JavascriptContext has been disposed");
    if (func2->mFuncHandle == nullptr)
        throw gcnew JavascriptException(L"'func2's owning JavascriptContext has been disposed");

	Local<Function> jsFuncPtr1 = func1->mFuncHandle->Get(func1->mContext->GetCurrentIsolate());
	Local<Function> jsFuncPtr2 = func2->mFuncHandle->Get(func2->mContext->GetCurrentIsolate());

	return jsFuncPtr1->Equals(JavascriptContext::GetCurrentIsolate()->GetCurrentContext(), jsFuncPtr2).ToChecked();
}

bool JavascriptFunction::Equals(JavascriptFunction^ other)
{
	return this == other;
}

bool JavascriptFunction::Equals(Object^ other)
{
    if (mFuncHandle == nullptr)
        throw gcnew JavascriptException(L"This function's owning JavascriptContext has been disposed");
    JavascriptFunction^ otherFunc = dynamic_cast<JavascriptFunction^>(other);
    if (otherFunc != nullptr && otherFunc->mFuncHandle == nullptr)
        throw gcnew JavascriptException(L"This function's owning JavascriptContext has been disposed");

	return (otherFunc && this->Equals(otherFunc));
}

System::String^ JavascriptFunction::ToString()
{
    if (mFuncHandle == nullptr)
        throw gcnew JavascriptException(L"This function's owning JavascriptContext has been disposed");
   
    JavascriptScope scope(mContext);
    auto isolate = mContext->GetCurrentIsolate();
    HandleScope handleScope(isolate);
    auto asString = mFuncHandle->Get(isolate)->ToString(isolate->GetCurrentContext());
    return safe_cast<System::String^>(JavascriptInterop::ConvertFromV8(asString.ToLocalChecked()));
}

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////