#include "Http/HttpContext.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"
#include "Http/HttpServer.h"
#include "Base/Timestamp.h"
#include <fstream>
#include <sstream>

using namespace NAMESPACE;

void onRequest(const HttpRequest &req, HttpResponse *resp) {
  std::cout << "Headers " << req.methodString() << " " << req.path()
            << std::endl;

  const std::unordered_map<std::string, std::string> &headers = req.headers();
  for (const auto &header : headers) {
    std::cout << header.first << ": " << header.second << std::endl;
  }

  if (req.path() == "/") {
    std::string now = Timestamp::Now().toFormattedString();
    resp->AddBodyString(
      "<html><head><title>This is title</title></head>"
      "<body><h1>Hello</h1>Now is " + now + "</body></html>", 
      "text/html"
    );
  } else if (req.path() == "/index") {
    resp->AddBodyFile("/index.html");
  } else if (req.path() == "/login") {
    resp->AddBodyFile("/login.html");
  } else if (req.path() == "/image") {
    resp->AddBodyFile("/image.html");
  } else if (req.path() == "/video") {
    resp->AddBodyFile("/video.html");
  } else if (req.path() == "/hello") {
    resp->AddBodyString("hello, world!\n", "text/plain");
  } else {
    resp->setCode(404);
    resp->setIsKeepAlive(false);
    resp->AddBodyString("Not Found", "text/plain");
  }
}

int main(int argc, char *argv[]) {
  setLogPath("./log.txt");
  setLogLevel(1);
  EventLoop loop;
  HttpServer server(&loop, InetAddress("0.0.0.0", 8080), "http-server", "../data/resources/html");
  server.setHttpCallback(onRequest);
  server.Start();
  loop.Loop();
}
