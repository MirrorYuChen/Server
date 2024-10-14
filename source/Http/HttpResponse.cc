/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:20:18
 * @Contact: 2458006466@qq.com
 * @Description: HttpResponse
 */
#include "Http/HttpResponse.h"
#include "Base/Buffer.h"
#include "Base/Logger.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <unordered_map>

NAMESPACE_BEGIN
const std::unordered_map<std::string, std::string> SuffixToType = {
  { ".html",  "text/html" },
  { ".xml",   "text/xml" },
  { ".xhtml", "application/xhtml+xml" },
  { ".txt",   "text/plain" },
  { ".rtf",   "application/rtf" },
  { ".pdf",   "application/pdf" },
  { ".word",  "application/nsword" },
  { ".png",   "image/png" },
  { ".gif",   "image/gif" },
  { ".jpg",   "image/jpeg" },
  { ".jpeg",  "image/jpeg" },
  { ".au",    "audio/basic" },
  { ".mpeg",  "video/mpeg" },
  { ".mpg",   "video/mpeg" },
  { ".avi",   "video/x-msvideo" },
  { ".gz",    "application/x-gzip" },
  { ".tar",   "application/x-tar" },
  { ".css",   "text/css "},
  { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> CodeToMessage = {
  { 200, "OK" },
  { 400, "Bad Request" },
  { 403, "Forbidden" },
  { 404, "Not Found" },
  { 405, "Method Not Allowed" },
};

const std::unordered_map<int, std::string> CodeToPath = {
  { 400, "/400.html" },
  { 403, "/403.html" },
  { 404, "/404.html" },
  { 405, "/405.html" },
};

HttpResponse::HttpResponse() = default;
HttpResponse::~HttpResponse() {
  UnmapFile();
}

void HttpResponse::Init(const std::string &root_path, const std::string &path, bool is_keep_alive, int code) {
  code_ = code;
  is_keep_alive_ = is_keep_alive;
  if (mm_file) {
    UnmapFile();
  }
  path_ = path;
  root_path_ = root_path;
  file_stat_ = { 0 };
}

void HttpResponse::AddStateLine(Buffer *buffer) {
  std::string code_msg;
  if (CodeToMessage.find(code_) != CodeToMessage.end()) {
    code_msg = CodeToMessage.at(code_);
  } else {
    code_ = 400;
    code_msg = CodeToMessage.at(400);
  }
  buffer->Append("HTTP/1.1 " + std::to_string(code_) + " " + code_msg + "\r\n");
}

void HttpResponse::AddHeaders(Buffer *buffer) {
  buffer->Append("Connection: ");
  if (is_keep_alive_) {
    buffer->Append("keep-alive\r\n");
    buffer->Append("Keep-Alive: timeout=120, max=6\r\n");
  } else {
    buffer->Append("close\r\n");
  }
}

void HttpResponse::AddContentType(const std::string &type, Buffer *buffer) {
  buffer->Append("Content-Type: " + type + "\r\n");
}

void HttpResponse::MakeResponse(const std::string &body, const std::string &type, Buffer *buffer) {
  AddStateLine(buffer);
  AddHeaders(buffer);
  AddContentType(type, buffer);
  buffer->Append("Content-Length: " + std::to_string(body.size()) + "\r\n");
  buffer->Append("\r\n");
  buffer->Append(body);
}

void HttpResponse::UnmapFile() {
  if (mm_file) {
    munmap(mm_file, file_stat_.st_size);
    mm_file = nullptr;
  }
}

void HttpResponse::MakeResponse(Buffer *buffer) {
  // 1.请求资源文件检查
  if(stat((root_path_ + path_).data(), &file_stat_) < 0 || S_ISDIR(file_stat_.st_mode)) {
    code_ = 404;
  } else if(!(file_stat_.st_mode & S_IROTH)) {
    code_ = 403;
  } else if(code_ == -1) { 
    code_ = 200; 
  }

  // 2.错误资源文件检查
  AddErrorHtml();

  // 3.添加相应状态行
  AddStateLine(buffer);

  // 4.添加头部信息
  AddHeaders(buffer);

  // 5.添加文件类型
  AddContentType(getFileType(path_), buffer);

  // 6.添加文件内容
  LogInfo("add body: {}.", root_path_ + path_);
  int fd = open((root_path_ + path_).c_str(), O_RDONLY);
  if (fd < 0) {
    AddErrorBody("File not found!", buffer);
    return;
  }
  void *mm_ret = mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mm_ret == MAP_FAILED) {
    AddErrorBody("File not found!", buffer);
    return;
  }
  mm_file = static_cast<char *>(mm_ret);
  close(fd);
  buffer->Append("Content-Length: " + std::to_string(file_stat_.st_size) + "\r\n\r\n");
}

void HttpResponse::AddErrorHtml() {
  if (CodeToPath.find(code_) != CodeToPath.end()) {
    path_ = CodeToPath.at(code_);
    stat((root_path_ + path_).data(), &file_stat_);
  }
}

const std::string HttpResponse::getFileType(const std::string &path) const {
  auto idx = path.find_last_of(".");
  if (idx == std::string::npos) {
    return "text/plain";
  }
  std::string suffix = path.substr(idx);
  if (SuffixToType.find(suffix) != SuffixToType.end()) {
    return SuffixToType.at(suffix);
  } else {
    return "text/plain";
  }
}

void HttpResponse::AddErrorBody(const std::string &err_msg, Buffer *buffer) {
  std::string body = "<html><title>Error</title><body bgcolor=\"ffffff\">";
  std::string code_msg;
  if (CodeToMessage.find(code_) != CodeToMessage.end()) {
    code_msg = CodeToMessage.at(code_);
  } else {
    code_msg = "Bad Request";
  }
  body += std::to_string(code_) + " : " + code_msg  + "\n";
  body += "<p>" + err_msg + "</p>";
  body += "<hr><em>HttpServer</em></body></html>";

  buffer->Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
  buffer->Append(body);
}

NAMESPACE_END
