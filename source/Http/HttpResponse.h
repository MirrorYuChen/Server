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
  void AddBodyString(const std::string &str, const std::string &type);
  void AddBodyFile(const std::string &path);

  void setCode(int code) { code_ = code; }
  void setIsKeepAlive(bool is_keep_alive) { is_keep_alive_ = is_keep_alive; }

  Buffer *getOutputBuffer() { return output_buffer_.get(); }

private:
  void AddStateLine();
  void AddHeaders();

  void AddErrorHtml();
  const std::string getFileType(const std::string &path) const;
  void AddErrorBody(const std::string &err_msg);
  void AddContentType(const std::string &type);

private:
  std::string root_path_;
  int code_ {-1};
  bool is_keep_alive_ {false};
  struct stat file_stat_{0};
  std::unique_ptr<Buffer> output_buffer_ {nullptr};
};

NAMESPACE_END
