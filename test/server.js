"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var express_1 = __importDefault(require("express"));
var http_1 = __importDefault(require("http"));
var cluster_1 = __importDefault(require("cluster"));
var land_of_promise_1 = require("land-of-promise");
var lib_1 = require("../lib");
if (cluster_1.default.isMaster) {
    var emitter_1 = new lib_1.SocketEmitter();
    var worker1 = cluster_1.default.fork({ semId: emitter_1.semId });
    var worker2 = cluster_1.default.fork({ semId: emitter_1.semId });
    var app = express_1.default();
    app.set('port', process.env.PORT || 8090);
    // Pending Request map
    var pendingReqMap_1 = new Map();
    app.use('/', function (req, res, next) {
        // Unique key for each request
        var messageId = new Date().valueOf().toString(36) + ':' + Math.random().toString(36).substr(2);
        // Parameter to hand over
        var jo = {
            id: messageId,
            uri: req.url
        };
        // Timeout promise
        var promise = new land_of_promise_1.TimeoutPromise(function (resolve, reject) {
            // send to child:any 
            if (!emitter_1.write(JSON.stringify(jo))) {
                reject('buffer full');
            }
        }).then(function (result) {
            // response back
            res.send("done : " + JSON.stringify(result));
        }).catch(function (error) {
            // on error
            res.send("failed: " + error);
        });
        // register pending request
        pendingReqMap_1.set(messageId, promise);
    });
    //define bevior when received
    emitter_1.on('data', function (buffer) {
        var returned = JSON.parse(buffer.toString());
        console.log('received', returned);
        // retrieve pending request from map with id
        var promise = pendingReqMap_1.get(returned.id);
        if (promise) {
            // resovle if found
            promise.resolve(returned);
            // remove resolved.
            pendingReqMap_1.delete(returned.id);
        }
    }).watch();
    http_1.default.createServer(app).listen(app.get('port'), function () {
        console.log('server start');
    });
}
else {
    // Child task
    //Init connection with parent
    var semId = -1;
    try {
        semId = parseInt(process.env['semId'] || '');
    }
    catch (error) {
        console.error("Process " + process.pid + " failed to acquire semaphore with id " + process.env['semId']);
        process.exit(1);
    }
    var emitter_2 = new lib_1.SocketEmitter(semId);
    // Define task given from parent.
    emitter_2.on('data', function (buffer) {
        var jo = JSON.parse(buffer.toString());
        jo.response = 'received';
        //Write back to parent
        emitter_2.write(JSON.stringify(jo));
    }).watch();
}
