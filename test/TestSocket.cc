/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 14:45:20
 * @Contact: 2458006466@qq.com
 * @Description: TestSocket
 */
#include "Core/Socket.h"
#include "Core/InetAddress.h"
#include "Base/Logger.h"

using namespace NAMESPACE;

int main(int argc, char *argv[]) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  CHECK(sockfd != -1) << "Socket create error.";
  Socket socket(sockfd);
  InetAddress addr("0.0.0.0", 8080);
  socket.BindAddress(addr);
  socket.Listen();

  InetAddress clnt_addr;
  int clnt_sockfd = socket.Accept(&clnt_addr);
  LogInfo("new client fd {}, IP {} Port {}.", clnt_sockfd, clnt_addr.toIp(), clnt_addr.toPort());

  while (true) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    ssize_t read_bytes = read(clnt_sockfd, buf, sizeof(buf));
    if (read_bytes > 0) {
      // 读数据并写回
      LogInfo("Message from clint fd {}:{}.", clnt_sockfd, buf);
      ssize_t write_bytes = write(clnt_sockfd, buf, sizeof(buf));
      LogInfo("write {} bytes to clinet.", write_bytes);
    } else if (read_bytes == 0) {
      // 表示EOF
      LogInfo("Client fd {} disconnected.", clnt_sockfd);
      close(clnt_sockfd);
      break;
    } else if (read_bytes == -1) {
      // 有错误发生
      close(clnt_sockfd);
      LogError("Socket read error.");
    }
  }
  return 0;
}
