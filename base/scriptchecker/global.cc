/*
 * The global variables of ScriptCheck
 *  Author: Luo Wu
 */
#include "base/scriptchecker/global.h"
#include "base/scriptchecker/capability.h"

namespace base {

namespace scriptchecker {

ScriptChecker* g_script_checker;
HostScriptChecker* g_host_script_checker;

void initScriptChecker() {
  if (!g_script_checker) {
    g_script_checker = new ScriptChecker();
    LOG(INFO) << g_name << "init script checker in renderer process...";
  }
}

void initHostScriptChecker() {
  if (!g_host_script_checker) {
    g_host_script_checker = new HostScriptChecker();
    LOG(INFO) << g_name << "init script checker in kernel process...";
  }
}

bool subTimeVal(timeval &result, timeval& begin, timeval& end) {
  if(begin.tv_sec > end.tv_sec)
    return false;
  if(begin.tv_sec == end.tv_sec && begin.tv_usec > end.tv_usec)
    return false;
  result.tv_sec = end.tv_sec - begin.tv_sec;
  result.tv_usec = end.tv_usec - begin.tv_usec;
  if(result.tv_usec < 0) {
    result.tv_sec --;
    result.tv_usec += 1000000;
  }
  return true;
}

extern "C" void rdtsc()
{
  __asm__ __volatile__
  (
   "rdtsc\n\t"  // low 32 -> eax, high 32 -> edx
   "shl $32, %%rdx\n\t"
   "or %%rdx, %%rax\n\t"
   "pop %%rbp\n\t"
   "retq\n\t"
   :::"rdx"
  );
}

// rdtsc for C++
uint64_t _rdtsc() {
  uint32_t hi, lo;
  __asm__ volatile (
    "rdtsc"
    : "=a"(lo), "=d"(hi)
  );
  return (uint64_t)hi << 32 | lo;
}

}

}
