#ifndef CAPABILITY_H
#define CAPABILITY_H

#include <map>
#include <string>
#include "base/logging.h"

namespace base {

namespace scriptchecker {

class BASE_EXPORT Capability {
  public:
    Capability();
    Capability(std::string js_str_capability);
    Capability(Capability* capability);
    Capability(Capability&& capability);
    ~Capability();

    bool operator==(const Capability& other) const;
    Capability& operator=(Capability&& other);

    void SetFrom(Capability* capability);
    void SetFromIPCMessage(std::string capability_atttached_in_ipc);
    void SetFromJSString(std::string capability_atttached_in_js_str);
    // load JS's specified capalitity
    void LoadJSStringToCapability(std::string js_str_capability);
    // load IPC message's specified capalitity
    void LoadIPCStringToCapability(std::string ipc_str_capability);

    // return the bits in the bit map towards a target capability
    //   @target_capability: refers to capability_definition.h,
    //      e.g., CapabilityDefinition::cap_cookie_
    int GetBitsForCapability(int target_capability);

    // whether the JS_WL contains the target object
    bool ContainsInJSWL(std::string target_object);

    bool IsRestricted();

    // used to attached the capablity to the IPC messages
    std::string ToIPCString();
    // used to show a readable capabilty string, the same as specified by setTimeoutWR
    std::string ToJSString();
    // for debug, it's the same as ToJSString()
    std::string ToString();

  private:
    std::string capability_;
    uint64_t capability_bit_map_;
    // maintain the WL for JS Access
    std::map<std::string, bool> capability_js_wl;
    // whether the capability is restricted
    bool is_restricted_;
};

}

}

#endif // CAPABILITY_H
