/*
 * The core of ScriptCheck running in the browser kernel process
 *  Author: Luo Wu
 */
#include "base/scriptchecker/global.h"
#include "base/scriptchecker/host_script_checker.h"
#include "base/scriptchecker/capability.h"
#include "base/scriptchecker/capability_definition.h"

namespace base {

namespace scriptchecker {

HostScriptChecker::HostScriptChecker() {}

HostScriptChecker::~HostScriptChecker() {
  for(auto &v : queue_to_save_info) {
    while(!v.second.empty())
      v.second.pop();
  }
  queue_to_save_info.clear();
}

// security monitor
bool HostScriptChecker::DisallowedToAccessCookie() {
  QueueData queueData = getAndPop();
  DCHECK_QUEUE_DATA_FUNCTION_WITH_SIZE(queueData, "HandleIncomingMessage");

  Capability* capability = new Capability();
  capability->SetFromIPCMessage(queueData.data_);
  return CapabilityDefinition::DisallowedToAccessCookie(capability->GetBitmap());
}

// queue
QueueData HostScriptChecker::getAndPop() {
  int tid = base::PlatformThread::CurrentId();
  if(queue_to_save_info.find(tid) == queue_to_save_info.end())
    return QueueData();
  if (queue_to_save_info[tid].empty())
    return QueueData();
  QueueData ret = queue_to_save_info[tid].front();
//  LOG(INFO) << "*** pop it to queue: [data, function, tid] = "
//            << ret.data_ << ", " << ret.function_ << ", " << tid;
  queue_to_save_info[tid].pop();
  return ret;
}

QueueData HostScriptChecker::get() {
  int tid = base::PlatformThread::CurrentId();
  if(queue_to_save_info.find(tid) == queue_to_save_info.end())
    return QueueData();
  if (queue_to_save_info[tid].empty())
    return QueueData();
  return queue_to_save_info[tid].front();
}

void HostScriptChecker::push(QueueData queueData) {
  int tid = base::PlatformThread::CurrentId();
  if(queue_to_save_info.find(tid) == queue_to_save_info.end()) {
    std::queue<QueueData> q;
    q.push(queueData);
    queue_to_save_info[tid] = q;
  } else {
    queue_to_save_info[tid].push(queueData);
  }
//  LOG(INFO) << "*** push it to queue: [data, function, tid] = "
//            << queueData.data_ << ", " << queueData.function_ << ", "
//            << tid;
}

int HostScriptChecker::size() {
  int tid = base::PlatformThread::CurrentId();
  if(queue_to_save_info.find(tid) == queue_to_save_info.end())
    return true;
  return queue_to_save_info[tid].size();
}

}

}
