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
#include <sstream>
#include <ctime>

using namespace v8;
using namespace node;
using namespace std;

namespace zebdus_v8 {

	struct EventContext {
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> callback;
	};

	struct Baton {
		uv_work_t request;
		EventContext *eventContext;
		int sleepTime;
		int id;
	};	

	void __sleep(uv_work_t* req) {
		Baton *baton = static_cast<Baton *>(req->data);
		printf("Thread with id %d Sleeping for %d ms...\n", baton->id, baton->sleepTime);
		Sleep(baton->sleepTime);
	}

	void after(uv_work_t *req, int status) {
		//printf("After\n");
		Isolate* isolate = Isolate::GetCurrent();

		Baton *baton = static_cast<Baton *>(req->data);
		EventContext *ctx = static_cast<EventContext *>(baton->eventContext);

		v8::Local<v8::Function> dafunk = v8::Local<v8::Function>::New(isolate, ((v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>)ctx->callback));
		
		const unsigned argc = 3;
		Local<Value> argv[argc] = { v8::Integer::New(isolate, baton->id), v8::Integer::New(isolate, baton->sleepTime), String::NewFromUtf8(isolate, "value 3 from the callback") };
		dafunk->Call(isolate->GetCurrentContext()->Global(), argc, argv);
		delete baton;
		delete req;

		//printf("Leaving\n");
	}

	void fooMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Isolate* isolate = Isolate::GetCurrent();
		srand(time(NULL));
		v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[0]);
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> value(isolate, cb);
		
		EventContext *ctx = new EventContext();
		ctx->callback = value;

		int min = 1500;
		int max = 10000;

		for (int i = 0; i < 4; i++){	

			Baton *baton = new Baton();
			baton->request.data = baton;
			baton->eventContext = ctx;
			baton->sleepTime = (min + (rand() % (int)(max - min + 1)));
			baton->id = i;

			uv_work_t* req = new uv_work_t;
			req->data = baton;

			uv_queue_work(uv_default_loop(), req, __sleep, after);
		}

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

