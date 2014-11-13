{
  'includes': [
    'talk/build/common.gypi',
  ],
  'targets': [
   {
   'target_name': 'natty',
        'type': 'executable',
        'sources': [
            '../../src/natty/defaults.h',
            '../../src/natty/flagdefs.h',
            '../../src/natty/main.cc',
            '../../src/natty/natty.cc',
            '../../src/natty/natty.h',
        ],
        'dependencies': [
            'talk/libjingle.gyp:libjingle',
            'talk/libjingle.gyp:libjingle_peerconnection',
        ]
    },    
  ]
}
 
