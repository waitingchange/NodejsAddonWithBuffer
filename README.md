# NodejsAddonWithBuffer
NodejsAddonWithBuffer

新增了 vscode 调试使用的配置，方便断点和定位具体问题

另外需要留意，Nan::NewBuffer  和 Nan::CopyBuffer  两个 ， API 之间的使用 ，如果使用  NewBuffer 留意一下对象销毁时候的操作
