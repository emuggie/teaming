#include "napi.h"
#include "node-socket.h"

Napi::Object Init(Napi::Env env, Napi::Object exports){
    return NODE::Socket::Init(env, exports);
}

NODE_API_MODULE(TeamSocket, Init);