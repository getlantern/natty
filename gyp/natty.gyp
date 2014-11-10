{
    "includes": [
        "common.gypi"
    ],
    "targets": [
        {
            "target_name": "mylib",
            "product_name": "mylib",
            "type": "static_library",
            "sources": [
                "src/*",
            ],
            "include_dirs": [
                "src/*"
            ],
            'direct_dependent_settings': {
              'include_dirs': [ 'src/' ],
            }
        },
        {
            "target_name": "natty",
            "type": "executable",
            "sources": [
                "./src/main.cc",
                "./src/talk/app/webrtc/webrtcsdp.cc",
                "./src/talk/app/webrtc/peerconnection.cc",
                "./src/talk/app/webrtc/peerconnection.h",
                "./src/talk/app/webrtc/mediastreamhandler.cc",
                "./src/talk/app/webrtc/mediastreamhandler.h",
                "./src/talk/app/webrtc/peerconnectionfactory.cc",
                "./src/talk/app/webrtc/proxy.h",
                "./src/talk/app/webrtc/datachannel.h",
                "./src/webrtc/base/stringencode.cc",
                "./src/webrtc/base/messagequeue.cc",
                "./src/talk/media/base/videoadapter.cc",
                "./src/talk/media/base/videoadapter.h",
                "./src/talk/app/webrtc/statscollector.cc",
                "./src/talk/app/webrtc/statscollector.h",
                "./src/talk/app/webrtc/mediaconstraintsinterface.cc",
                "./src/talk/app/webrtc/mediaconstraintsinterface.h",
                "./src/webrtc/base/checks.cc",
                "./src/webrtc/base/event.cc",
                "./src/webrtc/base/maccocoathreadhelper.mm",
                "./src/webrtc/base/maccocoathreadhelper.h", 
                "./src/webrtc/base/socketaddress.cc",
                "./src/webrtc/base/socketaddress.h",
                "./src/webrtc/base/helpers.cc",
                "./src/webrtc/base/helpers.h",
                "./src/webrtc/base/checks.h",
                "./src/webrtc/base/timeutils.cc",
                "./src/webrtc/base/timeutils.h",
                "./src/talk/app/webrtc/localaudiosource.cc",
                "./src/talk/app/webrtc/peerconnectionfactory.h",
                "./src/talk/app/webrtc/webrtcsdp.h",
                "./src/webrtc/base/flags.cc",
                "./src/webrtc/base/flags.h",
                "./src/webrtc/base/logging.h",
                "./src/webrtc/base/logging.cc",
                "./src/webrtc/base/thread.cc",
                "./src/webrtc/base/thread.h",
                "./src/webrtc/base/logging.cc",
                "./src/webrtc/base/ssladapter.cc",
                "./src/third_party/jsoncpp/source/src/lib_json/json_writer.cpp",
                "./src/third_party/jsoncpp/source/src/lib_json/json_value.cpp",
                "./src/natty.cc"
            ],
            "include_dirs": [
                "src/natty.h",
                "src"
            ],
            "dependencies": [
                "mylib",
            ]
        }
    ]
}
