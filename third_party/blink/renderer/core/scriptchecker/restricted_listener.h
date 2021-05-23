#ifndef RESTRICTED_LISTENER_H
#define RESTRICTED_LISTENER_H

#include "third_party/blink/renderer/core/dom/events/event.h"
#include "base/scriptchecker/capability.h"

namespace blink {

struct CORE_EXPORT ListenerTaskArgs {
  RegisteredEventListener registered_listener;
  bool flag_should_report;
  double report_time;

  base::scriptchecker::Capability* capability;

  ListenerTaskArgs();
  ListenerTaskArgs(RegisteredEventListener* registered_listener,
                   bool flag_should_report, double report_time,
                   base::scriptchecker::Capability* capability);
};

class CORE_EXPORT RestrictedListener {
 public:
  RestrictedListener();
  ~RestrictedListener();

  // add a restricted listener, which will be run in a new restricted task
  void AddRestrictedListener(ListenerTaskArgs args,
                             EventTarget* event_target,
                             Event* event,
                             ExecutionContext* context);
  // add a terminate task after all restricted listener tasks are done
  void AddTerminateTask(Node* activation_target,
                        EventDispatchHandlingState* pre_dispatch_event_handler_result,
                        Node* node_, Event* event_);

  ListenerTaskArgs* GetListenerTaskArgs(size_t idx);
  size_t GetNumberOfTasks();

  bool HasRestrictedListener();
  void Clear();

 private:
  void RunEventListenerInScriptCheck();

  bool has_restricted_listener_;
  std::vector<ListenerTaskArgs> listener_task_args_;
};

extern CORE_EXPORT RestrictedListener* g_restricted_listener;
void CORE_EXPORT InitRestrictedListenerForScriptChecker();
}

#endif // RESTRICTED_LISTENER_H
