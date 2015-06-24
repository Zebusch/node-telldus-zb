/// <reference path="typings/tsd.d.ts" />

var express = require('express');
var fs = require('fs');
var http = require('http');
var bodyParser = require('body-parser');
var app = express();

var port = process.env.PORT || 3000;


var telldus = require('./build/debug/telldus-mod');
//console.log(telldus.GetDevices());

//var test = telldus.fooMethod(function (args, test, test2) {
//    console.log("This is callback in JS: " + args+" - slept for "+test+" miliseconds.");
//    //console.log("This is callback in JS: " + test);
//    //console.log("This is callback in JS: " + test2);
//});

var registeredEventId = telldus.AddSensorEventListener(function (args) {
    console.log("This is value from callback: " + args);
});


//console.log("helloooooo: " + registeredEventId);

//var listenerx = telldus.GetDevices();

/**
 * Start App Server
 */
var server = app.listen(port, function () {
    console.log('ZedCo API listening at http://%s:%s', server.address().address, server.address().port);
});
