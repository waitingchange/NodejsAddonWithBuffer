{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/addon.cc", "src/MyBuffer.cpp" ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")",
          './src',
      ]
    }
  ]
}