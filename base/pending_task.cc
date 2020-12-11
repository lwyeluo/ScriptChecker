// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/pending_task.h"

#include "base/message_loop/message_loop.h"

#include "base/scriptchecker/global.h"
#include "base/scriptchecker/task_type.h"

namespace base {

PendingTask::PendingTask(const Location& posted_from,
                         OnceClosure task,
                         TimeTicks delayed_run_time,
                         Nestable nestable
                         /*Added by Luo Wu*/ ,
                         base::scriptchecker::Capability* capability,
                         int task_type_in_scriptchecker
                         /* Added End */)
    : task(std::move(task)),
      posted_from(posted_from),
      delayed_run_time(delayed_run_time),
      nestable(nestable) {
    /* Added by Luo Wu */
    capability_ = nullptr;
    task_type_in_scriptchecker_ = task_type_in_scriptchecker;
    if(capability && capability->IsRestricted())
      SetCapability(capability);
    /* End */
}

PendingTask::PendingTask(PendingTask&& other) = default;

PendingTask::~PendingTask() = default;

PendingTask& PendingTask::operator=(PendingTask&& other) = default;

bool PendingTask::operator<(const PendingTask& other) const {
  // Since the top of a priority queue is defined as the "greatest" element, we
  // need to invert the comparison here.  We want the smaller time to be at the
  // top of the heap.

  if (delayed_run_time < other.delayed_run_time)
    return false;

  if (delayed_run_time > other.delayed_run_time)
    return true;

  // If the times happen to match, then we use the sequence number to decide.
  // Compare the difference to support integer roll-over.
  return (sequence_num - other.sequence_num) > 0;
}

void PendingTask::SetCapability(base::scriptchecker::Capability* in_capability) {
  if(!capability_)
    capability_ = new base::scriptchecker::Capability();
  capability_->SetFrom(in_capability);
  has_set_capability = true;
}

void PendingTask::NarrowDownCapability(base::scriptchecker::Capability* current_task_capability) {
  if(!capability_)
    capability_ = new base::scriptchecker::Capability();
  capability_->NarrowDownFrom(current_task_capability);
  has_set_capability = true;
}

void PendingTask::SetCapabilityFromIPCMessage(std::string in_capabilty_attached_in_ipc) {
  if(!capability_)
    capability_ = new base::scriptchecker::Capability();
  capability_->SetFromIPCMessage(in_capabilty_attached_in_ipc);
  has_set_capability = true;
  task_type_in_scriptchecker_ = base::scriptchecker::TaskType::IPC_TASK;
}

void PendingTask::SetCapabilityFromJSString(std::string in_capabilty_specified_in_js_str) {
  if(!capability_)
    capability_ = new base::scriptchecker::Capability();
  capability_->SetFromJSString(in_capabilty_specified_in_js_str);
  has_set_capability = true;
}

std::string PendingTask::GetCapbilityAsJSString() {
  return capability_ ? capability_->ToJSString() : "";
}

std::string PendingTask::GetCapbilityAsIPCMessage() {
  return capability_ ? capability_->ToIPCString() : "";
}

bool PendingTask::IsTaskRestricted() {
  if(!has_set_capability)
    return false;
  DCHECK(capability_);
  return capability_->IsRestricted();
}

}  // namespace base
