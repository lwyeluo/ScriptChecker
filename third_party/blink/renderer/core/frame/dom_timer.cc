/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "third_party/blink/renderer/core/frame/dom_timer.h"

#include "third_party/blink/public/platform/task_type.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/inspector/InspectorTraceEvents.h"
#include "third_party/blink/renderer/core/probe/core_probes.h"
#include "third_party/blink/renderer/platform/instrumentation/tracing/trace_event.h"
#include "third_party/blink/renderer/platform/wtf/time.h"

#include "base/scriptchecker/global.h"
#include "base/scriptchecker/script_checker.h"
#include "base/scriptchecker/task_type.h"

namespace blink {

static const TimeDelta kMaxIntervalForUserGestureForwarding =
    TimeDelta::FromMilliseconds(1000);  // One second matches Gecko.
static const int kMaxTimerNestingLevel = 5;
// Chromium uses a minimum timer interval of 4ms. We'd like to go
// lower; however, there are poorly coded websites out there which do
// create CPU-spinning loops.  Using 4ms prevents the CPU from
// spinning too busily and provides a balance between CPU spinning and
// the smallest possible interval timer.
static constexpr TimeDelta kMinimumInterval = TimeDelta::FromMilliseconds(4);

static inline bool ShouldForwardUserGesture(TimeDelta interval,
                                            int nesting_level) {
  if (RuntimeEnabledFeatures::UserActivationV2Enabled())
    return false;
  return UserGestureIndicator::ProcessingUserGestureThreadSafe() &&
         interval <= kMaxIntervalForUserGestureForwarding &&
         nesting_level ==
             1;  // Gestures should not be forwarded to nested timers.
}

int DOMTimer::Install(ExecutionContext* context,
                      ScheduledAction* action,
                      TimeDelta timeout,
                      bool single_shot,
                      /* Added by Luo Wu */
                      const String& capability,
                      int task_type
                      /* End */) {
  int timeout_id = context->Timers()->InstallNewTimeout(context, action,
                                                        timeout, single_shot,
                                                        /* Added by Luo Wu */
                                                        capability.Ascii().data(),
                                                        task_type/* End */);
  return timeout_id;
}

void DOMTimer::RemoveByID(ExecutionContext* context, int timeout_id) {
  DOMTimer* timer = context->Timers()->RemoveTimeoutByID(timeout_id);
  TRACE_EVENT_INSTANT1("devtools.timeline", "TimerRemove",
                       TRACE_EVENT_SCOPE_THREAD, "data",
                       InspectorTimerRemoveEvent::Data(context, timeout_id));
  // Eagerly unregister as ExecutionContext observer.
  if (timer)
    timer->ClearContext();
}

DOMTimer::DOMTimer(ExecutionContext* context,
                   ScheduledAction* action,
                   TimeDelta interval,
                   bool single_shot,
                   int timeout_id,
                   /* Added by Luo Wu */
                   std::string capability,
                   int task_type
                   /* End */)
    : PausableTimer(context, TaskType::kJavascriptTimer),
      timeout_id_(timeout_id),
      nesting_level_(context->Timers()->TimerNestingLevel() + 1),
      action_(action) {
  DCHECK_GT(timeout_id, 0);
  if (ShouldForwardUserGesture(interval, nesting_level_)) {
    // Thread safe because shouldForwardUserGesture will only return true if
    // execution is on the the main thread.
    user_gesture_token_ = UserGestureIndicator::CurrentToken();
  }

  /* Added by Luo Wu */
  capability_ = nullptr;
  task_type_ = task_type;
  if(base::scriptchecker::g_script_checker &&
          capability != "" &&
          base::PlatformThread::CurrentId() == 1) {
    // record a new timer
#ifdef SCRIPT_CHECKER_PRINT_SECURITY_MONITOR_LOG
    LOG(INFO) << base::scriptchecker::g_name << "DOMTimer::DOMTimer. [cap] = "
              << capability;
#endif
    capability_ = new base::scriptchecker::Capability(capability);
  }
  /* Added End */

  TimeDelta interval_milliseconds =
      std::max(TimeDelta::FromMilliseconds(1), interval);
  /* Added by Luo Wu */
  if(task_type == base::scriptchecker::TaskType::SETTIMEOUTWR_DELAY_ZERO_TIMER_TASK) {
    // we need to schedule this task as soon as possible to maintain the code logic
    interval_milliseconds = interval;
  }
  /* Added End */
  if (interval_milliseconds < kMinimumInterval &&
      nesting_level_ >= kMaxTimerNestingLevel)
    interval_milliseconds = kMinimumInterval;
  if (single_shot)
    StartOneShot(interval_milliseconds, FROM_HERE
                 /* Added by Luo Wu */, capability_,
                 task_type /* Added End */);
  else
    StartRepeating(interval_milliseconds, FROM_HERE
                   /* Added by Luo Wu */, capability_,
                   task_type /* Added End */);

  PauseIfNeeded();
  TRACE_EVENT_INSTANT1("devtools.timeline", "TimerInstall",
                       TRACE_EVENT_SCOPE_THREAD, "data",
                       InspectorTimerInstallEvent::Data(context, timeout_id,
                                                        interval, single_shot));
  probe::AsyncTaskScheduledBreakable(
      context, single_shot ? "setTimeout" : "setInterval", this);
}

DOMTimer::~DOMTimer() {
  if (action_)
    action_->Dispose();
}

void DOMTimer::Stop() {
  probe::AsyncTaskCanceledBreakable(
      GetExecutionContext(),
      RepeatInterval() ? "clearInterval" : "clearTimeout", this);

  user_gesture_token_ = nullptr;
  // Need to release JS objects potentially protected by ScheduledAction
  // because they can form circular references back to the ExecutionContext
  // which will cause a memory leak.
  if (action_)
    action_->Dispose();
  action_ = nullptr;
  PausableTimer::Stop();
}

void DOMTimer::ContextDestroyed(ExecutionContext*) {
  Stop();
}

void DOMTimer::Fired() {
#ifdef SCRIPT_CHECKER_INSPECT_TIME_USAGE
  if(base::scriptchecker::g_script_checker) {
    base::scriptchecker::g_script_checker->FinishTimeMeasureForAsyncTask();
  }
#endif
  ExecutionContext* context = GetExecutionContext();
  DCHECK(context);
  context->Timers()->SetTimerNestingLevel(nesting_level_);
  DCHECK(!context->IsContextPaused());
  // Only the first execution of a multi-shot timer should get an affirmative
  // user gesture indicator.
  UserGestureIndicator gesture_indicator(std::move(user_gesture_token_));

  TRACE_EVENT1("devtools.timeline", "TimerFire", "data",
               InspectorTimerFireEvent::Data(context, timeout_id_));
  probe::UserCallback probe(context,
                            RepeatInterval() ? "setInterval" : "setTimeout",
                            AtomicString(), true);
  probe::AsyncTask async_task(context, this,
                              RepeatInterval() ? "fired" : nullptr);

  // Simple case for non-one-shot timers.
  if (IsActive()) {
    if (!RepeatIntervalDelta().is_zero() &&
        RepeatIntervalDelta() < kMinimumInterval) {
      nesting_level_++;
      if (nesting_level_ >= kMaxTimerNestingLevel)
        AugmentRepeatInterval(kMinimumInterval - RepeatIntervalDelta());
    }

    // No access to member variables after this point, it can delete the timer.
    action_->Execute(context);

    context->Timers()->SetTimerNestingLevel(0);

    return;
  }

  // Unregister the timer from ExecutionContext before executing the action
  // for one-shot timers.
  ScheduledAction* action = action_.Release();
  context->Timers()->RemoveTimeoutByID(timeout_id_);

#ifdef SCRIPT_CHECKER_INSPECT_TASK_SCEDULER
  if(base::scriptchecker::g_script_checker) {
    LOG(INFO) << base::scriptchecker::g_name << "DOMTimer::Fired. [id] = "
              << base::scriptchecker::g_script_checker->GetCurrentTaskID();
  }
#endif

  action->Execute(context);

  // ExecutionContext might be already gone when we executed action->execute().
  ExecutionContext* execution_context = GetExecutionContext();
  if (!execution_context)
    return;

  execution_context->Timers()->SetTimerNestingLevel(0);
  // Eagerly unregister as ExecutionContext observer.
  ClearContext();
  // Eagerly clear out |action|'s resources.
  action->Dispose();
}

scoped_refptr<base::SingleThreadTaskRunner> DOMTimer::TimerTaskRunner() const {
  return GetExecutionContext()->Timers()->TimerTaskRunner();
}

void DOMTimer::Trace(blink::Visitor* visitor) {
  visitor->Trace(action_);
  PausableTimer::Trace(visitor);
}

}  // namespace blink
