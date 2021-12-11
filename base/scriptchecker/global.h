#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include "base/logging.h"

namespace base {

namespace scriptchecker {

// some logs for micro-benchmark
//#define SCRIPT_CHECKER_INSPECT_TIME_USAGE

// to log events as baseline
#define LOG_MALICIOUS_EVENTS_AS_BASELINE

#ifdef LOG_MALICIOUS_EVENTS_AS_BASELINE
const char g_test_origin[] = "http://host.com:3001/";
#endif

/*
 * core logic of #ifdef
 */

#ifdef SCRIPT_CHECKER_INSPECT_TIME_USAGE

#define TIME_MEASUREMENT_BEGIN \
  uint64_t begin, end; \
  timeval tBegin, tEnd, tDiff; \
  gettimeofday(&tBegin, 0); \
  begin = base::scriptchecker::_rdtsc();

#define TIME_MEASUREMENT_END \
  end = base::scriptchecker::_rdtsc(); \
  gettimeofday(&tEnd, 0); \
  base::scriptchecker::subTimeVal(tDiff, tBegin, tEnd); \
  bool is_risky_task = 0; \
  LOG(INFO) << base::scriptchecker::g_name << __FUNCTION__ \
            << "[is_risky_task, cpu_cycle, time] = " << is_risky_task << ", " \
            << (end - begin) << ", " \
            << (tDiff.tv_sec * 1000000 + tDiff.tv_usec) << "μs! "

#define TIME_MEASUREMENT_END_WITH_DATA(datakey, datavalue) \
  end = base::scriptchecker::_rdtsc(); \
  gettimeofday(&tEnd, 0); \
  base::scriptchecker::subTimeVal(tDiff, tBegin, tEnd); \
  bool is_risky_task = 0; \
  LOG(INFO) << base::scriptchecker::g_name << __FUNCTION__ \
            << "[is_risky_task, cpu_cycle, time] = " << is_risky_task << ", " \
            << (end - begin) << ", " \
            << (tDiff.tv_sec * 1000000 + tDiff.tv_usec) << "μs! " \
            << "metadata:" << datakey << ":" << datavalue;

#else
#define TIME_MEASUREMENT_BEGIN
#define TIME_MEASUREMENT_END
#define TIME_MEASUREMENT_END_WITH_DATA(metadatakey, metadatavalue)
#endif

const char BASE_EXPORT g_name[] = "[SCRIPTCHECKER] ";

/************************************************************
 *
 *           Record Time Usages
 *
 **************************************************************/
bool BASE_EXPORT subTimeVal(timeval &result, timeval& begin, timeval& end);
extern "C" void rdtsc();
uint64_t BASE_EXPORT _rdtsc();

} // scriptchecker
} // base


#endif  // GLOBAL_H
