/*
 * @Author: chenjingyu
 * @Date: 2024-05-27 14:35:47
 * @Contact: 2458006466@qq.com
 * @Description: Api
 */
#pragma once

#include "Platform.h"

#define EXTERN_C extern "C"

#if OS_WINDOWS
#define API_IMPORT __declspec(dllimport)
#define API_EXPORT __declspec(dllexport)
#define API_LOCAL
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define API_IMPORT __attribute__((visibility("default")))
#define API_EXPORT __attribute__((visibility("default")))
#define API_LOCAL  __attribute__((visibility("hidden")))
#else
#define API_IMPORT
#define API_EXPORT
#define API_LOCAL
#endif
#endif

#define API API_EXPORT

#ifdef __cplusplus
#define C_API EXTERN_C API
#else
#define C_API API
#endif

#define NAMESPACE mirror

#define NAMESPACE_BEGIN namespace NAMESPACE {
#define NAMESPACE_END   }  // namespace mirror

NAMESPACE_BEGIN
#ifndef M_PI
#define M_PI 3.14159265358979323846  // pi
#endif

#define MAX_(x, y) ((x) > (y) ? (x) : y)
#define MIN_(x, y) ((x) < (y) ? (x) : y)

#define NOT_ALLOWED_COPY(Type)                                                                                         \
  Type(const Type &)  = delete;                                                                                        \
  const Type &operator=(const Type &) = delete;

#define NOT_ALLOWED_MOVE(Type)                                                                                         \
  Type(const Type &) = delete;                                                                                         \
  Type &operator=(const Type &) = delete;                                                                              \
  Type(Type &&) noexcept        = delete;                                                                              \
  Type &operator=(Type &&) noexcept = delete;

#ifdef __has_builtin
#define HAVE_BUILTIN(x) __has_builtin(x)
#else
#define HAVE_BUILTIN(x) 0
#endif

#ifdef __has_feature
#define HAVE_FEATURE(f) __has_feature(f)
#else
#define HAVE_FEATURE(f) 0
#endif

#if HAVE_BUILTIN(__builtin_expect) || (defined(__GNUC__) && !defined(__clang__))
#define PREDICT_FALSE(x) (__builtin_expect(false || (x), false))
#define PREDICT_TRUE(x)  (__builtin_expect(false || (x), true))
#else
#define PREDICT_FALSE(x) (x)
#define PREDICT_TRUE(x)  (x)
#endif

NAMESPACE_END
