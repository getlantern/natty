{
  'includes': [
    'talk/build/common.gypi',
  ],
  'targets': [
   {
   'target_name': 'natty',
        'type': 'executable',
        'sources': [
            'natty/defaults.h',
            'natty/flagdefs.h',
            'natty/main.cc',
            'natty/natty.cc',
            'natty/natty.h',
        ],
        'dependencies': [
            'talk/libjingle.gyp:libjingle',
            'talk/libjingle.gyp:libjingle_peerconnection',
        ]
    },    
  ]
}
 
