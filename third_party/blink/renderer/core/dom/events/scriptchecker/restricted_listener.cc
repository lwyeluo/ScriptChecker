/*
 *
 *
 */

#include "third_party/blink/renderer/core/dom/events/scriptchecker/restricted_listener.h"
#include "third_party/blink/renderer/core/dom/events/scriptchecker/listener_runner.h"

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
  ListenerTaskArgs* args = g_restricted_listener->
          GetListenerTaskArgs(idx_for_pending_tasks);
  RunEventListener(event_target, args->entry,
                   args->idx_in_entry, event,
                   context, args->flag_should_report,
                   args->report_time);
}

ListenerTaskArgs::ListenerTaskArgs(
        EventListenerVector* entry, int idx_in_entry,
        bool flag_should_report, double report_time,
        base::scriptchecker::Capability* capability) :
    entry(entry), idx_in_entry(idx_in_entry),
    flag_should_report(flag_should_report),
    report_time(report_time), capability(capability) {}

ListenerTaskArgs::ListenerTaskArgs() :
    entry(nullptr), idx_in_entry(0), flag_should_report(false),
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

  LOG(INFO) << "EventTarget::FireEventListeners. listener is restricted, push it as a new task."
            << ", " << args.entry->size() << ", " << args.idx_in_entry;

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

  LOG(INFO) << "RestrictedListener::AddTerminateTask.";

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

void RestrictedListener::Clear() {
  has_restricted_listener_ = false;
  for(size_t i = 0; i < listener_task_args_.size(); i ++) {
    listener_task_args_.pop_back();
  }
}

};
