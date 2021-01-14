/*
 * The queue of ScriptCheck running in the renderer process
 *  Author: Luo Wu
 */
#include "base/scriptchecker/async_exec_queue.h"
#include "base/scriptchecker/global.h"
#include "base/optional.h"

namespace base {

namespace scriptchecker {

AsyncExecQueue::AsyncExecQueue() {}

AsyncExecQueue::~AsyncExecQueue() {
  Clear();
}

void AsyncExecQueue::Push(PendingTask&& task) {
  async_exec_queue_.push_back(std::move(task));
}

void AsyncExecQueue::Clear() {
  while(!async_exec_queue_.empty())
    async_exec_queue_.pop_back();
}

void AsyncExecQueue::RunAll() {
  for(size_t i = 0; i < async_exec_queue_.size(); i ++) {
    base::Optional<base::PendingTask> task = std::move(async_exec_queue_[i]);

    LOG(INFO) << g_name << "\tRun Async Exec Task [tid] = " << task->sequence_num;
    //task_annotator->RunTask(__FUNCTION__, &*task);
    base::scriptchecker::g_script_checker->UpdateCurrentTask(&*task);
    std::move(task->task).Run();
  }
  Clear();
}

}
}
