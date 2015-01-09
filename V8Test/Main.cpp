#include <iostream>
#include <v8.h>


/*
== Quick Info ==

Looks like the API is desgined as some-kind of a state machine
that changes it's internal state based on the object's constructors/destructures

V8 Handles - Wrapper pointers to JavaScript objects that are updated by V8

*/

using namespace v8;

Handle<ObjectTemplate> NewObjectWrapperTemplate(v8::Isolate* isolate)
{
	Handle<ObjectTemplate> result = ObjectTemplate::New();
	result->SetInternalFieldCount(1);
	//result->SetNamedPropertyHandler(Getter, Setter);
	//result->SetIndexedPropertyHandler(IndexGetter, IndexSetter);
	return result;
}

int main(int argc, char const *argv[])
{
	std::cout << "=========Info=========" << std::endl;
	std::cout << "V8 Version: " << V8::GetVersion() << std::endl;
	std::cout << std::endl;
	std::cout << "=======Testing=======" << std::endl;

	std::cout << "Initalizing V8" << std::endl;
	V8::InitializeICU();
	V8::Initialize();
	std::cout << "V8 Initalized" << std::endl;
	std::cout << std::endl;
	std::cout << "=====Running Scripts=====" << std::endl;
	std::cout << std::endl;

	// Create a new Isolate and make it the current one.
	Isolate* isolate = Isolate::New();
	{
		Isolate::Scope isolate_scope(isolate);

		// Create a stack-allocated handle scope.
		HandleScope handle_scope(isolate);

		// Create a new context.
		Local<Context> context = Context::New(isolate);

		// Enter the context for compiling and running the hello world script.
		Context::Scope context_scope(context);

		// Create a string containing the JavaScript source code.
		Local<String> source = String::NewFromUtf8(isolate, "'Hello' + ', World!'");

		// Compile the source code.
		Local<Script> script = Script::Compile(source);

		// Run the script to get the result.
		Local<Value> result = script->Run();

		// Convert the result to an UTF8 string and print it.
		String::Utf8Value utf8(result);
		printf("%s\n", *utf8);
	}

	// Dispose the isolate and tear down V8.
	isolate->Dispose();
	V8::Dispose();

	getchar(); // Pause

	return 0;
}