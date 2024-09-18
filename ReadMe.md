## Linux服务器编程学习
 - Author：chenjingyu
 - Date：20240910

## 1.项目描述
 - Linux端服务器搭建学习笔记及代码；

## 2.更新内容
日期 | 更新内容
--|--
20240910 | 1.创建项目，添加Logger模块封装；2.添加Socket和InetAddress封装；
20240911 | 1.添加poller，Channel，EventLoop封装；2.添加EventLoopThread和EventLoopThreadPool相关封装和实现；3.添加TcpConnection和Acceptor相关代码实现；4.添加TcpServer及EchoServer测试用例；
20240918 | 1.添加Http服务基本实现及bug修复；

## 3.使用方法
 - [1] 编译运行server
```bash
>> mkdir build && cd build && ./TestServer
[2024-09-14 09:51:12.961] [chenjingyu] [info] [TestServer.cc: 59] pid = 2485
[2024-09-14 09:51:12.961] [chenjingyu] [info] [Acceptor.cc: 24] Acceptor create nonblocking socket [fd = 6]
[2024-09-14 09:51:12.961] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-09-14 09:51:12.961] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-09-14 09:51:12.961] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-09-14 09:51:12.961] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
```
 - [2] 命令模拟client
```bash
>> nc 127.0.0.1 8080
aadadadad
aadadadad
112121
112121
hello
hello
```

## 参考资料
 - [1] [30dayMakeCppServer](https://github.com/yuesong-feng/30dayMakeCppServer)
 - [2] [muduo](https://github.com/chenshuo/muduo)
 - [3] [A-Tiny-Network-Library](https://github.com/Shangyizhou/A-Tiny-Network-Library)
 - [4] [TinyWebServer](https://github.com/qinguoyi/TinyWebServer)

