
#ifndef LOG_INTERFACE_H
#define LOG_INTERFACE_H
#include <node.h>
#include <nan.h>
#include <uv.h>
#include <iostream>
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

void async_on_callback(uv_async_t *handle);

class MyBuffer : public node::ObjectWrap
{
public:
    static void Init(v8::Local<v8::Object> exports);
    // libuv 用于异步回调 跨线程
    static void setCallback(const Nan::FunctionCallbackInfo<v8::Value> &info);

    uv_async_t m_callback_async;         // 保存线程任务的
    Nan::Persistent<v8::Function> callback; // 保存回调函数的
    // libuv end

    // 主动方式 传递buffer 和获取buffer
    static void getNewBuffer(const Nan::FunctionCallbackInfo<v8::Value> &info);

    // 用于产生异步回调数据
    void TimerCallBack();

    // 使用线程安全的队列，解决生产者消费者之间的问题
    moodycamel::BlockingConcurrentQueue< MyCallbackData * > m_BufferData_l;


    static MyBuffer *getInstance();

    int m_counter;
    
private:
    static MyBuffer *m_instance;
    
    explicit MyBuffer();
    ~MyBuffer();
    static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
    static Nan::Persistent<v8::Function> constructor;

    
};


#endif /* LOG_INTERFACE_H */
