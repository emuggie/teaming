#ifndef NODE_SOCKET_H_
#define NODE_SOCKET_H_

#include "napi.h"
#include "socket.h"

#define NODE_SOCKET_CLASS_NAME "TeamSocket"

namespace NODE 
{
    
class Socket: public Napi::ObjectWrap<Socket>{
    public : 
        static Napi::Object Init(Napi::Env env, Napi::Object exports);

        Socket(const Napi::CallbackInfo& info);
        ~Socket();
        Napi::Value getSfd(const Napi::CallbackInfo& info){
            return Napi::Number::New(info.Env(), socket->getSfd());
        }  
        Napi::Value getCsfd(const Napi::CallbackInfo& info){
            return Napi::Number::New(info.Env(), socket->getCsfd());
        }
        Napi::Value getSemId(const Napi::CallbackInfo& info){
            return Napi::Number::New(info.Env(), socket->getSemId());
        }
        Napi::Value read(const Napi::CallbackInfo& info);
        Napi::Value write(const Napi::CallbackInfo& info);
    private :
        static Napi::FunctionReference constructor;
        ASSC::Socket* socket;
};

}

#endif