#ifndef ASYNC_EXEC_QUEUE_H
#define ASYNC_EXEC_QUEUE_H

#include <map>
#include <string>
#include "base/logging.h"
#include "base/pending_task.h"
#include "base/containers/queue.h"

namespace base {

namespace scriptchecker {

/*
 * ScriptChecker uses async execution for JS code. To maintain the code logic, the
 *  task created by `setTimeoutWR(xxx, xxx, 0)` should be run as soon as possible. so we
 *  record these tasks
 */
 class AsyncExecQueue {
  public:
   void Push(PendingTask&&);
   void Clear();
   void RunAll();

   AsyncExecQueue();
   ~AsyncExecQueue();

  private:
   std::vector<PendingTask> async_exec_queue_;
 };

}
}

#endif // ASYNC_EXEC_QUEUE_H
