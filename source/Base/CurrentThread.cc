/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 10:26:15
 * @Contact: 2458006466@qq.com
 * @Description: CurrentThread
 */
#include "Base/CurrentThread.h"

NAMESPACE_BEGIN
namespace CurrentThread {
__thread int t_cached_tid = 0;

void cachedTid() {
  if (0 == t_cached_tid) {
    t_cached_tid = static_cast<pid_t>(syscall(SYS_gettid));
  }
}

} // namespace CurrentThread
NAMESPACE_END
