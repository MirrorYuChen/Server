/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 15:11:37
 * @Contact: 2458006466@qq.com
 * @Description: Buffer
 */
#pragma once

#include "Api.h"
#include <vector>
#include <algorithm>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      reader_idx   <=    writer_idx    <=     size
NAMESPACE_BEGIN
class API Buffer {
public:
  static const constexpr size_t kCheapPrepend = 8;
  static const constexpr size_t kInitialSize = 1024;

  explicit Buffer(size_t init_size = kInitialSize);

  size_t ReadableBytes() const;

  size_t WriteableBytes() const;

  size_t PrependableBytes() const;

  // 缓冲区中可读数据的起始地址
  const char *peek() const;

  void RetrieveUtil(const char *end);

  void Retrieve(size_t len);

  void RetrieveAll();

  std::string RetrieveAllAsString();

  std::string RetrieveAsString(size_t len);

  void EnsureWriteableBytes(size_t len);

  void Append(const std::string &str);

  void Append(const char *data, size_t len);

  const char *FindCRLF() const;

  ssize_t ReadFd(int fd, int *save_errno);
  ssize_t WriteFd(int fd, int *save_errno);

private:
  char *begin();
  const char *begin() const;
  void MakeSpace(int len);

private:
  std::vector<char> buffer_;
  size_t reader_idx_;
  size_t writer_idx_;
};

NAMESPACE_END
