#include <cstdlib>
#include <string.h>
#include <list>
#include <uv.h>
#include <node.h>
#include <v8.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <telldus-core.h>

using namespace v8;
using namespace node;
using namespace std;

namespace zebdus_v8 {

	struct Baton {
		uv_work_t request;
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> callback;
	};

	struct EventContext {		
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> callback;
	};

	void __sleep(uv_work_t* req) {
		Sleep(1500);
	}

	void after(uv_work_t *req, int status) {
		printf("After\n");
		Isolate* isolate = Isolate::GetCurrent();	

		Baton *baton = static_cast<Baton *>(req->data);

		v8::Local<v8::Function> dafunk = v8::Local<v8::Function>::New(isolate, ((v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>)baton->callback));

		const unsigned argc = 3;
		Local<Value> argv[argc] = { String::NewFromUtf8(isolate, "value 1 from the callback"), String::NewFromUtf8(isolate, "value 2 from the callback"), String::NewFromUtf8(isolate, "value 3 from the callback") };
		dafunk->Call(isolate->GetCurrentContext()->Global(), argc, argv);


		printf("Leaving\n");
	}

	void fooMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();

		v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[0]);
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> value(isolate, cb);

		Baton *baton = new Baton();
		baton->request.data = baton;
		baton->callback = value;

		uv_work_t* req = new uv_work_t;
		req->data = baton;		
		uv_queue_work(uv_default_loop(), req, __sleep, after);		
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

