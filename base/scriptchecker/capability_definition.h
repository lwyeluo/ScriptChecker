#ifndef CAPABILITY_DEFINITION_H
#define CAPABILITY_DEFINITION_H

#include <map>
#include <string>
#include "base/logging.h"

namespace base {

namespace scriptchecker {

class BASE_EXPORT CapabilityDefinition {
  public:
    // 00: No_DOM_Access, 01: DOM_Access_Protective, 11: DOM_Access
    static const int cap_dom_ = 0x3;
    // 0: No_Cookie_Access, 1: Cookie_Access
    static const int cap_cookie_ = 0x1 << 2;
    // 0: No_Network_Access, 1: Network_Access
    static const int cap_network_ = 0x1 << 3;
    // 0: No_JS_WL, 1: JS_WL
    static const int cap_js_wl_ = 0x1 << 4;

    static constexpr const char* cap_str_no_dom = "No_DOM_Access";
    static constexpr const char* cap_str_protective_dom = "DOM_Access_Protective";
    static constexpr const char* cap_str_no_cookie = "No_Cookie_Access";
    static constexpr const char* cap_str_no_network = "No_network_Access";
    static constexpr const char* cap_str_js_wl = "JS_WL";

    // string -> cap
    friend class Rules;
    struct Rules {
      std::map<std::string, int> data;
      Rules();
      ~Rules();
    };

    Rules rule;
};

}

}

#endif // CAPABILITY_DEFINITION_H
