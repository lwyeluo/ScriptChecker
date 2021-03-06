// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_PRESENTATION_PRESENTATION_RECEIVER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_PRESENTATION_PRESENTATION_RECEIVER_H_

#include "mojo/public/cpp/bindings/binding.h"
#include "third_party/blink/public/platform/modules/presentation/presentation.mojom-blink.h"
#include "third_party/blink/public/platform/modules/presentation/web_presentation_receiver.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_property.h"
#include "third_party/blink/renderer/core/dom/context_lifecycle_observer.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/heap/heap.h"

namespace blink {

class Document;
class PresentationConnectionList;
class ReceiverPresentationConnection;
class WebPresentationClient;

// Implements the PresentationReceiver interface from the Presentation API from
// which websites can implement the receiving side of a presentation. This needs
// to be eagerly created in order to have the receiver associated with the
// client.
class MODULES_EXPORT PresentationReceiver final
    : public ScriptWrappable,
      public ContextLifecycleObserver,
      public WebPresentationReceiver,
      public mojom::blink::PresentationReceiver {
  USING_GARBAGE_COLLECTED_MIXIN(PresentationReceiver);
  DEFINE_WRAPPERTYPEINFO();
  using ConnectionListProperty =
      ScriptPromiseProperty<Member<PresentationReceiver>,
                            Member<PresentationConnectionList>,
                            Member<DOMException>>;

 public:
  PresentationReceiver(LocalFrame*, WebPresentationClient*);
  ~PresentationReceiver() = default;

  static PresentationReceiver* From(Document&);

  // PresentationReceiver.idl implementation
  ScriptPromise connectionList(ScriptState*);

  // WebPresentationReceiver implementation.
  // Initializes the PresentationReceiver Mojo binding and registers itself as
  // a receiver with PresentationService. No-ops if already initialized.
  void Init() override;
  void OnReceiverTerminated() override;

  // mojom::blink::PresentationReceiver
  void OnReceiverConnectionAvailable(
      mojom::blink::PresentationInfoPtr,
      mojom::blink::PresentationConnectionPtr,
      mojom::blink::PresentationConnectionRequest) override;

  void RegisterConnection(ReceiverPresentationConnection*);
  void RemoveConnection(ReceiverPresentationConnection*);
  void Terminate();

  virtual void Trace(blink::Visitor*);

 private:
  friend class PresentationReceiverTest;

  static void RecordOriginTypeAccess(ExecutionContext&);

  // ContextLifecycleObserver implementation.
  void ContextDestroyed(ExecutionContext*) override;

  Member<ConnectionListProperty> connection_list_property_;
  Member<PresentationConnectionList> connection_list_;

  mojo::Binding<mojom::blink::PresentationReceiver> receiver_binding_;
  WebPresentationClient* client_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_PRESENTATION_PRESENTATION_RECEIVER_H_
