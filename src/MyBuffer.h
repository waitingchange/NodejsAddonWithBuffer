
#ifndef LOG_INTERFACE_H
#define LOG_INTERFACE_H
#include <node.h>
#include <nan.h>
#include <uv.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "Timer.h"
// 解决生产者消费者模型 线程安全的队列
#include "blockingconcurrentqueue.h"

using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::To;
using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;


struct MyCallbackData
{
    int size;
    char * data;
};


class MyBuffer : public node::ObjectWrap
{
public:
    static void Init(v8::Local<v8::Object> exports);
    // libuv 用于异步回调 跨线程
    static void setCallback(const Nan::FunctionCallbackInfo<v8::Value> &info);
    Nan::Persistent<v8::Function> m_callback;
    static void async_on_callback(uv_async_t *handle);
    uv_async_t m_callback_async;
    // libuv end
    

    // 主动方式 传递buffer 和获取buffer
    static void getNewBuffer(const Nan::FunctionCallbackInfo<v8::Value> &info);

    // 用于产生异步回调数据
    void TimerCallBack();

    // 使用线程安全的队列，解决生产者消费者之间的问题
    moodycamel::BlockingConcurrentQueue< MyCallbackData * > m_BufferData_l;


    static MyBuffer *getInstance();
private:
    static MyBuffer *m_instance;

    // 测试使用，新增timer ， 目的是在C++ 内部异步发送数据，并且回调给JavaScript 主线程
    Timer *m_timer;
    
    explicit MyBuffer();
    ~MyBuffer();
    static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
    static Nan::Persistent<v8::Function> constructor;

    int m_counter;
};


#endif /* LOG_INTERFACE_H */
