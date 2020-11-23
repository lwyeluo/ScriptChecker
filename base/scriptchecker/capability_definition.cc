/*
 * The capability of ScriptCheck
 *  Author: Luo Wu
 */
#include "base/scriptchecker/capability_definition.h"
#include "base/scriptchecker/global.h"

namespace base {

namespace scriptchecker {

constexpr const char* CapabilityDefinition::cap_str_no_dom;
constexpr const char* CapabilityDefinition::cap_str_no_cookie;
constexpr const char* CapabilityDefinition::cap_str_no_network;
constexpr const char* CapabilityDefinition::cap_str_js_wl;

CapabilityDefinition::Rules::Rules() {
  LOG(INFO) << g_name << "Init the CapabilityDefinition::Rules::Rules";
  data = std::map<std::string, int>();
  data[CapabilityDefinition::cap_str_no_cookie]
          = CapabilityDefinition::cap_cookie_;
  data[CapabilityDefinition::cap_str_no_network]
          = CapabilityDefinition::cap_network_;
  data[CapabilityDefinition::cap_str_no_dom]
          = CapabilityDefinition::cap_dom_;
  data[CapabilityDefinition::cap_str_protective_dom]
          = CapabilityDefinition::cap_dom_;
  data[CapabilityDefinition::cap_str_js_wl]
          = CapabilityDefinition::cap_js_wl_;
}

CapabilityDefinition::Rules::~Rules() {};

}

}
