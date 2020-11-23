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
    static const int IPC_LISTENER = 2;
};

}
}

#endif // TASK_TYPE_H
