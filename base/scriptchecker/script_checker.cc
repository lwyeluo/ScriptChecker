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
            << task->sequence_num << ", " << task->capability.ToString();
}

void ScriptChecker::RecordNewTask(PendingTask *task) {
  DCHECK(task);
  if(IsCurrentTaskWithRestricted()) {
    LOG(INFO) << g_name << "ScriptChecker::RecordNewTask with restricted capabilty. [seq, task_type, cap] = "
              << task->sequence_num << ", " << task->task_type
              //<< ", " << task->capability.ToString()
              << ", " << m_current_task_->IsTaskRestricted() << ", "
              //<< m_current_task_->capability.ToString() << ", "
              << m_current_task_->sequence_num;
    // try to update the capability with its parent task
    switch(task->task_type) {
      case TaskType::NORMAL_TASK:
        // the parent is the current task
        task->SetCapability(GetCurrentTaskCapability());
        break;
      case TaskType::IPC_TASK:
        // its capabilty is set accoding to the IPC message, see ScriptChecker::RecordIPCTask
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

bool ScriptChecker::IsCurrentTaskWithRestricted() {
  return m_current_task_ ? m_current_task_->IsTaskRestricted() : false;
}

Capability* ScriptChecker::GetCurrentTaskCapability() {
  return &m_current_task_->capability;
}

}

}
