{
    "targets": [
        {
            "target_name": "teaming",
            "sources": [ 
                "src/index.cc" ,
                "src/node-socket.cc",
                "src/socket.cc",
                "src/packet.cc",
                "src/debugger.cc"
            ],
            'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
            'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
        }
    ]
}