#ifndef HOST_SCRIPT_CHECKER_HELPER_H
#define HOST_SCRIPT_CHECKER_HELPER_H

#include <string>

namespace base {

namespace scriptchecker {

/*
 * Record a data pushed into |HostScriptChecker::queue_to_save_info|
 *    The relationship between the functions which pop/push data into queue is:
 *
 *        format:   message name \n src -> dst
 *
 *    kRenderFrameMessageFilter_SetCookie_Name
 *        InterfaceEndpointClient::HandleIncomingMessage  -> RenderFrameMessageFilter::SetCookie
 *    kRenderFrameMessageFilter_GetCookies_Name
 *        InterfaceEndpointClient::HandleIncomingMessage  -> RenderFrameMessageFilter::GetCookies
 *
 */
struct QueueData {
  std::string data_;
  std::string function_; // which function push this data

  QueueData(std::string data, std::string function) :
    data_(data), function_(function) {}
  QueueData() : data_(""), function_("") {}
};

#ifdef DCHECK_SCRIPT_CHECKER_DEBUG
  // use it for debug
  #define DCHECK_QUEUE_DATA_FUNCTION(queueData, intendFunction) { \
      if(queueData.function_ != intendFunction) { \
        LOG(INFO) << "[ERROR] validate queueData failed. [data, intendFunction] = " \
                  << queueData.data_ << ", " << queueData.function_ << ", " \
                  << intendFunction; \
        DCHECK(queueData.function_ == intendFunction); \
      } \
    }

  // use it for debug
  //    every time we poped all data, the size of queue should equal 0
  #define DCHECK_QUEUE_SIZE() { \
      int size = base::switcher::g_switcher_host_helper->size(); \
      DCHECK(size == 0); \
    }

  #define DCHECK_QUEUE_DATA_FUNCTION_WITH_SIZE(queueData, intendFunction) { \
      DCHECK_QUEUE_DATA_FUNCTION(queueData, intendFunction) \
      DCHECK_QUEUE_SIZE(); \
  }

#else
  #define DCHECK_QUEUE_DATA_FUNCTION(queueData, intendFunction)
  #define DCHECK_QUEUE_SIZE()
  #define DCHECK_QUEUE_DATA_FUNCTION_WITH_SIZE(queueData, intendFunction)
#endif

}

}

#endif // HOST_SCRIPT_CHECKER_HELPER_H
