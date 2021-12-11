#ifndef HOST_SCRIPT_CHECKER_H
#define HOST_SCRIPT_CHECKER_H

#include <map>
#include <queue>
#include <string>
#include "base/logging.h"

#include "base/scriptchecker/host_script_checker_helper.h"

// to capture logs for webpages, e.g., Aleax top 1000
#define SCRIPT_CHECKER_TEST_WEBPAGE

namespace base {

namespace scriptchecker {

class BASE_EXPORT HostScriptChecker {
  public:
    HostScriptChecker();
    ~HostScriptChecker();

    // security monitor
    bool DisallowedToAccessCookie();

  //================================================================
  //           Queue to save Site for IPC msg
  //================================================================
  public:

    QueueData getAndPop();
    QueueData get();
    void push(QueueData queueData);
    int size();

  private:
    // queue to save Site for IPC message
    //   key: gettid()
    //   value: a queue to transfer QueueData
    std::map<int, std::queue<QueueData> > queue_to_save_info;
};

}

}

#endif // HOST_SCRIPT_CHECKER_H
