#include <cstdlib>
#include <string.h>
#include <list>
#include <uv.h>
#include <node.h>
#include <v8.h>
#include <string>
#include <iostream>
#include <windows.h>

using namespace v8;
using namespace node;
using namespace std;

namespace zebdus_v8 {

	void __sleep(uv_work_t* req) {
		Sleep(1500);
	}

	void after(uv_work_t *req, int status) {
		printf("After\n");

		Isolate* isolate = Isolate::GetCurrent();
		Function *localFunc = (Function *)(req->data);
		const unsigned argc = 3;
		Local<Value> argv[argc] = { String::NewFromUtf8(isolate, "value 1 from the callback"), String::NewFromUtf8(isolate, "value 2 from the callback"), String::NewFromUtf8(isolate, "value 3 from the callback") };
		&localFunc->Call(isolate->GetCurrentContext()->Global(), argc, argv);	
		printf("Leaving\n");
	}

	void fooMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		Persistent<Function> context;
		context.Reset(isolate, args[0].As<Function>());
		Local<Function> localFunc = Local<Function>::New(isolate, context);

		uv_work_t* req = new uv_work_t;
		req->data = *localFunc;
		uv_queue_work(uv_default_loop(), req, __sleep, after);
		//return scope.Close(Undefined());
	}
	extern "C"
		void initAll(Handle<Object> target) {

		Isolate* isolate = Isolate::GetCurrent(); // returns NULL
		if (!isolate) {
			isolate = Isolate::New();
			isolate->Enter();
		}

		target->Set(v8::String::NewFromUtf8(isolate, "fooMethod"), FunctionTemplate::New(isolate, zebdus_v8::fooMethod)->GetFunction());
	}
	NODE_MODULE(zebdus, initAll)
}

