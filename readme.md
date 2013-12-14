telldus - Node bindings for telldus-core
===

Latest release is 0.0.1, available at npm using ```npm install telldus```

---

Installation (From source):

_Note that the master branch isn't always top notch. If it doesn't compile, try an older revision or install the stable release_

1. Install telldus-core library
	* Windows, Mac: Install Telldus Center -- go [here](http://download.telldus.se/TellStick/Software/TelldusCenter/) and get the latest version of the appropriate DMG or EXE file and install
	* Linux Ubuntu/Debian: http://developer.telldus.com/wiki/TellStickInstallationUbuntu
	* Linux source install: http://developer.telldus.com/wiki/TellStickInstallationSource
2. Clone this project and enter the node-telldus directory ```cd node-telldus```
2. Compile this module ```npm install -g```
3. Link the module to your project ```cd yourprojectdirectory``` ```npm link telldus```

---

Installation (From npm):

1. Install telldus-core library
	* Windows, Mac: Install Telldus Center -- go [here](http://download.telldus.se/TellStick/Software/TelldusCenter/) and get the latest version of the appropriate DMG or EXE file and install
	* Linux Ubuntu/Debian: http://developer.telldus.com/wiki/TellStickInstallationUbuntu
	* Linux source install: http://developer.telldus.com/wiki/TellStickInstallationSource
2. Install this module using npm ```cd yourprojectdirectory``` ```npm install telldus```

---

# Basic Usage

Make sure telldusd is running on the same machine.

```javascript
var tellduscore = require('telldus');
var devices = tellduscore.getDevicesSync();
```

---

API
===

getDevicesSync
----------

Returns an array of device dictionary objects.
Only configured devices are returned.

Currently only available as a synchronous function.

Signature:

```javascript
var devices = tellduscore.getDevicesSync();
```

```javascript
[
  {
    id: 1,
    name: 'name from telldus.conf',
    methods: [ 'TURNON', 'TURNOFF' ],
    model: 'codeswitch',
    type: 'DEVICE',
    status: {status: 'OFF'}
  },
  ...
]
```


turnOn
------

Turns a configured device ON.

Synchronous version: turnOnSync(deviceId);

Signature:

```javascript
tellduscore.turnOn(deviceId,function(returnValue) {
	console.log('deviceId is now ON');
});
```

Similar to the command

```bash
tdtool --on deviceId
```


turnOff
-------

Turns a configured device OFF.

Synchronous version: turnOffSync(deviceId);

Signature:

```javascript
tellduscore.turnOff(deviceId,function(returnValue) {
	console.log('Device' + deviceId + ' is now OFF');
});
```

Similar to the command

```bash
tdtool --off deviceId
```


dim
---

Dims a configured device to a certain level.

Synchronous version: dimSync(deviceId,level);

Signature:

```javascript
tellduscore.dim(deviceId, level,function(returnValue) {
	console.log('Device ' + deviceId + ' is now dimmed to level ' + level);
});
```


addRawDeviceEventListener
-------------------------

Add a listener for raw device events.
This is usefull for scanning for devices not yet configured

Signature:

```javascript
var listener = tellduscore.addRawDeviceEventListener(function(controllerId, data) {
	console.log('Raw device event: ' + data);
});
```

* `controllerId`: id of receiving controller, can identify the TellStick if several exists in the system.
* `data`: A semicolon separated string with colon separated key / value pairs.

```javascript
'class:command;protocol:arctech;model:selflearning;house:5804222;unit:2;group:0;method:turnon;'
```


addDeviceEventListener
----------------------

Add a listener for device events

Signature:

```javascript
var listener = tellduscore.addDeviceEventListener(function(deviceId, status) {
	console.log('Device ' + deviceId + ' is now ' + status.status);
});
```

* `status`: is an object of the form:
```
    {"status": "the status"}
```

addSensorEventListener
----------------------

Add a listener for sensor events

Signature:

```javascript
var listener = telldus.addSensorEventListener(function(deviceId,protocol,model,type,value,timestamp) {
	console.log('New sensor event received: ',deviceId,protocol,model,type,value,timestamp);
});
```


removeEventListener
-------------------

Remove a previously added listener.

Currently only available as a synchronous function.

Signature:

```javascript
tellduscore.removeEventListenerSync(listener);
```

---

License and Credits:

This project is licensed under the MIT license and is forked from telldus-core-js (https://github.com/evilmachina/telldus-core-js) by GitHub user evilmachina. 

