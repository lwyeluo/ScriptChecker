/*
 * The capability of ScriptCheck
 *  Author: Luo Wu
 */
#include "base/scriptchecker/capability_definition.h"
#include "base/scriptchecker/global.h"

namespace base {

namespace scriptchecker {

constexpr const char* CapabilityDefinition::cap_str_no_dom;
constexpr const char* CapabilityDefinition::cap_str_with_dom;
constexpr const char* CapabilityDefinition::cap_str_protective_dom;
constexpr const char* CapabilityDefinition::cap_str_no_cookie;
constexpr const char* CapabilityDefinition::cap_str_with_cookie;
constexpr const char* CapabilityDefinition::cap_str_no_network;
constexpr const char* CapabilityDefinition::cap_str_with_network;
constexpr const char* CapabilityDefinition::cap_str_js_wl;
const uint64_t CapabilityDefinition::cap_no_dom;
const uint64_t CapabilityDefinition::cap_no_cookie;
const uint64_t CapabilityDefinition::cap_no_network;

CapabilityDefinition::Rules::Rules() {
  LOG(INFO) << g_name << "Init the CapabilityDefinition::Rules::Rules";

  // string_to_bitmap_
  string_to_bitmap_ = std::map<std::string, uint64_t>();
  string_to_bitmap_[cap_str_no_cookie] = cap_no_cookie << cap_cookie_bit_offset;
  string_to_bitmap_[cap_str_with_cookie] = cap_with_cookie << cap_cookie_bit_offset;

  string_to_bitmap_[cap_str_no_network] = cap_no_network << cap_network_bit_offset;
  string_to_bitmap_[cap_str_with_network] = cap_with_network << cap_network_bit_offset;

  string_to_bitmap_[cap_str_no_dom] = cap_no_dom << cap_dom_bit_offset;
  string_to_bitmap_[cap_str_with_dom] = cap_with_dom << cap_dom_bit_offset;
  string_to_bitmap_[cap_str_protective_dom] = cap_protective_dom << cap_dom_bit_offset;

  // bitmap_to_string_
  bitmap_to_string_ = std::map<uint64_t, std::string>();
  for(auto iter = string_to_bitmap_.begin(); iter != string_to_bitmap_.end(); iter ++) {
    if(iter->second == 0)
        continue;
    bitmap_to_string_[iter->second] = iter->first;
  }

  // bitmap_shadows_
  bitmap_shadows_ = std::map<uint64_t, std::string>();
  bitmap_shadows_[string_to_bitmap_[cap_str_no_cookie]] = cap_str_with_cookie;
  bitmap_shadows_[string_to_bitmap_[cap_str_no_network]] = cap_str_with_network;
  bitmap_shadows_[string_to_bitmap_[cap_str_no_dom]] = cap_str_with_dom;
}

CapabilityDefinition::Rules::~Rules() {};

bool CapabilityDefinition::Rules::Match(std::string in_capability_js_str,
                                        uint64_t& out_capability_bit_map,
                                        std::map<std::string, bool>& out_capability_js_wl) {
  out_capability_bit_map = 0;
  out_capability_js_wl.clear();

  LOG(INFO) << g_name << "CapabilityDefinition::Rules::Match for cap: " << in_capability_js_str;

  std::map<std::string, uint64_t>::iterator iter;
  size_t idx = in_capability_js_str.find(cap_str_sep_symbol);
  std::string permission = "", remain = in_capability_js_str;
  while(idx != std::string::npos) {
    // split permission
    permission = remain.substr(0, idx);
    remain = remain.substr(idx + 1);
    idx = remain.find(cap_str_sep_symbol);

    // for permissions except JS_WL
    iter = string_to_bitmap_.find(permission);
    if(iter != string_to_bitmap_.end()) {
      out_capability_bit_map |= iter->second;
      continue;
    }

    // for permission JS_WL
    if(permission.find(cap_str_js_wl) != std::string::npos) {
      std::string js_wl_values = permission.substr(strlen(cap_str_js_wl) + 1);

      // record JS_WL
      size_t js_wl_idx = 0;
      std::string v = "", r = js_wl_values;
      while(r.length() != 0) {
        js_wl_idx = r.find(cap_str_js_wl_value_sep_sysmbol);
        if(js_wl_idx != std::string::npos) {
          v = r.substr(0, js_wl_idx);
          r = r.substr(js_wl_idx + 1);
          out_capability_js_wl[v] = true;
        } else {
          // we have found all, so we return
          out_capability_js_wl[r] = true;
          return true;
        }
      }
      continue;
    }

    // the format is wrong
    LOG(INFO) << g_name << "[ERROR] bad format for JS capability: " << in_capability_js_str;
    out_capability_bit_map = 0;
    out_capability_js_wl.clear();
    return false;
  }
  return true;
}

std::string CapabilityDefinition::Rules::ToStringFromBitMap(
        uint64_t capability_bit_map) {
  std::string output = "";
  for(auto iter = bitmap_shadows_.begin();
      iter != bitmap_shadows_.end(); iter ++) {
    if(capability_bit_map & iter->first)
      output += bitmap_to_string_[capability_bit_map & iter->first];
    else
      output += iter->second;
    output += cap_str_sep_symbol;
  }
  return output;
}

}

}
