#include "MyBuffer.h"

#include <v8.h>
#include <nan.h>
#include <iostream>

using namespace v8;
using namespace std;
using namespace Nan;

Nan::Persistent<v8::Function> MyBuffer::constructor;

MyBuffer *MyBuffer::m_instance = nullptr;

MyBuffer::MyBuffer() : m_counter(0), m_timer(nullptr)
{
}

MyBuffer::~MyBuffer()
{
}


MyBuffer *MyBuffer::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new MyBuffer();
    }

    return m_instance;
}

void MyBuffer::TimerCallBack()
{
    // 构造假数据
    m_counter += 1;
    string dataStr = "TimerCallBack count is " + to_string(m_counter);
    MyCallbackData *m_data = new MyCallbackData();
    m_data->data = new char[strlen(dataStr.c_str()) + 1];
    strcpy(m_data->data, dataStr.c_str());
    m_data->size = strlen(dataStr.c_str());
    // 数据入队
    m_BufferData_l.enqueue(m_data);
    uv_async_send(&m_callback_async); // 通知libuv  异步句柄允许用户“唤醒”事件循环并获取从另一个线程调用的回调。
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
void MyBuffer::async_on_callback(uv_async_t *handle)
{
    Nan::HandleScope scope;
    Isolate *isolate = Isolate::GetCurrent();
    // 数据出队
    MyCallbackData *m_bufferData;
    while (MyBuffer::getInstance()->m_BufferData_l.try_dequeue(m_bufferData))
    {
        const unsigned argc = 1;
        // 留意这个 copy 字段 ， 存在内存重新分配 使用时候要小心  v8 data 不在C++ 也不在JavaScript
        v8::Local<v8::Object> buferObj = Nan::CopyBuffer(m_bufferData->data, m_bufferData->size).ToLocalChecked();
        delete []m_bufferData->data;
        delete(m_bufferData);
        v8::Local<v8::Value> argv[argc] = {buferObj};
        v8::Local<v8::Function> m_call = Nan::New(MyBuffer::getInstance()->m_callback);
        Nan::Call(m_call, Nan::GetCurrentContext()->Global(), argc, argv);
    }
}

// 用于保存和设置 JavaScript 回调函数
void MyBuffer::setCallback(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    Local<String> strUtf8 = Local<String>::Cast(info[0]);
    Isolate *isolate = info.GetIsolate();
    v8::HandleScope scope(isolate);
    Local<Value> arg = info[0];
    if (!arg->IsFunction())
    {
        return;
    }
    // 持久化存储 JavaScript callback
    v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(info[0]);
    MyBuffer::getInstance()->m_callback.Reset(func);
    uv_async_init(uv_default_loop(), &MyBuffer::getInstance()->m_callback_async, MyBuffer::async_on_callback);
    // 这里为了测试先在这里启动timer  多次调用请注意合理关闭timer
    MyBuffer::getInstance()->m_timer->StartTimer(2000, std::bind(&MyBuffer::TimerCallBack, m_instance));
}

void MyBuffer::getNewBuffer(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    Isolate *isolate = info.GetIsolate();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> infoObj = v8::Local<v8::Function>::Cast(info[0]);
    unsigned char*buffer = (unsigned char*) node::Buffer::Data(infoObj);  
    size_t size = node::Buffer::Length(infoObj);

    for (int i = 0; i < size; i++)
    {
        if (i > size / 2)
        {
            buffer[i] += 1;
        }
        // cout << buffer[i] << endl;
    }
    info.GetReturnValue().Set(Nan::NewBuffer((char *)buffer, size).ToLocalChecked());
}

void MyBuffer::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        if (m_instance == nullptr)
        {
            m_instance = new MyBuffer();
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
        }
        m_instance->m_timer = new Timer();
        m_instance->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
}
