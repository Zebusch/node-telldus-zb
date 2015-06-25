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

	struct SensorEventBaton {
		uv_work_t request;
		Function *callback;
		EventContext *eventContext;
		int sensorId;
		char *model;
		char *protocol;
		char *value;
		int ts;
		int dataType;
	};


	void SensorEventCallbackWorking(uv_work_t *req) {
	}

	void SensorEventCallbackAfter(uv_work_t *req, int status) {
		Isolate* isolate = Isolate::GetCurrent();

		SensorEventBaton *baton = static_cast<SensorEventBaton *>(req->data);
		EventContext *ctx = static_cast<EventContext *>(baton->eventContext);
		v8::Local<v8::Function> dafunk = v8::Local<v8::Function>::New(isolate, ((v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>)ctx->callback));

		Local<Value> args[] = {
			Number::New(isolate, baton->sensorId),
			v8::String::NewFromUtf8(isolate, baton->model),
			v8::String::NewFromUtf8(isolate, baton->protocol),
			Number::New(isolate, baton->dataType),
			v8::String::NewFromUtf8(isolate, baton->value),
			Number::New(isolate, baton->ts)
		};

		dafunk->Call(isolate->GetCurrentContext()->Global(), 6, args);

		free(baton->model);
		free(baton->protocol);
		free(baton->value);
		//delete ctx;
		delete baton;
		delete req;
	}

	void SensorEventCallback(const char *protocol, const char *model, int sensorId, int dataType, const char *value,
		int ts, int callbackId, void *callbackVoid) {

		EventContext *ctx = static_cast<EventContext *>(callbackVoid);
		SensorEventBaton *baton = new SensorEventBaton();

		baton->eventContext = ctx;
		baton->sensorId = sensorId;
		baton->protocol = strdup(protocol);
		baton->model = strdup(model);
		baton->ts = ts;
		baton->dataType = dataType;
		baton->value = strdup(value);

		uv_work_t* req = new uv_work_t;
		req->data = baton;

		uv_queue_work(uv_default_loop(), req, (uv_work_cb)SensorEventCallbackWorking, (uv_after_work_cb)SensorEventCallbackAfter);
	}

	void RegisterSensorEvent(const v8::FunctionCallbackInfo<v8::Value>& args){
		Isolate* isolate = Isolate::GetCurrent();

		v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[0]);
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> value(isolate, cb);

		EventContext *ctx = new EventContext();
		ctx->callback = value;

		Local<Number> num = Number::New(isolate, tdRegisterSensorEvent((TDSensorEvent)&SensorEventCallback, ctx));
		args.GetReturnValue().Set(num);
	}
	extern "C"
		void initAll(Handle<Object> target) {

		Isolate* isolate = Isolate::GetCurrent(); // returns NULL
		if (!isolate) {
			isolate = Isolate::New();
			isolate->Enter();
		}
		target->Set(v8::String::NewFromUtf8(isolate, "AddSensorEventListener"), FunctionTemplate::New(isolate, zebdus_v8::RegisterSensorEvent)->GetFunction());
	}
	NODE_MODULE(zebdus, initAll)
}

