#ifndef GLOBAL_H
#define GLOBAL_H

#include "base/scriptchecker/script_checker.h"

namespace base {

namespace scriptchecker {

const char BASE_EXPORT g_name[] = "[SCRIPTCHECKER] ";

extern ScriptChecker* BASE_EXPORT g_script_checker;

void BASE_EXPORT initScriptChecker();

} // scriptchecker
} // base


#endif  // GLOBAL_H
