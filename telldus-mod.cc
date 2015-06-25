#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif // BUILDING_NODE_EXTENSION

#include <cstdlib>
#include <string.h>
#include <list>
#include <uv.h>
#include <node.h>
#include <v8.h>
#include <string>
#include <iostream>
#include <telldus-core.h>


using namespace v8;
using namespace node;
using namespace std;

namespace telldus_v8 {
	
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
		printf("Entering SensorEventCallbackWorking\n");
	}

	void SensorEventCallbackAfter(uv_work_t *req, int status) {

		printf("Entering SensorEventCallbackAfter\n");

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
		delete baton;
		delete ctx;
		delete req;

	}

	void SensorEventCallback(const char *protocol, const char *model, int sensorId, int dataType, const char *value,
		int ts, int callbackId, void *callbackVoid) {

		

		Isolate* isolate = Isolate::GetCurrent();
		printf("Before cast in SensorEventCallback\n");
		EventContext *ctx = static_cast<EventContext *>(callbackVoid);

		printf("SensorEventCallback.*callbackVoid: %s\n", ctx);
		printf("RegisterSensorEvent.*callbackVoid.callback: %s\n", ctx->callback);

		printf("Before func in SensorEventCallback\n");
		v8::Local<v8::Function> dafunk = v8::Local<v8::Function>::New(isolate, ((v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>)ctx->callback));
		printf("Before args in SensorEventCallback\n");
		const unsigned argc = 3;
		Local<Value> argv[argc] = { String::NewFromUtf8(isolate, "value 1 from the callback"), String::NewFromUtf8(isolate, "value 2 from the callback"), String::NewFromUtf8(isolate, "value 3 from the callback") };
		printf("Before call in SensorEventCallback\n");
		dafunk->Call(isolate->GetCurrentContext()->Global(), argc, argv);


		//EventContext *ctx = static_cast<EventContext *>(eventContext);
		SensorEventBaton *baton = new SensorEventBaton();

		//baton->eventContext = ctx;
		baton->sensorId = sensorId;
		baton->protocol = strdup(protocol);
		baton->model = strdup(model);
		baton->ts = ts;
		baton->dataType = dataType;
		baton->value = strdup(value);

		uv_work_t* req = new uv_work_t;
		req->data = baton;

		printf("Entering SensorEventCallback\n");
		uv_queue_work(uv_default_loop(), req, (uv_work_cb)SensorEventCallbackWorking, (uv_after_work_cb)SensorEventCallbackAfter);
	}

	void RegisterSensorEvent(const v8::FunctionCallbackInfo<v8::Value>& args){
		Isolate* isolate = Isolate::GetCurrent();

		v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[0]);
		v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> value(isolate, cb);

		EventContext *ctx = new EventContext();
		ctx->callback = value;
		printf("RegisterSensorEvent.*callbackVoid: %s\n", ctx);
		printf("RegisterSensorEvent.*callbackVoid.callback: %s\n", ctx->callback);

		//Local<Number> num = Number::New(isolate, tdRegisterSensorEvent((TDSensorEvent)&SensorEventCallback, ctx));
		Local<Number> num = Number::New(isolate, TestCallBackFunction(ctx));
		args.GetReturnValue().Set(num);
	}

	extern "C"
		void init(Handle<Object> target) {

		Isolate* isolate = Isolate::GetCurrent(); // returns NULL
		if (!isolate) {
			isolate = Isolate::New();
			isolate->Enter();
		}

		target->Set(v8::String::NewFromUtf8(isolate, "GetDevices"), FunctionTemplate::New(isolate, telldus_v8::GetDevices)->GetFunction());
		target->Set(v8::String::NewFromUtf8(isolate, "AddSensorEventListener"), FunctionTemplate::New(isolate, telldus_v8::RegisterSensorEvent)->GetFunction());
	}
	NODE_MODULE(telldus, init)
}
