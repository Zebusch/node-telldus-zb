/// <reference path="typings/tsd.d.ts" />

var express = require('express');
var fs = require('fs');
var http = require('http');
var bodyParser = require('body-parser');
var app = express();

var port = process.env.PORT || 3000;


var telldus = require('./build/Release/telldus-mod');
//console.log(telldus.GetDevices());

var test = telldus.fooMethod(function (args, test, test2) {
    console.log("This is callback in JS: " + args);
    //console.log("This is callback in JS: " + test);
    //console.log("This is callback in JS: " + test2);
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
