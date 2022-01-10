/*
 * The core of ScriptCheck to run restricted listener in a new child task
 *  Author: Luo Wu
 */

#include "third_party/blink/renderer/core/scriptchecker/restricted_listener.h"
#include "third_party/blink/renderer/core/scriptchecker/listener_runner.h"

#include "base/scriptchecker/global.h"
#include "base/scriptchecker/task_type.h"

namespace blink {

RestrictedListener* g_restricted_listener = nullptr;

void InitRestrictedListenerForScriptChecker() {
  if(g_restricted_listener)
    return;
  g_restricted_listener = new RestrictedListener();
}

void RunRestrictedListener(size_t idx_for_pending_tasks,
                           EventTarget* event_target,
                           Event* event,
                           ExecutionContext* context) {
  bool is_last_task = (idx_for_pending_tasks + 1 == g_restricted_listener->GetNumberOfTasks());
  ListenerTaskArgs* args = g_restricted_listener->
          GetListenerTaskArgs(idx_for_pending_tasks);
  if(is_last_task) {
      g_restricted_listener->ResetRestrictedListenerFlag();
      // all pending listeners are run, so clear it
      g_restricted_listener->Clear();
  }
  RunEventListener(event_target, &args->registered_listener, event,
                   context, args->flag_should_report,
                   args->report_time);
}

ListenerTaskArgs::ListenerTaskArgs(
        RegisteredEventListener* in_registered_listener,
        bool flag_should_report, double report_time,
        base::scriptchecker::Capability* capability) :
    //registered_listener(registered_listener),
    flag_should_report(flag_should_report),
    report_time(report_time), capability(capability) {
  registered_listener = *in_registered_listener;
}

ListenerTaskArgs::ListenerTaskArgs() :
    registered_listener(RegisteredEventListener()), flag_should_report(false),
    report_time(0), capability(nullptr) {}

RestrictedListener::RestrictedListener(){
  has_restricted_listener_ = false;
  listener_task_args_.clear();
}

RestrictedListener::~RestrictedListener() = default;

void RestrictedListener::AddRestrictedListener(ListenerTaskArgs args,
                                               EventTarget* event_target,
                                               Event* event,
                                               ExecutionContext* context) {
  has_restricted_listener_ = true;
  listener_task_args_.push_back(args);
  size_t num = listener_task_args_.size() - 1;

  base::PendingTask pending_task(
              FROM_HERE, base::BindOnce(&RunRestrictedListener, num, event_target, event, context),
              TimeTicks(), base::Nestable::kNestable, listener_task_args_[num].capability,
              base::scriptchecker::TaskType::RESTRICTED_LISTENER_TASK);
  pending_task.sequence_num = -num-1; // just to identify the task
  base::scriptchecker::g_script_checker->RecordNewAsyncExecTask(std::move(pending_task));
}

void RestrictedListener::AddTerminateTask(Node* activation_target,
                                          EventDispatchHandlingState* pre_dispatch_event_handler_result,
                                          Node* node_, Event* event_) {
  size_t num = listener_task_args_.size() - 1;

  base::PendingTask pending_task(
              FROM_HERE, base::BindOnce(&RunDispatchEventPostProcess,
                                        activation_target,
                                        pre_dispatch_event_handler_result,
                                        node_, event_),
              TimeTicks(), base::Nestable::kNestable,
              listener_task_args_[num].capability,
              base::scriptchecker::TaskType::RESTRICTED_LISTENER_TASK);
  pending_task.sequence_num = -num-2; // just to identify the task
  base::scriptchecker::g_script_checker->RecordNewAsyncExecTask(std::move(pending_task));
}

ListenerTaskArgs* RestrictedListener::GetListenerTaskArgs(size_t idx) {
  DCHECK(idx < GetNumberOfTasks());
  return &listener_task_args_[idx];
}

size_t RestrictedListener::GetNumberOfTasks() {
  return listener_task_args_.size();
}

bool RestrictedListener::HasRestrictedListener() {
  return has_restricted_listener_;
}

void RestrictedListener::ResetRestrictedListenerFlag() {
  has_restricted_listener_ = false;
}

void RestrictedListener::Clear() {
  has_restricted_listener_ = false;
  for(size_t i = 0; i < listener_task_args_.size(); i ++) {
    listener_task_args_.pop_back();
  }
}

};
