// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_WEBKIT_SOURCE_PLATFORM_SCHEDULER_RENDERER_MAIN_THREAD_SCHEDULER_HELPER_H_
#define THIRD_PARTY_WEBKIT_SOURCE_PLATFORM_SCHEDULER_RENDERER_MAIN_THREAD_SCHEDULER_HELPER_H_

#include "third_party/blink/renderer/platform/scheduler/child/scheduler_helper.h"

#include "third_party/blink/renderer/platform/scheduler/renderer/main_thread_task_queue.h"

namespace blink {
namespace scheduler {

class RendererSchedulerImpl;

class PLATFORM_EXPORT MainThreadSchedulerHelper : public SchedulerHelper {
 public:
  MainThreadSchedulerHelper(
      std::unique_ptr<TaskQueueManager> task_queue_manager,
      RendererSchedulerImpl* renderer_scheduler);
  ~MainThreadSchedulerHelper() override;

  scoped_refptr<MainThreadTaskQueue> NewTaskQueue(
      const MainThreadTaskQueue::QueueCreationParams& params);

  scoped_refptr<MainThreadTaskQueue> DefaultMainThreadTaskQueue();
  scoped_refptr<MainThreadTaskQueue> ControlMainThreadTaskQueue();

 protected:
  scoped_refptr<TaskQueue> DefaultTaskQueue() override;
  scoped_refptr<TaskQueue> ControlTaskQueue() override;

 private:
  RendererSchedulerImpl* renderer_scheduler_;  // NOT OWNED

  const scoped_refptr<MainThreadTaskQueue> default_task_queue_;
  const scoped_refptr<MainThreadTaskQueue> control_task_queue_;

  DISALLOW_COPY_AND_ASSIGN(MainThreadSchedulerHelper);
};

}  // namespace scheduler
}  // namespace blink

#endif  // THIRD_PARTY_WEBKIT_SOURCE_PLATFORM_SCHEDULER_RENDERER_MAIN_THREAD_SCHEDULER_HELPER_H_
