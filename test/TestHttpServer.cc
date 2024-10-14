/*
 * @Author: chenjingyu
 * @Date: 2024-10-14 11:37:37
 * @Contact: 2458006466@qq.com
 * @Description: TestHttpServer
 */
#include "Http/HttpContext.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"
#include "Http/HttpServer.h"
#include "Base/Timestamp.h"
#include <fstream>
#include <sstream>

using namespace NAMESPACE;

int main(int argc, char *argv[]) {
  setLogPath("./log.txt");
  setLogLevel(1);
  EventLoop loop;
  HttpServer server(&loop, InetAddress("0.0.0.0", 8080), "http-server", "../data/resources/html");
  server.Start();
  loop.Loop();
}
