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

	void SensorEventCallback(const char *protocol, const char *model, int sensorId, int dataType, const char *value, int ts, int callbackId, void *callbackVoid) {
		Isolate* isolate = Isolate::GetCurrent();
		EventContext *ctx = static_cast<EventContext *>(callbackVoid);
		v8::Local<v8::Function> function = v8::Local<v8::Function>::New(isolate, ((v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>)ctx->callback));
		const unsigned argc = 3;
		Local<Value> argv[argc] = { v8::Number::New(isolate, sensorId) };
		function->Call(isolate->GetCurrentContext()->Global(), 6, argv);
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
