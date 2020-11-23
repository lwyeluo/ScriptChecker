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
  is_restricted_ = false;
}

Capability::Capability(std::string js_str_capability) :
    capability_(js_str_capability) {
  LoadJSStringToCapability(js_str_capability);
}

Capability::Capability(Capability *capability) {
  SetFrom(capability);
  capability_ = "";
}

Capability::Capability(Capability&& capability) {
  SetFrom(&capability);
  capability_ = "";
}

Capability::~Capability() = default;

bool Capability::operator==(const Capability& other) const {
  return capability_bit_map_ == other.capability_bit_map_;
}

Capability& Capability::operator=(Capability&& other) {
  Capability* cap = new Capability();
  cap->SetFrom(&other);
  return *cap;
}

void Capability::LoadJSStringToCapability(std::string js_str_capability) {
  capability_ = js_str_capability;
  if(js_str_capability != "") {
    // TODO
    is_restricted_ = true;
    return;
  }
  is_restricted_ = false;
}

void Capability::LoadIPCStringToCapability(std::string ipc_str_capability) {
  if(ipc_str_capability != "") {
    // TODO
    is_restricted_ = true;
    return;
  }
  is_restricted_ = false;
}

void Capability::SetFrom(Capability *capability) {
  DCHECK(capability);
  is_restricted_ = capability->is_restricted_;
  if(is_restricted_) {
    capability_bit_map_ = capability->capability_bit_map_;
    capability_js_wl.clear();
    capability_js_wl.insert(capability->capability_js_wl.begin(),
                            capability->capability_js_wl.end());
  }
}

void Capability::SetFromIPCMessage(std::string capability_attached_in_ipc) {
  LoadIPCStringToCapability(capability_attached_in_ipc);
}

void Capability::SetFromJSString(std::string capability_attached_in_js_string) {
  LoadJSStringToCapability(capability_attached_in_js_string);
}

int Capability::GetBitsForCapability(int target_capability) {
  // TODO
  return true;
}

bool Capability::ContainsInJSWL(std::string target_object) {
  // TODO
  return true;
}

bool Capability::IsRestricted() {
  LOG(INFO) << g_name << "Capability::IsRestricted. " << is_restricted_;
  return is_restricted_;
}

std::string Capability::ToIPCString() {
  if(!is_restricted_)
    return "";
  if(capability_ != "")
    return capability_;
  // TODO
  return "TODO-IPC";
}

std::string Capability::ToJSString() {
  if(!is_restricted_)
    return "";
  if(capability_ != "")
    return capability_;
  // TODO
  return "TODO-JS";
}

std::string Capability::ToString() {
  return ToJSString();
}

}

}
