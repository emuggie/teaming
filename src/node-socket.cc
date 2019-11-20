#include "node-socket.h"

#include "debugger.h"

struct DataDt {
    void operator()(Napi::Env env, char* data){
        delete[] data;
        printf("destroyed\n");
    }
} dealoc;

namespace NODE {
    Napi::FunctionReference NODE::Socket::constructor;

    Napi::Object Socket::Init(Napi::Env env, Napi::Object exports){
        Napi::HandleScope scope(env);
        Napi::Function func = DefineClass(env,
            NODE_SOCKET_CLASS_NAME,{
                InstanceMethod("getSfd", &Socket::getSfd),
                InstanceMethod("getCsfd", &Socket::getCsfd),
                InstanceMethod("getSemId", &Socket::getSemId),
                InstanceMethod("read", &Socket::read),
                InstanceMethod("write", &Socket::write)
            }
        );
        constructor = Napi::Persistent(func);
        constructor.SuppressDestruct();
        exports.Set(NODE_SOCKET_CLASS_NAME, func);
        return exports;
    }
    
    Socket::Socket(const Napi::CallbackInfo& info): Napi::ObjectWrap<Socket>(info){
        Napi::Env env = info.Env();
        Napi::HandleScope scope(env);
        int length = info.Length();
    
        if (length >= 1){
            if(!info[0].IsNumber()){
                Napi::TypeError::New(env, "Socket File descriptor expected ").ThrowAsJavaScriptException();
            }
            Napi::Number value = info[0].As<Napi::Number>();
            int csfd = value.Int32Value();
            try{
                socket = new ASSC::Socket(csfd);
            }catch(const char* message){
                Napi::Error::New(env, "Socket create Error : ERROR CODE : " + std::string(message)).ThrowAsJavaScriptException();
            }
            return;
        }
        try{
            socket = new ASSC::Socket();
        }catch(const char* message){
            Napi::Error::New(env, "Socket create Error : ERROR CODE : "+std::string(message)).ThrowAsJavaScriptException();
        }
    }

    Socket::~Socket(){
        delete socket;
    }

    Napi::Value Socket::read(const Napi::CallbackInfo& info){
        try{
            char* data = socket->read();
            if(data == nullptr){
                return info.Env().Undefined();    
            }
            Napi::Buffer<char> val = Napi::Buffer<char>::New(info.Env(), data, strlen(data), dealoc);
            return val;
        }catch(int err){
            return info.Env().Undefined();    
        }
    }

    Napi::Value Socket::write(const Napi::CallbackInfo& info){
        Napi::Value param = info[0];
        if(param.IsNull()){
            return Napi::Boolean::New(info.Env(), false);
        }

        if(param.IsString()){
            std::string str = param.As<Napi::String>().Utf8Value();
            return Napi::Boolean::New(info.Env(), socket->write(str.c_str(), strlen(str.c_str())));
        }

        if(param.IsBuffer()){
            Napi::Buffer<char> buffer = param.As<Napi::Buffer<char>>();
            return Napi::Boolean::New(info.Env(), socket->write(buffer.Data(),buffer.Length()));
        }

        return Napi::Boolean::New(info.Env(), false);
    }

} // namespace NODE