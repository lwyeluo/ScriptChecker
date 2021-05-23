/*
 * Separate some of the listener runner from current blink, for the purpose of
 * run restricted listener in new child task
 *  Author: Luo Wu
 */

#include "base/macros.h"
#include "base/scriptchecker/global.h"
#include "third_party/blink/renderer/core/scriptchecker/listener_runner.h"
#include "third_party/blink/renderer/core/scriptchecker/restricted_listener.h"
#include "third_party/blink/renderer/core/frame/PerformanceMonitor.h"
#include "third_party/blink/renderer/bindings/core/v8/script_event_listener.h"
#include "third_party/blink/renderer/platform/histogram.h"
#include "third_party/blink/renderer/core/probe/core_probes.h"
#include "third_party/blink/renderer/bindings/core/v8/source_location.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/events/mouse_event.h"
#include "third_party/blink/renderer/core/frame/settings.h"
#include "third_party/blink/renderer/core/frame/use_counter.h"

namespace blink {

enum PassiveForcedListenerResultType {
  kPreventDefaultNotCalled,
  kDocumentLevelTouchPreventDefaultCalled,
  kPassiveForcedListenerResultTypeMax
};

Event::PassiveMode EventPassiveMode(
    const RegisteredEventListener& event_listener) {
  if (!event_listener.Passive()) {
    if (event_listener.PassiveSpecified())
      return Event::PassiveMode::kNotPassive;
    return Event::PassiveMode::kNotPassiveDefault;
  }
  if (event_listener.PassiveForcedForDocumentTarget())
    return Event::PassiveMode::kPassiveForcedDocumentLevel;
  if (event_listener.PassiveSpecified())
    return Event::PassiveMode::kPassive;
  return Event::PassiveMode::kPassiveDefault;
}

bool IsInstrumentedForAsyncStack(const AtomicString& event_type) {
  return event_type == EventTypeNames::load ||
         event_type == EventTypeNames::error;
}

void ReportBlockedEvent(ExecutionContext* context,
                        const Event* event,
                        RegisteredEventListener* registered_listener,
                        double delayed_seconds) {
  if (registered_listener->Callback()->GetType() !=
      EventListener::kJSEventListenerType)
    return;

  String message_text = String::Format(
      "Handling of '%s' input event was delayed for %ld ms due to main thread "
      "being busy. "
      "Consider marking event handler as 'passive' to make the page more "
      "responsive.",
      event->type().GetString().Utf8().data(), lround(delayed_seconds * 1000));

  PerformanceMonitor::ReportGenericViolation(
      context, PerformanceMonitor::kBlockedEvent, message_text, delayed_seconds,
      GetFunctionLocation(context, registered_listener->Callback()));
  registered_listener->SetBlockedEventWarningEmitted();
}

void RunEventListener(EventTarget* event_target,
                      RegisteredEventListener* registered_listener,
                      Event* event,
                      ExecutionContext* context,
                      bool flag_should_report,
                      double report_time) {
  EventListener* listener = registered_listener->Callback();

  event->SetHandlingPassive(EventPassiveMode(*registered_listener));
  bool passive_forced = registered_listener->PassiveForcedForDocumentTarget();

  probe::UserCallback probe(context, nullptr, event->type(), false, event_target);
  probe::AsyncTask async_task(
      context, V8AbstractEventListener::Cast(listener), "event",
      IsInstrumentedForAsyncStack(event->type()));

  // To match Mozilla, the AT_TARGET phase fires both capturing and bubbling
  // event listeners, even though that violates some versions of the DOM spec.
  listener->handleEvent(context, event);
  //fired_listener = true;

  /* Added by Luo Wu */
#ifdef SCRIPT_CHECKER_PRINT_SECURITY_MONITOR_LOG
  if(base::scriptchecker::g_script_checker
          && base::PlatformThread::CurrentId() == 1) {
    LOG(INFO) << ">>> [Event] EventTarget::FireEventListeners. Finished one listener: "
              << "[event, capability] = " << event->type() << ", "
              << base::scriptchecker::g_script_checker->GetCurrentTaskCapabilityAsJSString();
  }
#endif
  /* Added End */

  // If we're about to report this event listener as blocking, make sure it
  // wasn't removed while handling the event.
  if (flag_should_report) {
    ReportBlockedEvent(context, event, registered_listener, report_time);
  }

  if (passive_forced) {
    DEFINE_STATIC_LOCAL(EnumerationHistogram, passive_forced_histogram,
                        ("Event.PassiveForcedEventDispatchCancelled",
                         kPassiveForcedListenerResultTypeMax));
    PassiveForcedListenerResultType breakage_type = kPreventDefaultNotCalled;
    if (event->PreventDefaultCalledDuringPassive())
      breakage_type = kDocumentLevelTouchPreventDefaultCalled;

    passive_forced_histogram.Count(breakage_type);
  }

  event->SetHandlingPassive(Event::PassiveMode::kNotPassive);
}

void RunDispatchEventPostProcess(Node* activation_target,
                                 EventDispatchHandlingState* pre_dispatch_event_handler_result,
                                 Node* node_, Event* event_) {
  event_->SetTarget(EventPath::EventTargetRespectingTargetRules(*node_));
  // https://dom.spec.whatwg.org/#concept-event-dispatch
  // 14. Unset event’s dispatch flag, stop propagation flag, and stop immediate
  // propagation flag.
  event_->SetStopPropagation(false);
  event_->SetStopImmediatePropagation(false);
  // 15. Set event’s eventPhase attribute to NONE.
  event_->SetEventPhase(0);
  // TODO(rakina): investigate this and move it to the bottom of step 16
  // 17. Set event’s currentTarget attribute to null.
  event_->SetCurrentTarget(nullptr);

  bool is_click = event_->IsMouseEvent() &&
                  ToMouseEvent(*event_).type() == EventTypeNames::click;
  if (is_click) {
    // Fire an accessibility event indicating a node was clicked on.  This is
    // safe if m_event->target()->toNode() returns null.
    if (AXObjectCache* cache = node_->GetDocument().ExistingAXObjectCache())
      cache->HandleClicked(event_->target()->ToNode());

    // Pass the data from the PreDispatchEventHandler to the
    // PostDispatchEventHandler.
    // This may dispatch an event, and node_ and event_ might be altered.
    if (activation_target) {
      activation_target->PostDispatchEventHandler(
          event_, pre_dispatch_event_handler_result);
    }
    // TODO(tkent): Is it safe to kick DefaultEventHandler() with such altered
    // event_?
  }

  // The DOM Events spec says that events dispatched by JS (other than "click")
  // should not have their default handlers invoked.
  bool is_trusted_or_click =
      !RuntimeEnabledFeatures::TrustedEventsDefaultActionEnabled() ||
      event_->isTrusted() || is_click;

  // For Android WebView (distinguished by wideViewportQuirkEnabled)
  // enable untrusted events for mouse down on select elements because
  // fastclick.js seems to generate these. crbug.com/642698
  // TODO(dtapuska): Change this to a target SDK quirk crbug.com/643705
  if (!is_trusted_or_click && event_->IsMouseEvent() &&
      event_->type() == EventTypeNames::mousedown &&
      IsHTMLSelectElement(*node_)) {
    if (Settings* settings = node_->GetDocument().GetSettings()) {
      is_trusted_or_click = settings->GetWideViewportQuirkEnabled();
    }
  }

  // Call default event handlers. While the DOM does have a concept of
  // preventing default handling, the detail of which handlers are called is an
  // internal implementation detail and not part of the DOM.
  if (!event_->defaultPrevented() && !event_->DefaultHandled() &&
      is_trusted_or_click) {
    // Non-bubbling events call only one default event handler, the one for the
    // target.
    node_->WillCallDefaultEventHandler(*event_);
    node_->DefaultEventHandler(event_);
    DCHECK(!event_->defaultPrevented());
    // For bubbling events, call default event handlers on the same targets in
    // the same order as the bubbling phase.
    if (!event_->DefaultHandled() && event_->bubbles()) {
      size_t size = event_->GetEventPath().size();
      for (size_t i = 1; i < size; ++i) {
        event_->GetEventPath()[i].GetNode()->WillCallDefaultEventHandler(
            *event_);
        event_->GetEventPath()[i].GetNode()->DefaultEventHandler(event_);
        DCHECK(!event_->defaultPrevented());
        if (event_->DefaultHandled())
          break;
      }
    }
  }

  // Track the usage of sending a mousedown event to a select element to force
  // it to open. This measures a possible breakage of not allowing untrusted
  // events to open select boxes.
  if (!event_->isTrusted() && event_->IsMouseEvent() &&
      event_->type() == EventTypeNames::mousedown &&
      IsHTMLSelectElement(*node_)) {
    UseCounter::Count(node_->GetDocument(),
                      WebFeature::kUntrustedMouseDownEventDispatchedToSelect);
  }
  // 16. If target's root is a shadow root, then set event's target attribute
  // and event's relatedTarget to null.
  event_->SetTarget(event_->GetEventPath().GetWindowEventContext().Target());
  if (!event_->target())
    event_->SetRelatedTargetIfExists(nullptr);
}

}
