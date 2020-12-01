/*
 * The capability of ScriptCheck
 *  Author: Luo Wu
 */
#include "base/scriptchecker/capability.h"
#include "base/scriptchecker/global.h"

namespace base {

namespace scriptchecker {

Capability::Capability() {
  capability_ = "";
  capability_ipc_ = "";
  is_restricted_ = false;
}

Capability::Capability(std::string js_str_capability) {
  LoadJSStringToCapability(js_str_capability);
}

Capability::Capability(Capability *capability) {
  SetFrom(capability);
  capability_ = "";
  capability_ipc_ = "";
}

Capability::Capability(Capability&& capability) = default;

Capability::~Capability() = default;

Capability& Capability::operator=(Capability&& other) = default;

bool Capability::operator==(const Capability& other) const {
  return capability_bit_map_ == other.capability_bit_map_;
}

void Capability::LoadJSStringToCapability(std::string js_str_capability) {
  capability_ipc_ = "";
  if(js_str_capability != "") {
    bool is_success = g_script_checker->MatchWithCapabilityJSRules(
                js_str_capability, capability_bit_map_, capability_js_wl);

    if(is_success) {
      capability_ = js_str_capability;
      is_restricted_ = true;
      return;
    }
  }
  capability_ = "";
  is_restricted_ = false;
}

void Capability::LoadIPCStringToCapability(std::string ipc_str_capability) {
  capability_ = "";
  if(ipc_str_capability != "") {
    capability_bit_map_ = strtoul(ipc_str_capability.c_str(), NULL, 10);
    capability_js_wl.clear();
    is_restricted_ = true;
    return;
  }
  capability_ipc_ = "";
  is_restricted_ = false;
}

void Capability::SetFrom(Capability *capability) {
  DCHECK(capability);
  is_restricted_ = capability->is_restricted_;
  if(is_restricted_) {
    capability_bit_map_ = capability->capability_bit_map_;
    if(capability->capability_js_wl.size() > 0) {
      capability_js_wl.clear();
      capability_js_wl.insert(capability->capability_js_wl.begin(),
                              capability->capability_js_wl.end());
    }
  }
  capability_ = capability->capability_;
  capability_ipc_ = capability->capability_ipc_;
}

void Capability::NarrowDownFrom(Capability* capability) {
  if(!capability)
    return;
  if(!is_restricted_) {
    SetFrom(capability);
    return;
  }

  capability_bit_map_ |= capability->capability_bit_map_;
  for(auto iter = capability_js_wl.begin(); iter != capability_js_wl.end(); iter ++) {
    if(!capability->capability_js_wl.count(iter->first))
      capability_js_wl.erase(iter);
  }
  capability_ = "";
  capability_ipc_ = "";
}

void Capability::SetFromIPCMessage(std::string capability_attached_in_ipc) {
  LoadIPCStringToCapability(capability_attached_in_ipc);
}

void Capability::SetFromJSString(std::string capability_attached_in_js_string) {
  LoadJSStringToCapability(capability_attached_in_js_string);
}

uint64_t Capability::GetBitmap() {
  return capability_bit_map_;
}

bool Capability::ContainsInJSWL(std::string target_object) {
  return capability_js_wl.count(target_object);
}

bool Capability::IsRestricted() {
  return is_restricted_;
}

std::string Capability::ToIPCString() {
  if(!is_restricted_)
    return "";
  if(capability_ipc_ != "")
    return capability_ipc_;

  // currently we only need to attach the bitmap into IPC message
  return std::to_string(capability_bit_map_);
}

std::string Capability::ToJSString() {
  if(!is_restricted_)
    return "";
  if(capability_ != "")
    return capability_;

  std::string output = g_script_checker->
          ToJSStringFromCapabilityBitmap(capability_bit_map_);
  if(capability_js_wl.size() > 0)
    output = output + CapabilityDefinition::cap_str_js_wl
            + CapabilityDefinition::cap_str_js_wl_key_value_sep_sysmbol;
  for(auto iter = capability_js_wl.begin();
      iter != capability_js_wl.end(); iter ++) {
    output += iter->first;
    output += CapabilityDefinition::cap_str_js_wl_value_sep_sysmbol;
  }
  if(capability_js_wl.size() > 0)
    output[output.length() - 1] = CapabilityDefinition::cap_str_sep_symbol;
  capability_ = output;

  return output;
}

std::string Capability::ToString() {
  return ToJSString();
}

}

}
