(function () {

    addon = require('./build/Debug/addon');
    global.logBridge = new addon.MyBuffer();

    // 异步 不阻塞主线程 方式获取buffer Data
    global.TimerCallback = function (buffer) {
        console.log("<<<<<<<<---------------->>>>>>>>");
        console.log("async callback from C++  buffer value: " + buffer + " , and data type : " + typeof (buffer));
        console.log(">>>>>>>>----------------<<<<<<<<<");
    }
    global.logBridge.setCallback(global.TimerCallback);
    // 阻塞JavaScript 方式调用JavaScript   十分不建议在C++ 中处理耗时内容
    setInterval(function () {
        console.log("---------------->>>>>>>>=========");
        // 输入buffer ， C++ 内部实现在拿到buffer 数据之后对数据的后部分做 “加一” 操作  
        // 返回数据应该为 Hello C++ Beepo"""
        var bufSource = Buffer.from("Hello C++ Addon!!!");
        console.log("bufSource is : " + bufSource);
        var buffer =  global.logBridge.getNewBuffer(bufSource);
        console.log("Direct return buffer is : " + buffer);
        console.log("----------------<<<<<<<<<=========");
    }, 60);
})()