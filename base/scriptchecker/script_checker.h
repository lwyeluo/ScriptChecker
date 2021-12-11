#ifndef SCRIPT_CHECKER_H
#define SCRIPT_CHECKER_H

#include <map>
#include <string>
#include "base/logging.h"
#include "base/scriptchecker/capability.h"
#include "base/scriptchecker/capability_definition.h"
#include "base/scriptchecker/async_exec_queue.h"
#include "base/pending_task.h"

// to capture logs for webpages, e.g., Aleax top 1000
#define SCRIPT_CHECKER_TEST_WEBPAGE

namespace base {

namespace scriptchecker {

class BASE_EXPORT ScriptChecker {
  public:
    ScriptChecker();
    ~ScriptChecker();

    /* Task Scheduler */
    void UpdateCurrentTask(PendingTask* task);
    void RunAsyncExecTasks();
    size_t GetAsyncExecTaskSize();

    /* Measure the time usage for schedule an async task for setTimeoutWR */
    struct TIME_MEASURE {
      timeval time_begin, time_end, time_diff;
      uint64_t cpu_begin, cpu_end;
      int task_id;
      bool is_valid;
    };
    void StartTimeMeasureForAsyncTask();
    void FinishTimeMeasureForAsyncTask();

    /* Task Recorder */
    void RecordNewTask(PendingTask* task);
    void RecordNewAsyncExecTask(PendingTask&& task);
    // <script xxx risky task_capability="xxx"> should be run in risky task
    void RecordRestrictedFrameParserTask(PendingTask&& task);
    // <script src="xxx" risky task_capability="xxx">: the ipc task should be risky
    void RecordRiskyScriptDownloadededFromNetwork(std::string capability_in_js);
    // the risky script is finished, so we need to launch new unrestricted
    //  task to parse remaining items
    void RecordNormalRestrictedFrameParserTask(PendingTask&& task);
    void FinishRestrictedFrameParserTask();
    //   The IPC task usually forms as two purposes, one is to trigger some listeners, and the
    //     other is to let renderer process parse some data, e.g., a script and execute it.
    //   The former's risky and capability should be attached on listener.
    //   The latter we now only attach risky and task_capability attributes when the SCRIPT
    //     tag is created. Need to consider more cases in this scenario later.
    //   So we now always set the parameter as "", see:
    //     ChannelAssociatedGroupController::AcceptOnProxyThread
    //     Connector::ReadSingleMessage
    void RecordIPCTask(std::string capability_attached_in_ipc_message);

#ifndef SCRIPT_CHECKER_SEPERATE_FRAME_PARSER
    // <script xxx risky task_capability=""> should be run in risky task.
    // currently we dynamic change task's capability to run risky script
    //   using new task later
    void UpdateCurrentTaskCapability(std::string task_capability);
    // listener with restricted capability should be run in risky task.
    // currently we dynamic change task's capability to run risky script
    //   using new task later
    void UpdateCurrentTaskCapability(Capability* capability);
#endif

    /* Information of Current Task */
    bool IsCurrentTaskWithRestricted();
    Capability* GetCurrentTaskCapability();
    std::string GetCurrentTaskCapabilityAsJSString();
    int GetCurrentTaskID();
    bool IsCurrentTaskHasRestrictedFrameParserTask();
    bool IsCurrentTaskNormalFrameParser();

    /* Security Monitor */
    bool DisallowedToAccessNetwork();
    bool DisallowedToAccessDOM(bool is_ele_has_task_cap_attr);
#ifdef SCRIPT_CHECKER_TEST_WEBPAGE
    bool DisallowedToAccessCookie();
#endif
    bool DisallowedToAccessJSObject(std::string object_name);

    // update capability information with predefined rules
    bool MatchWithCapabilityJSRules(std::string in_capability_js_str,
                                    uint64_t& out_capability_bit_map,
                                    std::map<std::string, bool>& out_capability_js_wl);

    std::string ToJSStringFromCapabilityBitmap(uint64_t capability_bitmap);
    CapabilityDefinition* GetCapabilityJSStringRules();

  private:
    // maintain the current task
    PendingTask* m_current_task_;
    // the capability map between the string and bitmap
    CapabilityDefinition* m_capability_definition;
    // maintain the tasks created by SCRIPTCHECKER's async execution
    AsyncExecQueue* m_async_exec_queue_;
    // whether the current task has restricted frame parser task
    bool m_has_restricted_frame_parser_task_;
    // to measure the time usage for an async task by setTimeoutWR
    std::map<int, struct TIME_MEASURE*> m_time_measure_map_;
};

}

}

#endif // SCRIPT_CHECKER_H
