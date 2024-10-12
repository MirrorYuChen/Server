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

void HttpResponse::Init(bool is_keep_alive, int code) {
  code_ = code;
  is_keep_alive_ = is_keep_alive;
  output_buffer_.reset(new Buffer());
}

void HttpResponse::AddStateLine() {
  std::string code_msg;
  if (CodeToMessage.find(code_) != CodeToMessage.end()) {
    code_msg = CodeToMessage.at(code_);
  } else {
    code_ = 400;
    code_msg = CodeToMessage.at(400);
  }
  output_buffer_->Append("HTTP/1.1 " + std::to_string(code_) + " " + code_msg + "\r\n");
}

void HttpResponse::AddHeaders() {
  output_buffer_->Append("Connection: ");
  if (is_keep_alive_) {
    output_buffer_->Append("keep-alive\r\n");
    output_buffer_->Append("Keep-Alive: timeout=120, max=6\r\n");
  } else {
    output_buffer_->Append("close\r\n");
  }
}

void HttpResponse::AddContentType(const std::string &type) {
  output_buffer_->Append("Content-Type: " + type + "\r\n");
}

void HttpResponse::AddBodyString(const std::string &body, const std::string &type) {
  AddStateLine();
  AddHeaders();
  AddContentType(type);
  output_buffer_->Append("Content-Length: " + std::to_string(body.size()) + "\r\n");
  output_buffer_->Append("\r\n");
  output_buffer_->Append(body);
}

void HttpResponse::AddBodyFile(const std::string &path) {
  std::string res_path = path;
  // 1.请求资源文件检查
  if(stat(res_path.data(), &file_stat_) < 0 || S_ISDIR(file_stat_.st_mode)) {
    code_ = 404;
  } else if(!(file_stat_.st_mode & S_IROTH)) {
    code_ = 403;
  } else if(code_ == -1) { 
    code_ = 200; 
  }

  // 2.错误资源文件检查
  auto idx = res_path.find_last_of("/");
  if (idx != std::string::npos) {
    std::string sub_path = res_path.substr(0, idx);
    if (CodeToPath.find(code_) != CodeToPath.end()) {
      res_path = sub_path + CodeToPath.at(code_);
      stat((res_path).c_str(), &file_stat_);
    }
  }

  // 3.添加相应状态行
  AddStateLine();

  // 4.添加头部信息
  AddHeaders();

  // 5.添加文件类型
  AddContentType(getFileType(res_path));

  // 5.添加文件内容
  LogInfo("add body: {}.", res_path);
  int fd = open(res_path.c_str(), O_RDONLY);
  if (fd < 0) {
    AddErrorBody("File not found!");
    return;
  }
  output_buffer_->Append("Content-Length: " + std::to_string(file_stat_.st_size) + "\r\n");
  output_buffer_->Append("\r\n");
  int save_errno = 0;
  int n = output_buffer_->ReadFd(fd, &save_errno);
  if (n < 0) {
    AddErrorBody("Read file error!");
  }
  close(fd);
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

void HttpResponse::AddErrorBody(const std::string &err_msg) {
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

  output_buffer_->Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
  output_buffer_->Append(body);
}

NAMESPACE_END
