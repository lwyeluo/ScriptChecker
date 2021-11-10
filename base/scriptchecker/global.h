#ifndef GLOBAL_H
#define GLOBAL_H

#include "base/scriptchecker/script_checker.h"
#include "base/scriptchecker/host_script_checker.h"

namespace base {

namespace scriptchecker {

// to inspect the task relationships
//#define SCRIPT_CHECKER_INSPECT_TASK_SCEDULER

// to print the result of task-based capability check
#define SCRIPT_CHECKER_PRINT_SECURITY_MONITOR_LOG

// some logs for micro-benchmark
//#define SCRIPT_CHECKER_INSPECT_TIME_USAGE

//seperate frame parser
//#define SCRIPT_CHECKER_SEPERATE_FRAME_PARSER

/*
 * core logic of #ifdef
 */

#ifdef SCRIPT_CHECKER_INSPECT_TIME_USAGE

// when we test the time usage, undefine the logs of security monitor
#undef SCRIPT_CHECKER_PRINT_SECURITY_MONITOR_LOG

#define TIME_MEASUREMENT_BEGIN \
  uint64_t begin, end; \
  timeval tBegin, tEnd, tDiff; \
  gettimeofday(&tBegin, 0); \
  begin = base::scriptchecker::_rdtsc();

#define TIME_MEASUREMENT_END \
  end = base::scriptchecker::_rdtsc(); \
  gettimeofday(&tEnd, 0); \
  base::scriptchecker::subTimeVal(tDiff, tBegin, tEnd); \
  bool is_risky_task = base::scriptchecker::g_script_checker ? \
    base::scriptchecker::g_script_checker->IsCurrentTaskWithRestricted() : 0; \
  LOG(INFO) << base::scriptchecker::g_name << __FUNCTION__ \
            << "[is_risky_task, cpu_cycle, time] = " << is_risky_task << ", " \
            << (end - begin) << ", " \
            << (tDiff.tv_sec * 1000000 + tDiff.tv_usec) << "μs! "

#define TIME_MEASUREMENT_END_WITH_DATA(datakey, datavalue) \
  end = base::scriptchecker::_rdtsc(); \
  gettimeofday(&tEnd, 0); \
  base::scriptchecker::subTimeVal(tDiff, tBegin, tEnd); \
  bool is_risky_task = base::scriptchecker::g_script_checker ? \
    base::scriptchecker::g_script_checker->IsCurrentTaskWithRestricted() : 0; \
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

// ScriptChecker running in the renderer process
extern ScriptChecker* BASE_EXPORT g_script_checker;
void BASE_EXPORT initScriptChecker();

// ScriptChecker running in the browser kernel process
extern HostScriptChecker* BASE_EXPORT g_host_script_checker;
void BASE_EXPORT initHostScriptChecker();


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
