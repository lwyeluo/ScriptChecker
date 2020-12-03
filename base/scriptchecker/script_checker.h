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

    /* Task Scheduler */
    void UpdateCurrentTask(PendingTask* task);

    /* Task Recorder */
    void RecordNewTask(PendingTask* task);
    //   The IPC task usually forms as two purposes, one is to trigger some listeners, and the
    //     other is to let renderer process parse some data, e.g., a script and execute it.
    //   The former's risky and capability should be attached on listener.
    //   The latter we now only attach risky and task_capability attributes when the SCRIPT
    //     tag is created. Need to consider more cases in this scenario later.
    //   So we now always set the parameter as "", see:
    //     ChannelAssociatedGroupController::AcceptOnProxyThread
    //     Connector::ReadSingleMessage
    void RecordIPCTask(std::string capability_attached_in_ipc_message);
    void RecordTIMERTask(std::string capability_from_js_string, bool is_restricted);

    bool IsCurrentTaskWithRestricted();
    Capability* GetCurrentTaskCapability();
    int GetCurrentTaskID();

    // <script xxx risky task_capability=""> should be run in risky task
    // currently we dynamic change task's capability to run risky script
    //   using new task later
    void UpdateCurrentTaskCapability(std::string task_capability);

    // Security Monitor
    bool DisallowedToAccessNetwork();
    bool DisallowedToAccessDOM(bool is_ele_has_task_cap_attr);
    bool DisallowedToAccessJSObject(std::string object_name);

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
