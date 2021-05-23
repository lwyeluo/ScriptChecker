// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/script/classic_script.h"

#include "third_party/blink/renderer/bindings/core/v8/script_controller.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"

#include "base/scriptchecker/global.h"

namespace blink {

void ClassicScript::Trace(blink::Visitor* visitor) {
  Script::Trace(visitor);
  visitor->Trace(script_source_code_);
}

void ClassicScript::RunScript(LocalFrame* frame,
                              const SecurityOrigin* security_origin) const {
  frame->GetScriptController().ExecuteScriptInMainWorld(
      GetScriptSourceCode(), BaseURL(), FetchOptions(), access_control_status_);
}

/* Added by Luo Wu */
void ClassicScript::RunScriptInRiskyWorld(LocalFrame* frame,
                                          const SecurityOrigin* origin,
                                          String task_capability) const {
#ifdef SCRIPT_CHECKER_PRINT_SECURITY_MONITOR_LOG
  LOG(INFO) << base::scriptchecker::g_name
            << ">>> [RISKY] ClassicScript::RunScriptInRiskyWorld with capability "
            << task_capability;
#endif
  frame->GetScriptController().ExecuteScriptInRiskyWorld(
      GetScriptSourceCode(), BaseURL(), FetchOptions(), access_control_status_);
}
/* Addded End */

}  // namespace blink
