"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var lib_1 = require("../lib");
var cluster_1 = __importDefault(require("cluster"));
if (cluster_1.default.isMaster) {
    console.log('master init');
    var emitter_1 = new lib_1.SocketEmitter();
    console.log('master init1');
    console.log(process.pid, emitter_1.psfd, emitter_1.semId);
    for (var i_1 = 0; i_1 < 1; i_1++) {
        var cp = cluster_1.default.fork({ csfd: emitter_1.semId });
    }
    emitter_1.on('data', function (data) {
        console.log('master received', data);
    }).watch();
    var i_2 = 0;
    setInterval(function () {
        console.log('Master sends', Buffer.from('testing'), new Date(), i_2++);
        try {
            if (!emitter_1.write(Buffer.from('testing'))) {
                throw 'write full';
            }
            ;
        }
        catch (e) {
            console.log('master', e, i_2);
        }
    }, 1000);
}
else {
    var csfd = parseInt(process.env['csfd'] || '0');
    console.log(process.pid, csfd);
    var emitter = new lib_1.SocketEmitter(csfd);
    emitter.on('data', function (data) {
        console.log("child " + process.pid + " received", data, new Date());
        //emitter.write(Buffer.from('testing'));
    }).watch(1000);
}
