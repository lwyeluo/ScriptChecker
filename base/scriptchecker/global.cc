/*
 * The global variables of ScriptCheck
 *  Author: Luo Wu
 */
#include "base/scriptchecker/global.h"
#include "base/scriptchecker/capability.h"

namespace base {

namespace scriptchecker {

ScriptChecker* g_script_checker;

void initScriptChecker() {
  if (!g_script_checker)
    g_script_checker = new ScriptChecker();
  LOG(INFO) << g_name << "init script checker...";
}

}

}
