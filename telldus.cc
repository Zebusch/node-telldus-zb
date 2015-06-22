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

  struct DeviceEventBaton {
    Persistent<Function> callback;
    int deviceId;
    int lastSentCommand;
    int levelNum;
  };

  struct SensorEventBaton {
    Persistent<Function> callback;
    int sensorId;
    char *model;
    char *protocol;
    char *value;
    int ts;
    int dataType;
  };

  struct RawDeviceEventBaton {
    Persistent<Function> callback;
    int controllerId;
    char *data;
  };

  const int SUPPORTED_METHODS =
    TELLSTICK_TURNON
    | TELLSTICK_TURNOFF
    | TELLSTICK_BELL
    | TELLSTICK_TOGGLE
    | TELLSTICK_DIM
    | TELLSTICK_LEARN
    | TELLSTICK_EXECUTE
    | TELLSTICK_UP
    | TELLSTICK_DOWN
    | TELLSTICK_STOP;

  struct telldusDeviceInternals {
    int supportedMethods;
    int deviceType;
    int lastSentCommand;
    int level;
    int id;
    char *name;
    char *model;
    char *protocol;
  };

  Local<Object> GetSupportedMethods(int id, int supportedMethods){

	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	if (!isolate) {
		isolate = Isolate::New();
		isolate->Enter();
	}
	
    Local<Array> methodsObj = Array::New(0);

    int i = 0;

    if (supportedMethods & TELLSTICK_TURNON)  methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "TURNON"));
    if (supportedMethods & TELLSTICK_TURNOFF) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "TURNOFF"));
    if (supportedMethods & TELLSTICK_BELL) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "BELL"));
    if (supportedMethods & TELLSTICK_TOGGLE) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "TOGGLE"));
    if (supportedMethods & TELLSTICK_DIM)  methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "DIM"));
    if (supportedMethods & TELLSTICK_UP) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "UP"));
    if (supportedMethods & TELLSTICK_DOWN) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "DOWN"));
    if (supportedMethods & TELLSTICK_STOP) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "STOP"));
    if (supportedMethods & TELLSTICK_LEARN) methodsObj->Set(i++, v8::String::NewFromUtf8(isolate, "LEARN"));

    return methodsObj;

  }

  Local<String> GetDeviceType(int id, int type){
	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	if (!isolate) {
		isolate = Isolate::New();
		isolate->Enter();
	}
	
    if(type & TELLSTICK_TYPE_DEVICE) return v8::String::NewFromUtf8(isolate, "DEVICE");
    if(type & TELLSTICK_TYPE_GROUP) return v8::String::NewFromUtf8(isolate, "GROUP");
    if(type & TELLSTICK_TYPE_SCENE) return v8::String::NewFromUtf8(isolate, "SCENE");

    return v8::String::NewFromUtf8(isolate, "UNKNOWN");

  }

  Local<Object> GetDeviceStatus(int id, int lastSentCommand, int level){
	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	if (!isolate) {
		isolate = Isolate::New();
		isolate->Enter();
	}
    Local<Object> status = v8::Object::New(isolate);

    switch(lastSentCommand) {
      case TELLSTICK_TURNON:
        status->Set(v8::String::NewFromUtf8(isolate, "name", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, "ON"));
        break;
      case TELLSTICK_TURNOFF:
        status->Set(v8::String::NewFromUtf8(isolate, "name", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, "OFF"));
        break;
      case TELLSTICK_DIM:
        status->Set(v8::String::NewFromUtf8(isolate, "name", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, "DIM"));
        status->Set(v8::String::NewFromUtf8(isolate, "level", v8::String::kInternalizedString), v8::Number::New(isolate, level));
        break;
      default:
        status->Set(v8::String::NewFromUtf8(isolate, "name", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, "UNNKOWN"));
    }

    return status;

  }

  Local<Object> GetDevice( telldusDeviceInternals deviceInternals ) {
	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	if (!isolate) {
		isolate = Isolate::New();
		isolate->Enter();
	}
    Local<Object> obj = v8::Object::New(isolate);
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "name", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, deviceInternals.name));
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "id", v8::String::kInternalizedString), v8::Number::New(isolate, deviceInternals.id));
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "methods", v8::String::kInternalizedString), GetSupportedMethods(deviceInternals.id,deviceInternals.supportedMethods));
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "model", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, deviceInternals.model));
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "protocol", v8::String::kInternalizedString), v8::String::NewFromUtf8(isolate, deviceInternals.protocol));
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "type", v8::String::kInternalizedString), GetDeviceType(deviceInternals.id,deviceInternals.deviceType));
    obj->ForceSet(v8::String::NewFromUtf8(isolate, "status", v8::String::kInternalizedString), GetDeviceStatus(deviceInternals.id,deviceInternals.lastSentCommand,deviceInternals.level));

    // Cleanup
    tdReleaseString(deviceInternals.name);
    tdReleaseString(deviceInternals.model);
    tdReleaseString(deviceInternals.protocol);

    return obj;

  }

  void getDevicesFromInternals(list<telldusDeviceInternals> t , const v8::FunctionCallbackInfo<v8::Value>& info) {

	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	if (!isolate) {
		isolate = Isolate::New();
		isolate->Enter();
	}
    // Destination array
    Local<Array> devices = Array::New(isolate,t.size());
    int i=0;
    for (list<telldusDeviceInternals>::const_iterator iterator = t.begin(), end = t.end(); iterator != end; ++iterator) {
        devices->Set(i, GetDevice(*iterator));
        i++;
    }
	info.GetReturnValue().Set(devices);
  }

  telldusDeviceInternals getDeviceRaw(int idx) {

    telldusDeviceInternals deviceInternals;

    deviceInternals.id = tdGetDeviceId( idx );
    deviceInternals.name = tdGetName( deviceInternals.id );
    deviceInternals.model = tdGetModel( deviceInternals.id );
    deviceInternals.protocol = tdGetProtocol( deviceInternals.id );

    deviceInternals.supportedMethods = tdMethods( deviceInternals.id, SUPPORTED_METHODS );
    deviceInternals.deviceType = tdGetDeviceType( deviceInternals.id );
    deviceInternals.lastSentCommand = tdLastSentCommand( deviceInternals.id, SUPPORTED_METHODS );

    if( deviceInternals.lastSentCommand == TELLSTICK_DIM ) {

        char * levelStr = tdLastSentValue(deviceInternals.id);

        // Convert to number and add to object
        deviceInternals.level = atoi(levelStr);

        // Clean up the mess
        tdReleaseString(levelStr);

    }

    return deviceInternals;

  }

