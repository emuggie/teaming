"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var events_1 = __importDefault(require("events"));
var team_socket_1 = require("./team-socket");
var SocketEmitter = /** @class */ (function (_super) {
    __extends(SocketEmitter, _super);
    function SocketEmitter(semId) {
        var _this = _super.call(this) || this;
        _this.watching = false;
        _this.interval = 0;
        if (semId) {
            _this.socket = new team_socket_1.TeamSocket(semId);
        }
        else {
            _this.socket = new team_socket_1.TeamSocket();
        }
        return _this;
    }
    SocketEmitter.prototype.watch = function (interval) {
        if (interval && interval >= 0) {
            this.interval = interval;
        }
        if (!this.watching) {
            this.watching = true;
            this.readOnEventLoop(this);
        }
    };
    SocketEmitter.prototype.unwatch = function () {
        this.watching = false;
        if (this.nextExec) {
            clearTimeout(this.nextExec);
            this.nextExec = undefined;
        }
    };
    SocketEmitter.prototype.write = function (data) {
        return this.socket.write(data);
    };
    Object.defineProperty(SocketEmitter.prototype, "psfd", {
        get: function () {
            return this.socket.getSfd();
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(SocketEmitter.prototype, "csfd", {
        get: function () {
            return this.socket.getCsfd();
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(SocketEmitter.prototype, "semId", {
        get: function () {
            return this.socket.getSemId();
        },
        enumerable: true,
        configurable: true
    });
    SocketEmitter.prototype.readOnEventLoop = function (context) {
        try {
            var val = context.socket.read();
            // console.log('read',process.pid,val);
            if (val) {
                context.emit("data", val);
            }
        }
        catch (e) {
            console.warn('SocketEmitter', e);
        }
        if (context.watching) {
            context.nextExec = setTimeout(context.readOnEventLoop, context.interval, context);
        }
    };
    SocketEmitter.prototype.on = function (event, listener) {
        return _super.prototype.on.call(this, event, listener);
    };
    return SocketEmitter;
}(events_1.default.EventEmitter));
exports.SocketEmitter = SocketEmitter;
