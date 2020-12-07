/*
 * Add a capability for event listeners
 *  Author: Luo Wu
 */

#include "third_party/blink/renderer/core/events/registered_event_listener.h"

namespace blink {

base::scriptchecker::Capability* RegisteredEventListener::GetCapability() {
  return capability_;
}

void RegisteredEventListener::SetCapability(base::scriptchecker::Capability *capability) {
  if(!(capability && capability->IsRestricted()))
    return;
  if(!capability_) {
    capability_ = new base::scriptchecker::Capability();
    capability_->SetFrom(capability);
    return;
  }
  capability_->NarrowDownFrom(capability);
}

void RegisteredEventListener::ReleaseCapability() {
  if(capability_)
    delete capability_;
}

}
