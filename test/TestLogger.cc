/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 09:49:27
 * @Contact: 2458006466@qq.com
 * @Description: TestLogger
 */
#include "Base/Logger.h"

int main(int argc, char *argv[]) {
  setLogPath("./log.txt");
  for (int i = 0; i < 100; ++i) {
    LogInfo("i: {}.", i);
  }

  CHECK(1 > 2) << "Error Check.";
  CHECK_EQ(1, 1) << "Errpr Check equal.";
  CHECK_GE(3, 4) << "Error Check great equal.";

  return 0;
}
