/*
 * The core of ScriptCheck to run restricted listener in a new child task
 *  Author: Luo Wu
 */

#include "third_party/blink/renderer/core/scriptchecker/restricted_frame_parser.h"

#include "base/scriptchecker/global.h"
#include "base/scriptchecker/capability.h"
#include "base/scriptchecker/task_type.h"

namespace blink {

bool RestrictedFrameParser::PostRestrictedTaskIfNecessary(
        HTMLParserScriptRunner* script_runner,
        Element* script_element,
        const TextPosition& script_start_position) {
  if(!base::scriptchecker::g_script_checker)
    return false;
  if(base::scriptchecker::g_script_checker->
          IsCurrentTaskHasRestrictedFrameParserTask()) {
    // the task has risky script, so run it
    return false;
  }

  base::scriptchecker::Capability* capability = nullptr;
  ScriptElementBase* element = ScriptElementBase::FromElementIfPossible(script_element);

  // check whether we need to create a new child task to run the pending risky script
  if(element && element->risky()) {
    std::string capability_str = element->CapabilityAttrbiuteValue().Utf8().data();
    if(capability_str != "") {
      capability = new base::scriptchecker::Capability(capability_str);
      // post new task
      base::PendingTask pending_task(
                  FROM_HERE, base::BindOnce(
                      &HTMLParserScriptRunner::ProcessScriptElement,
                      base::Unretained(script_runner),
                      script_element,
                      script_start_position),
                  TimeTicks(), base::Nestable::kNestable, capability,
                  base::scriptchecker::TaskType::RESTRICTED_FRAME_PARSER_TASK);
      pending_task.sequence_num = -1; // just to identify the task
      base::scriptchecker::g_script_checker->
              RecordRestrictedFrameParserTask(std::move(pending_task));
      return true;
    }
  }

  return false;
}

void RestrictedFrameParser::PostNormalTaskToContinueParsing(
        base::OnceClosure callback) {
  base::PendingTask pending_task(
              FROM_HERE, std::move(callback),
              TimeTicks(), base::Nestable::kNestable, nullptr,
              base::scriptchecker::TaskType::NORMAL_FRAME_PARSER_TASK);
  pending_task.sequence_num = -100; // just to identify the task
  base::scriptchecker::g_script_checker->
          RecordNormalRestrictedFrameParserTask(std::move(pending_task));
}

void RestrictedFrameParser::SetIPCTaskCapabilityIfNecessary(
        ScriptElementBase* element) {
  // the ipc task should be normal task
  if(base::scriptchecker::g_script_checker &&
          !base::scriptchecker::g_script_checker->IsCurrentTaskWithRestricted()) {
    // the downloaded script should be risky
    if(element && element->risky()) {
      std::string capability_str = element->CapabilityAttrbiuteValue().Utf8().data();
      // update capability
      base::scriptchecker::g_script_checker->
              RecordRiskyScriptDownloadededFromNetwork(capability_str);
    }
  }
}

}
