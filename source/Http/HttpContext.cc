/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:55:54
 * @Contact: 2458006466@qq.com
 * @Description: HttpContext
 */
#include "Http/HttpContext.h"
#include "Base/Buffer.h"
#include "Base/Logger.h"
#include "Database/ConnectionPool.h"
#include <regex>
#include <unordered_set>
#include <unordered_map>

NAMESPACE_BEGIN
const std::unordered_set<std::string> DefaultHtml {
  "/index", "/register", "/login",
  "/welcome", "/video", "/image",
};

const std::unordered_map<std::string, int> DefaultHtmlTag {
  {"/register.html", 0}, {"/login.html", 1},
};

static int ConverHex(char ch) {
  if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  return ch;
}

static bool Verify(const std::string &username, const std::string &passward, bool is_login) {
  ConnectionPoolRAII raii(ConnectionPool::getInstance());
  if (is_login) {
    // 1.登录验证
    std::string sql_query = "SELECT * FROM user WHERE username = '" + username + "' AND password = '" + passward + "'";
    auto result = raii.Query(sql_query);
    if (!result) {
      return false;
    }
    return true;
  } else {
    // 2.注册验证
    std::string sql_query = "SELECT * FROM user WHERE username = '" + username + "' LIMITE 1";
    auto result = raii.Query(sql_query);
    if (result) {
      return false;
    }
    // 3.注册用户
    std::string sql_insert = "INSERT INTO user (username, password) VALUES ('" + username + "', '" + passward + "')";
    return raii.Execute(sql_insert);
  }
}

HttpContext::HttpContext() : state_(kExpectRequestLine) {

}

HttpContext::~HttpContext() = default;

bool HttpContext::ParseRequestLine(const std::string &line) {
  LogDebug("ParseRequestLine: {}", line);
  std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
  std::smatch match;
  if (std::regex_match(line, match, pattern)) {
    req_.setMethod(match[1].str());
    req_.setPath(match[2].str());
    req_.setVersion(match[3].str());
    state_ = kExpectHeaders;
    return true;
  }
  return false;
}

void HttpContext::ParseHeaders(const std::string &line) {
  LogDebug("ParseHeaders: {}", line);
  std::regex pattern("^([^:]*): ?(.*)$");
  std::smatch match;
  if (std::regex_match(line, match, pattern)) {
    req_.AddHeader(match[1].str(), match[2].str());
  } else {
    state_ = kExpectBody;
  }
}

void HttpContext::ParseBody(const std::string &line) {
  LogDebug("ParseBody: {}", line);
  req_.setBody(line);
  ParsePosts();
  state_ = kGotAll;
}

void HttpContext::ParsePosts() {
  LogInfo("ParsePosts: method: {}, Content-Type: {}", req_.methodString(), req_.getHeader("Content-Type"));
  // 1.解析post相关信息
  if (req_.methodString() != "POST" || req_.getHeader("Content-Type") != "application/x-www-form-urlencoded") {
    return;
  }
  std::string body = req_.body();
  if (body.empty()) {
    return;
  }
  std::string key, value;
  int num = 0;
  int n = body.size();
  int i = 0, j = 0;
  for (; i < n; ++i) {
    char ch = body[i];
    switch (ch) {
      case '=': {
        key = body.substr(j, i - j);
        j = i + 1;
        break;
      }
      case '+': {
        body[i] = ' ';
        break;
      }
      case '%': {
        num = ConverHex(body[i + 1]) * 16 + ConverHex(body[i + 2]);
        body[i + 2] = num % 10 + '0';
        body[i + 1] = (num / 10) + '0';
        i += 2;
        break;
      }
      case '&': {
        value = body.substr(j, i - j);
        j = i + 1;
        req_.AddPost(key, value);
        LogDebug("key: {}, value: {}", key, value);
        break;
      }
      default: {
        break;
      }
    }  
  }
  assert(j <= i);
  if (req_.posts().count(key) == 0 && j < i) { 
    value = body.substr(j, i - j);
    req_.AddPost(key, value);
  }

  // 2.解析登录和注册信息
  if (req_.path() == "/0" || req_.path() == "/1") {
    std::string username = req_.getPost("username");
    std::string password = req_.getPost("password");
    LogInfo("username: {}, password: {}", username, password);
    bool is_login = (req_.path() == "/1");
    if (Verify(username, password, is_login)) {
      req_.setPath("/welcome.html");
    } else {
      req_.setPath("/405.html");
    }
  }
}

void HttpContext::ParsePath() {
  std::string path = req_.path();
  if (path == "/") {
    path = "/index.html";
  } else {
    if (DefaultHtml.find(path) != DefaultHtml.end()) {
      path += ".html";
    }
  }
  req_.setPath(path);
}


bool HttpContext::ParseRequest(Buffer *buffer, Timestamp recv_time) {
  if (buffer->ReadableBytes() <= 0) {
    return false;
  }
  while (buffer->ReadableBytes() > 0 && state_ != kGotAll) {
    const char *crlf = buffer->FindCRLF();
    if (!crlf) {
      LogInfo("no CRLF found, wait for more data");
      break;
    }
    std::string line(buffer->peek(), crlf);
    switch (state_) {
      case kExpectRequestLine:
        if (!ParseRequestLine(line)) {
          return false;
        }
        ParsePath();
        break;
      case kExpectHeaders:
        ParseHeaders(line);
        if (buffer->ReadableBytes() <= 2) {
          LogInfo("kGotAll");
          state_ = kGotAll;
        }
        break;
      case kExpectBody:
        ParseBody(line);
        break;
      default:
        break;
    }
    if (crlf == buffer->beginWrite()) {
      break;
    }
    buffer->RetrieveUtil(crlf + 2);
  }
  LogDebug("[{}], [{}], [{}]", req_.methodString(), req_.path(), req_.version());
  return true;
}

NAMESPACE_END
