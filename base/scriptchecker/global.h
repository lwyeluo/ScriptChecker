#ifndef GLOBAL_H
#define GLOBAL_H

#include "base/scriptchecker/script_checker.h"
#include "base/scriptchecker/host_script_checker.h"

namespace base {

namespace scriptchecker {

const char BASE_EXPORT g_name[] = "[SCRIPTCHECKER] ";

// ScriptChecker running in the renderer process
extern ScriptChecker* BASE_EXPORT g_script_checker;
void BASE_EXPORT initScriptChecker();

// ScriptChecker running in the browser kernel process
extern HostScriptChecker* BASE_EXPORT g_host_script_checker;
void BASE_EXPORT initHostScriptChecker();


} // scriptchecker
} // base


#endif  // GLOBAL_H
