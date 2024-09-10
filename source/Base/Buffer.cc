/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 15:11:42
 * @Contact: 2458006466@qq.com
 * @Description: Buffer
 */
#include "Base/Buffer.h"
#include "Base/Logger.h"
#include <sys/uio.h>
#include <errno.h>

NAMESPACE_BEGIN
static const constexpr char kCRLF[] = "\r\n";

Buffer::Buffer(size_t init_sz) :
  buffer_(kCheapPrepend + init_sz),
  reader_idx_(kCheapPrepend),
  writer_idx_(kCheapPrepend) {
  
}

size_t Buffer::ReadableBytes() const {
  return writer_idx_ - reader_idx_;
}

size_t Buffer::WriteableBytes() const {
  return buffer_.size() - writer_idx_;
}

size_t Buffer::PrependableBytes() const {
  return reader_idx_;
}

// 缓冲区中可读数据的起始地址
const char *Buffer::peek() const {
  return begin() + reader_idx_;
}

void Buffer::RetrieveUtil(const char *end) {
  Retrieve(end - peek());
}

void Buffer::Retrieve(size_t len) {
  if (len < ReadableBytes()) {
    reader_idx_ += len;
  } else {
    RetrieveAll();
  }
}

void Buffer::RetrieveAll() {
  reader_idx_ = kCheapPrepend;
  writer_idx_ = kCheapPrepend;
}

std::string Buffer::RetrieveAllAsString() {
  return RetrieveAsString(ReadableBytes());
}

std::string Buffer::RetrieveAsString(size_t len) {
  std::string result(peek(), len);
  Retrieve(len);
  return result;
}

void Buffer::EnsureWriteableBytes(size_t len) {
  if (WriteableBytes() < len) {
    MakeSpace(len);
  }
}

void Buffer::Append(const std::string &str) {
  Append(str.data(), str.size());
}

void Buffer::Append(const char *data, size_t len) {
  EnsureWriteableBytes(len);
  std::copy(data, data + len, begin() + writer_idx_);
}

const char *Buffer::FindCRLF() const {
  const char *crlf = std::search(peek(), begin() + writer_idx_, kCRLF, kCRLF + 2);
  return crlf == begin() + writer_idx_ ? NULL : crlf;
}

char *Buffer::begin() {
  return buffer_.data();
}

const char *Buffer::begin() const {
  return buffer_.data();
}

void Buffer::MakeSpace(int len) {
  // 1.整个buffer都不够用
  if (WriteableBytes() + PrependableBytes() < len + kCheapPrepend) {
    buffer_.resize(writer_idx_ + len);
  } else {
    // 2.整个buffer够用，将后面移动到前面继续分配
    size_t readable = ReadableBytes();
    std::copy(
      begin() + reader_idx_, 
      begin() + writer_idx_, 
      begin() + kCheapPrepend
    );
    reader_idx_ = kCheapPrepend;
    writer_idx_ = reader_idx_ + readable;
  }
}

ssize_t Buffer::ReadFd(int fd, int *save_errno) {
  char extra_buf[65536] = {0};   // 64Kb
  // 1.使用iovec分配两个连续缓冲区
  struct iovec vec[2];
  const size_t writeable = WriteableBytes();

  // 2.第一缓冲区指向可写空间
  vec[0].iov_base = begin() + writer_idx_;
  vec[0].iov_len = writeable;

  // 3.第二缓冲区指向栈空间
  vec[1].iov_base = extra_buf;
  vec[1].iov_len = sizeof(extra_buf);

  // 4.判断使用那一块缓冲区
  const int iov_cnt = (writeable < sizeof(extra_buf)) ? 2 : 1;
  const ssize_t n = readv(fd, vec, iov_cnt);

  if (n < 0) {
    *save_errno = errno;
  } else if (n <= writeable) {
    // buffer可写缓冲区足够存储读出来数据
    writer_idx_ += n;
  } else {
    // extra_buf中写入n - writeable长度数据
    writer_idx_ = buffer_.size();
    Append(extra_buf, n - writeable);
  }

  return n;
}

ssize_t Buffer::WriteFd(int fd, int *save_errno) {
  ssize_t n = write(fd, begin(), ReadableBytes());
  if (n < 0) {
    *save_errno = errno;
  }
  return n;
}

NAMESPACE_END
