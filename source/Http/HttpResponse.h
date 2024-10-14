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
  HttpResponse();
  ~HttpResponse();

  void Init(const std::string &root_path, const std::string &path, bool is_keep_alive = false, int code = -1);
  void MakeResponse(const std::string &str, const std::string &type, Buffer *buffer = nullptr);
  void MakeResponse(Buffer *buffer);

  char *file() {
    return mm_file;
  }

  size_t fileSize() const {
    return file_stat_.st_size;
  }

private:
  void AddStateLine(Buffer *buffer);
  void AddHeaders(Buffer *buffer);

  void AddErrorHtml();
  const std::string getFileType(const std::string &path) const;
  void AddErrorBody(const std::string &err_msg, Buffer *buffer);
  void AddContentType(const std::string &type, Buffer *buffer);

  void UnmapFile();

private:
  std::string root_path_ {};
  std::string path_ {};
  int code_ {-1};
  bool is_keep_alive_ {false};
  char *mm_file {nullptr};
  struct stat file_stat_{0};
};

NAMESPACE_END
