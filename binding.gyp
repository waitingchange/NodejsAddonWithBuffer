{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/addon.cc", "src/MyBuffer.cpp" ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")",
          './src',
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "cflags": [ "-std=c++11","-D_FILE_OFFSET_BITS=64"],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}