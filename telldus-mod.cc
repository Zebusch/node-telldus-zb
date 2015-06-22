#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif // BUILDING_NODE_EXTENSION

#include <cstdlib>
#include <string.h>
#include <list>
#include <uv.h>
#include <node.h>
#include <v8.h>

#include <telldus-core.h>


using namespace v8;
using namespace node;
using namespace std;

namespace telldus_v8 {

	struct SensorEventBaton {
		Persistent<Function> callback;
		int sensorId;
		char *model;
		char *protocol;
		char *value;
		int ts;
		int dataType;
	};

	void SensorEventCallbackWorking(uv_work_t *req) { }

	void SensorEventCallbackAfter(uv_work_t *req, int status) {
		/*Isolate* isolate = Isolate::GetCurrent();

		SensorEventBaton *baton = static_cast<SensorEventBaton *>(req->data);

		Local<Value> args[] = {
			Number::New(isolate,baton->sensorId),
			v8::String::NewFromUtf8(isolate, baton->model),
			v8::String::NewFromUtf8(isolate, baton->protocol),
			Number::New(isolate, baton->dataType),
			v8::String::NewFromUtf8(isolate, baton->value),
			Number::New(isolate, baton->ts)
		};

		baton->callback->Call(baton->callback, 6, args);

		free(baton->model);
		free(baton->protocol);
		free(baton->value);
		delete baton;
		delete req;*/

	}

	void SensorEventCallback(const char *protocol, const char *model, int sensorId, int dataType, const char *value,
		int ts, int callbackId, void *callbackVoid) {

		Isolate* isolate = Isolate::GetCurrent();

		SensorEventBaton *baton = new SensorEventBaton();
		
		/*Persistent<Function> callback;
		callback.Reset(isolate, callbackVoid);*/

		baton->callback = static_cast<Function *>(callbackVoid);
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

	void GetDevices(const v8::FunctionCallbackInfo<v8::Value>& info) {

		int intNumberOfDevices = tdGetNumberOfDevices();
		for (int i = 0; i < intNumberOfDevices; i++) {
			int id = tdGetDeviceId(i);
			char *name = tdGetName(id);
			//printf("%d\t%s\n", id, name);
			tdReleaseString(name);
		}
		info.GetReturnValue().Set(intNumberOfDevices);
	}

	void RegisterSensorEvent(const v8::FunctionCallbackInfo<v8::Value>& args){
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);

		Local<Function> cb = Local<Function>::Cast(args[0]);
		Local<Number> num = Number::New(isolate, tdRegisterSensorEvent((TDSensorEvent)&SensorEventCallback, *cb));
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
