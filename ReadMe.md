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
20241015 | 1.完善Http服务实现，参考TinyWebServer，添加HttpServer内容；

## 3.使用方法
### 3.1 简单的echo服务器案例
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
### 3.2 TinyWebServer案例
 - [1] Win11的WSL安装MySQL：
 ```bash
# 系统环境：Win11的WSL
# 1.安装mysql相关模块
>> sudo apt-get install mysql-server -y
>> sudo apt-get install libmysqlclient-dev -y
 ```
 - [2] 配置MySQL
```bash
# 1.进入mysql
>> sudo mysql
# 2.进入mysql数据库
mysql> use mysql;
# 3.修改root用户密码
mysql> ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '123456';
# 4.修改远程访问
mysql> UPDATE user SET `Host` = '%' WHERE `User` = 'root' LIMIT 1;
mysql> flush privileges;
mysql> exit;
# 5.创建数据库
mysql> create database webdb;
# 6.给创建的数据库创建表
mysql> use webdb;
mysql> create table user(
  username char(50) NULL,
  passwd char(50) NULL
) engine=InnoDB;
# 7.创建用户
mysql> create user 'mirror'@'localhost' identified by 'cjy';
# 8.给用户mirror表访问权限
mysql> grant all on webdb.user to 'mirror'@'localhost';
# 9.刷新系统权限，即时生效
mysql> flush privileges;
```
 - [3] 修改`test/TestHttpServer.cc`文件：
```C++
server.InitDatabase("localhost", "mirror", "cjy", "webdb", 3306, 8);
```
 - [4] 编译运行HttpServer
```bash
>> mkdir build && cd build && ./TestHttpServer
[2024-10-15 09:53:09.630] [chenjingyu] [info] [Acceptor.cc: 24] Acceptor create nonblocking socket [fd = 6]
[2024-10-15 09:53:09.637] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.637] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.637] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.638] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.638] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.639] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.639] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.639] [chenjingyu] [info] [ConnectionPool.cc: 55] connect mysql succeed!
[2024-10-15 09:53:09.639] [chenjingyu] [info] [HttpServer.cc: 40] HttpServer [http-server] starts listening on 0.0.0.0:8080.
[2024-10-15 09:53:09.639] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-10-15 09:53:09.640] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-10-15 09:53:09.640] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-10-15 09:53:09.640] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
[2024-10-15 09:53:09.640] [chenjingyu] [info] [EventLoop.cc: 68] EventLoop start looping.
```
 - [5] 浏览器访问[访问地址](http://127.0.0.1:8080)，进行相关操作即可


## 参考资料
 - [1] [30dayMakeCppServer](https://github.com/yuesong-feng/30dayMakeCppServer)
 - [2] [muduo](https://github.com/chenshuo/muduo)
 - [3] [A-Tiny-Network-Library](https://github.com/Shangyizhou/A-Tiny-Network-Library)
 - [4] [TinyWebServer](https://github.com/qinguoyi/TinyWebServer)

