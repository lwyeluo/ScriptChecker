// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_WEBKIT_PUBLIC_COMMON_PAGE_LAUNCHING_PROCESS_STATE_H_
#define THIRD_PARTY_WEBKIT_PUBLIC_COMMON_PAGE_LAUNCHING_PROCESS_STATE_H_

#include "build/build_config.h"
#include "third_party/blink/common/common_export.h"

namespace blink {

// This file is used to maintain a consistent initial set of state between the
// RendererProcessHostImpl and the RendererSchedulerImpl.
#if defined(OS_ANDROID)
// This matches Android's ChildProcessConnection state before OnProcessLaunched.
constexpr bool kLaunchingProcessIsBackgrounded = true;
constexpr bool kLaunchingProcessIsBoostedForPendingView = true;
#else
constexpr bool kLaunchingProcessIsBackgrounded = false;
constexpr bool kLaunchingProcessIsBoostedForPendingView = false;
#endif

}  // namespace blink

#endif  // THIRD_PARTY_WEBKIT_PUBLIC_COMMON_PAGE_LAUNCHING_PROCESS_STATE_H_
