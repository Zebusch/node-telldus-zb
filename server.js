/// <reference path="typings/tsd.d.ts" />

var express = require('express');
var fs = require('fs');
var http = require('http');
var bodyParser = require('body-parser');
var moment = require('moment');
var app = express();

var port = process.env.PORT || 3000;


var telldus = require('./build/debug/telldus-mod');
//console.log(telldus.GetDevices());

var test = telldus.AddSensorEventListener(function (sensorId, model, protocol,dataType,value,ts) {
    console.log("sensorId: " + sensorId);
    console.log("model: " + model);
    console.log("protocol: " + protocol);
    console.log("dataType: " + dataType);
    console.log("value: " + value);
    console.log("TimeStamp: " + new moment.unix(ts).format());
    console.log("");
});

//var registeredEventId = telldus.AddSensorEventListener(function (args, test, test2) {
//    console.log("This is callback: " + args);
//    console.log("This is callback: " + test);
//    console.log("This is callback: " + test2);
//});


//console.log("helloooooo: " + registeredEventId);

//var listenerx = telldus.GetDevices();

/**
 * Start App Server
 */
var server = app.listen(port, function () {
    console.log('ZedCo API listening at http://%s:%s', server.address().address, server.address().port);
});
