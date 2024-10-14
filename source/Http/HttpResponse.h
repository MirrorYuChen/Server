/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:20:10
 * @Contact: 2458006466@qq.com
 * @Description: HttpResponce
 */
#pragma once

#include "Api.h"
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <memory>

NAMESPACE_BEGIN
class Buffer;
class API HttpResponse {
public:
  HttpResponse(const std::string &root_path);
  ~HttpResponse() = default;

  void Init(bool is_keep_alive = false, int code = -1);
  void MakeResponse(const std::string &str, const std::string &type, Buffer *buffer = nullptr);
  void MakeResponse(Buffer *buffer);

  void setCode(int code) { code_ = code; }
  void setPath(const std::string &path) { path_ = path; }
  void setIsKeepAlive(bool is_keep_alive) { is_keep_alive_ = is_keep_alive; }

private:
  void AddStateLine(Buffer *buffer);
  void AddHeaders(Buffer *buffer);

  void AddErrorHtml();
  const std::string getFileType(const std::string &path) const;
  void AddErrorBody(const std::string &err_msg, Buffer *buffer);
  void AddContentType(const std::string &type, Buffer *buffer);

private:
  std::string root_path_;
  std::string path_;
  int code_ {-1};
  bool is_keep_alive_ {false};
  struct stat file_stat_{0};
};

NAMESPACE_END
