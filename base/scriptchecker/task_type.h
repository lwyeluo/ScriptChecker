#ifndef TASK_TYPE_H
#define TASK_TYPE_H

#include <map>
#include <string>
#include "base/logging.h"

namespace base {

namespace scriptchecker {

class TaskType {
  public:
    static const int NORMAL_TASK = 0;
    static const int IPC_TASK = 1;
    static const int NORMAL_TIMER_TASK = 2;
    static const int SETTIMEOUTWR_DELAY_ZERO_TIMER_TASK = 3;
    static const int SETTIMEOUTWR_DELAY_NONZERO_TIMER_TASK = 4;
    static const int RESTRICTED_LISTENER_TASK = 5;
    static const int RESTRICTED_FRAME_PARSER_TASK = 6;
    static const int NORMAL_FRAME_PARSER_TASK = 7;
    // chrome's own scheduler task, we should not assign capability to them
    static const int SCHEDULER_TASK = 8;
};

}
}

#endif // TASK_TYPE_H
