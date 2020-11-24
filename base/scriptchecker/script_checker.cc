/*
 * The core of ScriptCheck
 *  Author: Luo Wu
 */
#include "base/scriptchecker/global.h"
#include "base/scriptchecker/task_type.h"

namespace base {

namespace scriptchecker {

ScriptChecker::ScriptChecker() {
  m_current_task_ = nullptr;
  m_capability_definition = new CapabilityDefinition();
}

ScriptChecker::~ScriptChecker() {
  delete m_capability_definition;
}

void ScriptChecker::UpdateCurrentTask(PendingTask* task) {
  DCHECK(task);
  m_current_task_ = task;
  LOG(INFO) << g_name << "ScriptChecker::UpdateCurrentTask. [seq, cap] = "
            << task->sequence_num << ", " << task->capability_.ToString();
}

void ScriptChecker::RecordNewTask(PendingTask *task) {
  DCHECK(task);
  if(IsCurrentTaskWithRestricted()) {
    LOG(INFO) << g_name << "ScriptChecker::RecordNewTask with restricted capabilty. [seq, task_type, cap] = "
              << task->sequence_num << ", " << task->task_type_in_scriptchecker_
              //<< ", " << task->capability.ToString()
              << ", " << m_current_task_->IsTaskRestricted() << ", "
              //<< m_current_task_->capability.ToString() << ", "
              << m_current_task_->sequence_num;
    // try to update the capability with its parent task
    switch(task->task_type_in_scriptchecker_) {
      case TaskType::NORMAL_TASK:
        // the parent is the current task
        task->SetCapability(GetCurrentTaskCapability());
        break;
      case TaskType::IPC_TASK:
        // its capabilty is set accoding to the IPC message, see ScriptChecker::RecordIPCTask
        break;
      case TaskType::TIMER_TASK:
        // we have set the capability according to the JS API's parameter, here we need to
        //  ensure that the assigned capability does not breach its parent
        task->NarrowDownCapability(&m_current_task_->capability_);
        break;
      default:
        // here is unreachable
        NOTREACHED();
    }
  }
}

void ScriptChecker::RecordIPCTask(std::string capability_attached_in_ipc_message) {
  // here should be attached into the current task which forms as IPC_TASK
  DCHECK(m_current_task_);
  m_current_task_->SetCapabilityFromIPCMessage(capability_attached_in_ipc_message);
}

void ScriptChecker::RecordTIMERTask(std::string capability_from_js_string, bool is_restricted) {
  // here should be attached into the current task which forms as TIMER_TASK
  DCHECK(m_current_task_);
  m_current_task_->SetCapabilityFromJSString(capability_from_js_string);
}

bool ScriptChecker::IsCurrentTaskWithRestricted() {
  return m_current_task_ ? m_current_task_->IsTaskRestricted() : false;
}

Capability* ScriptChecker::GetCurrentTaskCapability() {
  return &m_current_task_->capability_;
}

bool ScriptChecker::MatchWithCapabilityJSRules(std::string in_capability_js_str,
                                               uint64_t& out_capability_bit_map,
                                               std::map<std::string, bool>& out_capability_js_wl) {
  return m_capability_definition->rule.Match(in_capability_js_str,
                                             out_capability_bit_map,
                                             out_capability_js_wl);
}

std::string ScriptChecker::ToStringFromCapabilityBitmap(uint64_t capability_bitmap) {
  return m_capability_definition->rule.ToStringFromBitMap(capability_bitmap);
}

CapabilityDefinition* ScriptChecker::GetCapabilityJSStringRules() {
  return m_capability_definition;
}

}

}
