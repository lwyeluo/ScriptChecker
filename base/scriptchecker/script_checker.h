#ifndef SCRIPT_CHECKER_H
#define SCRIPT_CHECKER_H

#include <map>
#include <string>
#include "base/logging.h"
#include "base/scriptchecker/capability.h"
#include "base/scriptchecker/capability_definition.h"
#include "base/pending_task.h"

namespace base {

namespace scriptchecker {

class BASE_EXPORT ScriptChecker {
  public:
    ScriptChecker();
    ~ScriptChecker();

    // Task Scheduler
    void UpdateCurrentTask(PendingTask* task);
    // Task Recorder
    void RecordNewTask(PendingTask* task);
    void RecordIPCTask(std::string capability_attached_in_ipc_message);
    void RecordTIMERTask(std::string capability_from_js_string, bool is_restricted);

    bool IsCurrentTaskWithRestricted();
    Capability* GetCurrentTaskCapability();

    // Security Monitor
    bool DisallowedToAccessCookie();
    bool DisallowedToAccessNetwork();
    bool DisallowedToAccessDOM(bool is_ele_has_task_cap_attr);

    // update capability information with predefined rules
    bool MatchWithCapabilityJSRules(std::string in_capability_js_str,
                                    uint64_t& out_capability_bit_map,
                                    std::map<std::string, bool>& out_capability_js_wl);

    std::string ToJSStringFromCapabilityBitmap(uint64_t capability_bitmap);
    CapabilityDefinition* GetCapabilityJSStringRules();

    std::string GetCurrentTaskCapabilityAsJSString();

  private:
    // maintain the current task
    PendingTask* m_current_task_;
    // the capability map between the string and bitmap
    CapabilityDefinition* m_capability_definition;
};

}

}

#endif // SCRIPT_CHECKER_H
