#ifndef LISTENER_RUNNER_H
#define LISTENER_RUNNER_H

#include "third_party/blink/renderer/core/dom/events/event.h"

namespace blink {

// the code is collected from the existing Chrome's `EventTarget::fireEventListeners`
void RunEventListener(EventTarget* event_target,
                      EventListenerVector* entry,
                      int idx_in_entry,
                      Event* event,
                      ExecutionContext* context,
                      bool flag_should_report,
                      double report_time);

// the code is collected from the existing Chrome's `EventDispatcher::DispatchEventPostProcess`
void RunDispatchEventPostProcess(Node* activation_target,
                                 EventDispatchHandlingState* pre_dispatch_event_handler_result,
                                 Node* node_, Event* event_);
};

#endif // LISTENER_RUNNER_H
