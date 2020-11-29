#ifndef CAPABILITY_DEFINITION_H
#define CAPABILITY_DEFINITION_H

#include <map>
#include <string>
#include "base/logging.h"

namespace base {

namespace scriptchecker {

class BASE_EXPORT CapabilityDefinition {
  public:
    static const int cap_dom_bit_offset = 0;
    static const int cap_cookie_bit_offset = 2;
    static const int cap_network_bit_offset = 3;

    static constexpr const char* cap_str_no_dom = "No_DOM_Access";
    static const uint64_t cap_no_dom = 0x3;
    static constexpr const char* cap_str_with_dom = "DOM_Access";
    static const uint64_t cap_with_dom = 0x0;
    static constexpr const char* cap_str_protective_dom = "DOM_Access_Protective";
    static const uint64_t cap_protective_dom = 0x1;

    static constexpr const char* cap_str_no_cookie = "No_Cookie_Access";
    static const uint64_t cap_no_cookie = 0x1;
    static constexpr const char* cap_str_with_cookie = "Cookie_Access";
    static const uint64_t cap_with_cookie = 0x0;

    static constexpr const char* cap_str_no_network = "No_Network_Access";
    static const uint64_t cap_no_network = 0x1;
    static constexpr const char* cap_str_with_network = "Network_Access";
    static const uint64_t cap_with_network = 0x0;

    static constexpr const char* cap_str_js_wl = "JS_WL";

    // a simple case of capability is:
    //    "No_Cookie_Access;No_DOM_Access;JS_WL:shareData,shareNum;"
    static const char cap_str_sep_symbol = ';';
    static const char cap_str_js_wl_key_value_sep_sysmbol = ':';
    static const char cap_str_js_wl_value_sep_sysmbol = ',';

    // string -> cap
    friend class Rules;
    struct Rules {
      std::map<std::string, uint64_t> string_to_bitmap_;
      std::map<uint64_t, std::string> bitmap_to_string_;
      std::map<uint64_t, std::string> bitmap_shadows_;

      Rules();
      ~Rules();

      bool Match(std::string in_capability_js_str, uint64_t& out_capability_bit_map,
                 std::map<std::string, bool>& out_capability_js_wl);
      std::string ToJSStringFromBitMap(uint64_t capability_bit_map);
    };

    Rules rule;

    // Security Monitor
    static bool DisallowedToAccessCookie(uint64_t capability_bit_map);
    static bool DisallowedToAccessNetwork(uint64_t capability_bit_map);
    static bool DisallowedToAccessDOM(uint64_t capability_bit_map,
                                      bool is_ele_has_task_cap_attr);
};

}

}

#endif // CAPABILITY_DEFINITION_H