//  list<telldusDeviceInternals> getDevicesRaw() {
//    
//    int intNumberOfDevices = tdGetNumberOfDevices();
//    list<telldusDeviceInternals> deviceList;
//
//    for ( int i = 0 ; i < intNumberOfDevices ; i++ ) {
//      deviceList.push_back(getDeviceRaw(i));
//    }
//
//    return deviceList;
//  }
//
//  void DeviceEventCallbackWorking(uv_work_t *req) { 
//
//    DeviceEventBaton *baton = static_cast<DeviceEventBaton *>(req->data);
//
//    // Get Status
//    baton->lastSentCommand = tdLastSentCommand(baton->deviceId, SUPPORTED_METHODS);
//    baton->levelNum = 0;
//    char *level = 0;
//
//    if(baton->lastSentCommand == TELLSTICK_DIM) {
//
//      // Get level, returned from telldus-core as char
//      level = tdLastSentValue(baton->deviceId);
//
//      // Convert to number and add to object
//      baton->levelNum = atoi(level);
//
//      // Clean up the mess
//      tdReleaseString(level);
//
//    }
//  }
//
//  void DeviceEventCallbackAfter(uv_work_t *req, int status,const v8::FunctionCallbackInfo<v8::Value>& info) {
//	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
//	if (!isolate) {
//		isolate = Isolate::New();
//		isolate->Enter();
//	}
//    DeviceEventBaton *baton = static_cast<DeviceEventBaton *>(req->data);
//
//    Local<Value> args[] = {
//      v8::Number::New(isolate, baton->deviceId),
//      GetDeviceStatus(baton->deviceId,baton->lastSentCommand,baton->levelNum),
//    };
//
//    //baton->callback->Call(baton->callback, 2, args);
//
//	info.GetReturnValue().Set(Undefined(isolate));
//	
//    delete baton;
//    delete req;
//
//  }
//
//  void DeviceEventCallback( int deviceId, int method, const char * data, int callbackId, void* callbackVoid ) {
//
//    DeviceEventBaton *baton = new DeviceEventBaton();
//
//    baton->callback = static_cast<Function *>(callbackVoid);
//    baton->deviceId = deviceId;
//
//    uv_work_t* req = new uv_work_t;
//    req->data = baton;
//
//    uv_queue_work(uv_default_loop(), req, (uv_work_cb)DeviceEventCallbackWorking, (uv_after_work_cb)DeviceEventCallbackAfter);
//
//  }
//
//  void addDeviceEventListener( const v8::FunctionCallbackInfo<v8::Value>& info) {
//	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
//	if (!isolate) {
//		isolate = Isolate::New();
//		isolate->Enter();
//	}
//
//    if (!info[0]->IsFunction()) {
//    return ThrowException(Exception::TypeError(v8::String::NewFromUtf8(isolate, "Expected 1 argument: (function callback)")));
//    }
//
//    Persistent<Function> callback = Persistent<Function>::New(Handle<Function>::Cast(info[0]));
//    Local<Number> num = v8::Number::New(isolate, tdRegisterDeviceEvent((TDDeviceEvent)&DeviceEventCallback, *callback));
//	info.GetReturnValue().Set(num);
//  }
//
//  void SensorEventCallbackWorking(uv_work_t *req) { }
//
//  void SensorEventCallbackAfter(uv_work_t *req, int status) {
//	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
//	if (!isolate) {
//		isolate = Isolate::New();
//		isolate->Enter();
//	}
//
//    SensorEventBaton *baton = static_cast<SensorEventBaton *>(req->data);
//
//    Local<Value> args[] = {
//      v8::Number::New(isolate, baton->sensorId),
//      v8::String::NewFromUtf8(isolate, baton->model),
//      v8::String::NewFromUtf8(isolate, baton->protocol),
//      v8::Number::New(isolate, baton->dataType),
//      v8::String::NewFromUtf8(isolate, baton->value),
//      v8::Number::New(isolate, baton->ts)
//    };
//
//    baton->callback->Call(baton->callback, 6, args);
//	info.GetReturnValue().Set(Undefined(isolate));
//    free(baton->model);
//    free(baton->protocol);
//    free(baton->value);
//    delete baton;
//    delete req;
//
//  }
//
//  void SensorEventCallback( const char *protocol, const char *model, int sensorId, int dataType, const char *value,
//        int ts, int callbackId, void *callbackVoid ) {
//
//    SensorEventBaton *baton = new SensorEventBaton();
//
//    baton->callback = static_cast<Function *>(callbackVoid);
//    baton->sensorId = sensorId;
//    baton->protocol = strdup(protocol);
//    baton->model = strdup(model);
//    baton->ts = ts;
//    baton->dataType = dataType;
//    baton->value = strdup(value);
//
//    uv_work_t* req = new uv_work_t;
//    req->data = baton;
//
//    uv_queue_work(uv_default_loop(), req, (uv_work_cb)SensorEventCallbackWorking, (uv_after_work_cb)SensorEventCallbackAfter);
//
//  }
//
//void addSensorEventListener( const v8::FunctionCallbackInfo<v8::Value>& info) {
//	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
//	if (!isolate) {
//		isolate = Isolate::New();
//		isolate->Enter();
//	}
//
//    if (!info[0]->IsFunction()) {
//    return ThrowException(Exception::TypeError(v8::String::NewFromUtf8(isolate, "Expected 1 argument: (function callback)")));
//    }
//
//    Persistent<Function> callback = Persistent<Function>::New(Handle<Function>::Cast(info[0]));
//    Local<Number> num = v8::Number::New(isolate, tdRegisterSensorEvent((TDSensorEvent)&SensorEventCallback, *callback));
//
//	info.GetReturnValue().Set(num);
//
//  }
//
//  void RawDataEventCallbackWorking(uv_work_t *req) { }
//
//  void RawDataEventCallbackAfter(uv_work_t *req, int status) {
//	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
//	if (!isolate) {
//		isolate = Isolate::New();
//		isolate->Enter();
//	}
//    RawDeviceEventBaton *baton = static_cast<RawDeviceEventBaton *>(req->data);
//
//    Local<Value> args[] = {
//      v8::Number::New(isolate, baton->controllerId),
//      v8::String::NewFromUtf8(isolate, baton->data),
//    };
//
//    baton->callback->Call(baton->callback, 2, args);
//	info.GetReturnValue().Set(Undefined(isolate));
//
//    free(baton->data);
//    delete baton;
//    delete req;
//
//  }
//
//  void RawDataCallback(const char* data, int controllerId, int callbackId, void *callbackVoid) {
//
//    RawDeviceEventBaton *baton = new RawDeviceEventBaton();
//
//    baton->callback = static_cast<Function *>(callbackVoid);
//    baton->data = strdup(data);
//    baton->controllerId = controllerId;
//
//    uv_work_t* req = new uv_work_t;
//    req->data = baton;
//
//    uv_queue_work(uv_default_loop(), req, (uv_work_cb)RawDataEventCallbackWorking, (uv_after_work_cb)RawDataEventCallbackAfter);
//
//  }
//
//
//  void addRawDeviceEventListener( const v8::FunctionCallbackInfo<v8::Value>& info) {
//	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
//	if (!isolate) {
//		isolate = Isolate::New();
//		isolate->Enter();
//	}
//    if (!info[0]->IsFunction()) {
//      return ThrowException(Exception::TypeError(v8::String::NewFromUtf8(isolate, "Expected 1 argument: (function callback)")));
//    }
//
//    Persistent<Function> callback = Persistent<Function>::New(Handle<Function>::Cast(info[0]));
//    Local<Number> num = v8::Number::New(isolate, tdRegisterRawDeviceEvent((TDRawDeviceEvent)&RawDataCallback, *callback));
//	info.GetReturnValue().Set(num);
//
//  }
//

  struct js_work {

    uv_work_t req;
    Persistent<Function> callback;
    //char* data;
    bool rb; // Return value, boolean
    int rn; // Return value, number
    char* rs; // Return value, string

    int f; // Worktype
    int devID; // Device ID
    int v; // Arbitrary number value
    char* s; // Arbitrary string value
    char* s2; // Arbitrary string value
    bool string_used;

    list<telldusDeviceInternals> l;

  };

 // void RunWork(uv_work_t* req) {
 //   js_work* work = static_cast<js_work*>(req->data);
 //   switch(work->f) {
 //     case 0:
 //       work->rn = tdTurnOn(work->devID);
 //       break;
 //     case 1:
 //       work->rn = tdTurnOff(work->devID);
 //       break;
 //     case 2:
 //       work->rn = tdDim(work->devID,(unsigned char)work->v);
 //       break;
 //     case 3:
 //       work->rn = tdLearn(work->devID);
 //       break;
 //     case 4:
 //       work->rn = tdAddDevice();
 //       break;
 //     case 5: // SetName
 //       work->rb = tdSetName(work->devID,work->s);
 //       break;
 //     case 6: // GetName
 //       work->rs = tdGetName(work->devID);
 //       work->string_used = true;
 //       break;
 //     case 7: // SetProtocol
 //       work->rb = tdSetProtocol(work->devID,work->s);
 //       break;
 //     case 8: // GetProtocol
 //       work->rs = tdGetProtocol(work->devID);
 //       work->string_used = true;
 //       break;
 //     case 9: // SetModel
 //       work->rb = tdSetModel(work->devID,work->s);
 //       break;
 //     case 10: // GetModel
 //       work->rs = tdGetModel(work->devID);
 //       work->string_used = true;
 //       break;
 //     case 11: // GetDeviceType
 //       work->rn = tdGetDeviceType(work->devID);
 //       break;
 //     case 12:
 //       work->rb = tdRemoveDevice(work->devID);
 //       break;
 //     case 13:
 //       work->rn = tdUnregisterCallback(work->devID);
 //       break;
 //     case 14: // GetModel
 //       work->rs = tdGetErrorString(work->devID);
 //       work->string_used = true;
 //       break;
 //     case 15: // tdInit();
 //       tdInit();
 //       work->rb = true; // tdInit() has no return value, so we augment true for a return value
 //       break;
 //     case 16: // tdClose();
 //       tdClose();
 //       work->rb = true; // tdClose() has no return value, so we augment true for a return value
 //       break;
 //     case 17: // tdGetNumberOfDevices();
 //       work->rn = tdGetNumberOfDevices();
 //       break;
 //     case 18: // tdStop
 //       work->rn = tdStop(work->devID);
 //       break;
 //     case 19: // tdBell
 //       work->rn = tdBell(work->devID);
 //       break;     
 //     case 20: // tdGetDeviceId(deviceIndex)
 //       work->rn = tdGetDeviceId(work->devID);
 //       break;
 //     case 21: // tdGetDeviceParameter(deviceId, name, val)
 //       work->rs = tdGetDeviceParameter(work->devID, work->s, work->s2);
 //       work->string_used = true;
 //       break;
 //     case 22: // tdSetDeviceParameter(deviceId, name, val)
 //       work->rb = tdSetDeviceParameter(work->devID, work->s, work->s2);
 //       break;
 //     case 23: // tdExecute
 //       work->rn = tdExecute(work->devID);
 //       break;
 //     case 24: // tdUp
 //       work->rn = tdUp(work->devID);
 //       break;
 //     case 25: // tdDown
 //       work->rn = tdDown(work->devID);
 //       break;
 //     case 26: // getDevices
 //       work->l = getDevicesRaw();
 //       break;
 //   }

 // }

 // void RunCallback(uv_work_t* req, int status) {
	//Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	//if (!isolate) {
	//	isolate = Isolate::New();
	//	isolate->Enter();
	//}
 //   js_work* work = static_cast<js_work*>(req->data);
 //   work->string_used = false;

 //   Handle<Value> argv[3];


 //   // This makes it possible to catch
 //   // the exception from JavaScript land using the
 //   // process.on('uncaughtException') event.
 //   TryCatch try_catch;

 //   // Reenter the js-world
 //   switch(work->f) {

 //     // Return Number
 //     case 0:
 //     case 1:
 //     case 2:
 //     case 3:
 //     case 4:
 //     case 11:
 //     case 13:
 //     case 17:
 //     case 18:
 //     case 19:
 //     case 20:
 //     case 23:
 //     case 24:
 //     case 25:
 //       argv[0] = Integer::New(work->rn); // Return number value
 //       argv[1] = Integer::New(work->f); // Return worktype

 //       work->callback->Call(Context::GetCurrent()->Global(), 2, argv);

 //       break;

 //     // Return boolean
 //     case 5:
 //     case 7:
 //     case 9:
 //     case 12:
 //     case 15:
 //     case 16:
 //     case 22:
 //       argv[0] = Boolean::New(work->rb); // Return number value
 //       argv[1] = Integer::New(work->f); // Return worktype

 //       work->callback->Call(Context::GetCurrent()->Global(), 2, argv);

 //       break;

 //     // Return String
 //     case 6:
 //     case 8:
 //     case 10:
 //     case 14:
 //     case 21:
 //       argv[0] = v8::String::NewFromUtf8(isolate, work->rs); // Return string value
 //       argv[1] = Integer::New(work->f); // Return callback function

 //       work->callback->Call(Context::GetCurrent()->Global(), 2, argv);

 //       break;

 //     // Return list<telldusDeviceInternals>
 //     case 26:
 //       argv[0] = getDevicesFromInternals(work->l); // Return Object
 //       argv[1] = Integer::New(work->f); // Return callback function

 //       work->callback->Call(Context::GetCurrent()->Global(), 2, argv);

 //       break;

 //   }

 //   // Handle any exceptions thrown inside the callback
 //   if (try_catch.HasCaught()) {
 //     node::FatalException(try_catch);
 //   }

 //   // Check if we have an allocated string from telldus
 //   if( work->string_used ) {
 //     tdReleaseString(work->rs);
 //   }

 //   // properly cleanup, or death by millions of tiny leaks
 //   work->callback.Dispose();
 //   work->callback.Clear();

 //   free(work->s); // char* Created in AsyncCaller
 //   free(work->s2); // char* Created in AsyncCaller

 //   delete work;

 // }

 // void AsyncCaller(const v8::FunctionCallbackInfo<v8::Value>& info) {
	//Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	//if (!isolate) {
	//	isolate = Isolate::New();
	//	isolate->Enter();
	//}

 //   // Make sure we don't get any funky data
 //   if(!info[0]->IsNumber() || !info[1]->IsNumber() || !info[2]->IsNumber() || !info[3]->IsString() || !info[4]->IsString()) {
 //     return ThrowException(Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
 //   }

 //   // Make a deep copy of the string argument as we don't want 
 //   // it memory managed by v8 in the worker thread
 //   String::Utf8Value str(info[3]);
 //   char * str_copy = strdup(*str); // Deleted at end of RunCallback

 //   String::Utf8Value str2(info[4]);
 //   char * str_copy2 = strdup(*str2); // Deleted at end of RunCallback

 //   js_work* work = new js_work;
 //   work->f = info[0]->NumberValue(); // Worktype
 //   work->devID = info[1]->NumberValue(); // Device ID
 //   work->v = info[2]->NumberValue(); // Arbitrary number value
 //   work->s = str_copy; // Arbitrary string value
 //   work->s2 = str_copy2; // Arbitrary string value

 //   work->req.data = work;
 //   work->callback = Persistent<Function>::New(Handle<Function>::Cast(info[5]));

 //   uv_queue_work(uv_default_loop(), &work->req, RunWork, (uv_after_work_cb)RunCallback);

 //   Local<String> retstr = v8::String::NewFromUtf8(isolate, "Running asynchronous process initializer");
	//info.GetReturnValue().Set(retstr);
 // }


 // void SyncCaller(const v8::FunctionCallbackInfo<v8::Value>& info) {
	//Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	//if (!isolate) {
	//	isolate = Isolate::New();
	//	isolate->Enter();
	//}

 //   // Make sure we don't get any funky data
 //   if(!info[0]->IsNumber() || !info[1]->IsNumber() || !info[2]->IsNumber() || !info[3]->IsString() || !info[4]->IsString()) {
 //      return ThrowException(Exception::TypeError(v8::String::NewFromUtf8(isolate, "Wrong arguments")));
 //   }

 //   // Make a deep copy of the string argument
 //   String::Utf8Value str(info[3]);
 //   char * str_copy = strdup(*str); // Deleted at end of this function

 //   String::Utf8Value str2(info[4]);
 //   char * str_copy2 = strdup(*str2); // Deleted at end of this function

 //   js_work* work = new js_work;
 //   work->f = info[0]->NumberValue(); // Worktype
 //   work->devID = info[1]->NumberValue(); // Device ID
 //   work->v = info[2]->NumberValue(); // Arbitrary number value
 //   work->s = str_copy; // Arbitrary string value
 //   work->s2 = str_copy2; // Arbitrary string value

 //   work->string_used = false; // Used to keep track of used telldus strings

 //   // Run requested operation
 //   switch(work->f) {
 //      case 0:
 //         work->rn = tdTurnOn(work->devID);
 //         break;
 //      case 1:
 //         work->rn = tdTurnOff(work->devID);
 //         break;
 //      case 2:
 //         work->rn = tdDim(work->devID,(unsigned char)work->v);
 //         break;
 //      case 3:
 //         work->rn = tdLearn(work->devID);
 //         break;
 //      case 4:
 //         work->rn = tdAddDevice();
 //         break;
 //      case 5: // SetName
 //         work->rb = tdSetName(work->devID,work->s);
 //         break;
 //      case 6: // GetName
 //         work->rs = tdGetName(work->devID);
 //         work->string_used = true;
 //         break;
 //      case 7: // SetProtocol
 //         work->rb = tdSetProtocol(work->devID,work->s);
 //         break;
 //      case 8: // GetProtocol
 //         work->rs = tdGetProtocol(work->devID);
 //         work->string_used = true;
 //         break;
 //      case 9: // SetModel
 //         work->rb = tdSetModel(work->devID,work->s);
 //         break;
 //      case 10: // GetModel
 //         work->rs = tdGetModel(work->devID);
 //         work->string_used = true;
 //         break;
 //      case 11: // GetDeviceType
 //         work->rn = tdGetDeviceType(work->devID);
 //         break;
 //      case 12:
 //         work->rb = tdRemoveDevice(work->devID);
 //         break;
 //      case 13:
 //         work->rn = tdUnregisterCallback(work->devID);
 //         break;
 //      case 14: // GetModel
 //         work->rs = tdGetErrorString(work->devID);
 //         work->string_used = true;
 //         break;
 //       case 15: // tdInit();
 //         tdInit();
 //         work->rb = true; // tdInit() has no return value, so we augment true for a return value
 //         break;
 //       case 16: // tdClose();
 //         tdClose();
 //         work->rb = true; // tdClose() has no return value, so we augment true for a return value
 //         break;
 //       case 17: // tdGetNumberOfDevices();
 //         work->rn = tdGetNumberOfDevices();
 //         break;
 //       case 18: // tdStop
 //         work->rn = tdStop(work->devID);
 //         break;
 //       case 19: // tdBell
 //         work->rn = tdBell(work->devID);
 //         break;     
 //       case 20: // tdGetDeviceId(deviceIndex)
 //         work->rn = tdGetDeviceId(work->devID);
 //         break;
 //       case 21: // tdGetDeviceParameter(deviceId, name, val)
 //         work->rs = tdGetDeviceParameter(work->devID, work->s, work->s2);
 //         work->string_used = true;
 //         break;
 //       case 22: // tdSetDeviceParameter(deviceId, name, val)
 //         work->rb = tdSetDeviceParameter(work->devID, work->s, work->s2);
 //         break;
 //       case 23: // tdExecute
 //         work->rn = tdExecute(work->devID);
 //         break;
 //       case 24: // tdUp
 //         work->rn = tdUp(work->devID);
 //         break;
 //       case 25: // tdDown
 //         work->rn = tdDown(work->devID);
 //         break;
 //       case 26: // getDevices
 //         work->l = getDevicesRaw();
 //   }

 //   // Run callback
 //   Handle<Value> argv;

 //   // Run the actual callback
 //   switch(work->f) {

 //     // Return Number
 //     case 0:
 //     case 1:
 //     case 2:
 //     case 3:
 //     case 4:
 //     case 11:
 //     case 13:
 //     case 17:
 //     case 18:
 //     case 19:
 //     case 20:
 //     case 23:
 //     case 24:
 //     case 25:
 //       argv = Integer::New(isolate,work->rn); // Return number value
 //       break;

 //     // Return boolean
 //     case 5:
 //     case 7:
 //     case 9:
 //     case 12:
 //     case 15:
 //     case 16:
 //     case 22:
 //       argv = Boolean::New(work->rb); // Return number value
 //       break;

 //     // Return String
 //     case 6:
 //     case 8:
 //     case 10:
 //     case 14:
 //     case 21:
 //       argv = v8::String::NewFromUtf8(isolate, work->rs); // Return string value
 //       break;

 //     // Return list<telldusDeviceInternals>
 //     case 26:
 //       argv = getDevicesFromInternals(work->l); // Return Object
 //       break;
 //   }

 //   // Check if we have an allocated string from telldus
 //   if( work->string_used ) {
 //     tdReleaseString(work->rs);
 //   }

 //   free(str_copy); // char* Created in the beginning of this function
 //   free(str_copy2); // char* Created in beginning of this function

 //   delete work;

	//info.GetReturnValue().Set(argv);

 // }

}

extern "C"
void init(Handle<Object> target) {
	Isolate* isolate = Isolate::GetCurrent(); // returns NULL
	if (!isolate) {
		isolate = Isolate::New();
		isolate->Enter();
	}

  // Asynchronous function wrapper
  target->Set(v8::String::NewFromUtf8(isolate, "AsyncCaller"),
    FunctionTemplate::New(telldus_v8::AsyncCaller)->GetFunction());

  // Syncronous function wrapper
  target->Set(v8::String::NewFromUtf8(isolate, "SyncCaller"),
    FunctionTemplate::New(telldus_v8::SyncCaller)->GetFunction());

  // Functions to add event-listener callbacks 
  target->Set(v8::String::NewFromUtf8(isolate, "addDeviceEventListener"),
    FunctionTemplate::New(telldus_v8::addDeviceEventListener)->GetFunction());
  target->Set(v8::String::NewFromUtf8(isolate, "addSensorEventListener"),
    FunctionTemplate::New(telldus_v8::addSensorEventListener)->GetFunction());
  target->Set(v8::String::NewFromUtf8(isolate, "addRawDeviceEventListener"),
    FunctionTemplate::New(telldus_v8::addRawDeviceEventListener)->GetFunction());

}
NODE_MODULE(telldus, init)
