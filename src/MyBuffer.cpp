#include "MyBuffer.h"

#include <v8.h>
#include <nan.h>
#include <iostream>
#include <fstream>

using namespace v8;
using namespace std;
using namespace Nan;

Nan::Persistent<v8::Function> MyBuffer::constructor;

MyBuffer *MyBuffer::m_instance = new MyBuffer;

MyBuffer::MyBuffer() : m_counter(0)
{
}

MyBuffer::~MyBuffer()
{
}


MyBuffer *MyBuffer::getInstance()
{
    return m_instance;
}

void MyBuffer::TimerCallBack()
{
}

void MyBuffer::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("MyBuffer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "setCallback", setCallback);
    Nan::SetPrototypeMethod(tpl, "getNewBuffer", getNewBuffer);

    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context,
                 Nan::New("MyBuffer").ToLocalChecked(),
                 tpl->GetFunction(context).ToLocalChecked());
}

// 真正用于异步执行JavaScript 存放在C++ 中的回调函数。 由libuv 触发
void async_on_callback(uv_async_t *handle)
{   
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    // // 数据出队
    MyCallbackData *m_bufferData;
    while (MyBuffer::getInstance()->m_BufferData_l.try_dequeue(m_bufferData))
    {
        // 留意这个 copy 字段 ， 存在内存重新分配 使用时候要小心  v8 data 不在C++ 也不在JavaScript
        v8::Local<v8::Object> buferObj = Nan::CopyBuffer(m_bufferData->data, m_bufferData->size).ToLocalChecked();
        std::string msg = std::string(m_bufferData->data,m_bufferData->size);
        std::cout << "---" <<  msg << std::endl;
        delete [](m_bufferData->data);
        delete(m_bufferData);
        m_bufferData = nullptr;

        const unsigned argc = 1;  
        v8::Local<v8::Value> argv[argc] = {buferObj};
        try
        {
            v8::Local<v8::Function> m_call = v8::Local<v8::Function>::New(isolate, MyBuffer::getInstance()->callback);
            Nan::Call(m_call, isolate->GetCurrentContext()->Global(), argc, argv);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

// 用于保存和设置 JavaScript 回调函数
void MyBuffer::setCallback(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    Isolate *isolate = info.GetIsolate();
    Nan::HandleScope mscope;
    v8::HandleScope scope(isolate);
    Local<Value> arg = info[0];
    if (!arg->IsFunction())
    {
        return;
    }
    // 持久化存储 JavaScript callback
    v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(info[0]);
    MyBuffer::getInstance()->callback.Reset(func);
    uv_async_init(uv_default_loop(), &MyBuffer::getInstance()->m_callback_async, async_on_callback);

    // 这里为了测试先在这里启动timer  多次调用请注意合理关闭timer
    MTimer *t = new MTimer();
    t->setInterval([&]() {
        MyBuffer::getInstance()->m_counter++;
        cout << "time is : " << MyBuffer::getInstance()->m_counter << endl;
        // 构造假数据
        string dataStr = "TimerCallBack count is " + to_string(MyBuffer::getInstance()->m_counter) + " hello ";
        MyCallbackData *m_data = new MyCallbackData();
        m_data->data = new char[strlen(dataStr.c_str()) + 1];
        strncpy(m_data->data, dataStr.c_str(), strlen(dataStr.c_str()));
        m_data->data[strlen(dataStr.c_str())] = '\0';
        m_data->size = strlen(dataStr.c_str());
        // 数据入队
        MyBuffer::getInstance()->m_BufferData_l.enqueue(m_data);

        uv_async_send(&MyBuffer::getInstance()->m_callback_async); // 通知libuv  异步句柄允许用户“唤醒”事件循环并获取从另一个线程调用的回调。
    },
   20);
}

void MyBuffer::getNewBuffer(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    Isolate *isolate = info.GetIsolate();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> infoObj = v8::Local<v8::Object>::Cast(info[0]);
    unsigned char *buffer = (unsigned char *)node::Buffer::Data(infoObj);
    size_t size = node::Buffer::Length(infoObj);

    for (int i = 0; i < size; i++)
    {
        if (i > size / 2)
        {
            buffer[i] += 1;
        }
        // cout << buffer[i] << endl;
    }
    info.GetReturnValue().Set(Nan::CopyBuffer((char *)buffer, size).ToLocalChecked());
}

void MyBuffer::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    Nan::HandleScope mscope;
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        v8::Isolate::Scope scope(isolate);
        v8::HandleScope handle_scope(isolate);
        m_instance->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
}
