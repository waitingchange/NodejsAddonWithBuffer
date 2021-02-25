
#include <nan.h>
#include "MyBuffer.h"

#include <iostream>

void InitAll(v8::Local<v8::Object> exports) {
  MyBuffer::Init(exports);
}

NODE_MODULE(addon, InitAll)