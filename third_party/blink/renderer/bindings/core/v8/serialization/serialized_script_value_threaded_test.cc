// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/bindings/core/v8/serialization/serialized_script_value.h"

#include "build/build_config.h"
#include "third_party/blink/renderer/bindings/core/v8/exception_state.h"
#include "third_party/blink/renderer/bindings/core/v8/serialization/unpacked_serialized_script_value.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_testing.h"
#include "third_party/blink/renderer/bindings/core/v8/worker_or_worklet_script_controller.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/typed_arrays/dom_array_buffer.h"
#include "third_party/blink/renderer/core/workers/worker_thread_test_helper.h"
#include "third_party/blink/renderer/platform/bindings/to_v8.h"

namespace blink {

// On debug builds, Oilpan contains checks that will fail if a persistent handle
// is destroyed on the wrong thread.
TEST(SerializedScriptValueThreadedTest,
     SafeDestructionIfSendingThreadKeepsAlive) {
  V8TestingScope scope;

  // Start a worker.
  WorkerReportingProxy proxy;
  WorkerThreadForTest worker_thread(nullptr, proxy);
  ParentFrameTaskRunners* parent_frame_task_runners =
      ParentFrameTaskRunners::Create(&scope.GetDocument());
  worker_thread.StartWithSourceCode(scope.GetDocument().GetSecurityOrigin(),
                                    "/* no worker script */",
                                    parent_frame_task_runners);

  // Create a serialized script value that contains transferred array buffer
  // contents.
  DOMArrayBuffer* array_buffer = DOMArrayBuffer::Create(1, 1);
  Transferables transferables;
  transferables.array_buffers.push_back(array_buffer);
  SerializedScriptValue::SerializeOptions options;
  options.transferables = &transferables;
  scoped_refptr<SerializedScriptValue> serialized =
      SerializedScriptValue::Serialize(
          scope.GetIsolate(),
          ToV8(array_buffer, scope.GetContext()->Global(), scope.GetIsolate()),
          options, ASSERT_NO_EXCEPTION);
  EXPECT_TRUE(serialized);
  EXPECT_TRUE(array_buffer->IsNeutered());

  // Deserialize the serialized value on the worker.
  // Intentionally keep a reference on this thread while this occurs.
  worker_thread.GetWorkerBackingThread().BackingThread().PostTask(
      FROM_HERE,
      CrossThreadBind(
          [](WorkerThread* worker_thread,
             scoped_refptr<SerializedScriptValue> serialized) {
            WorkerOrWorkletScriptController* script =
                worker_thread->GlobalScope()->ScriptController();
            EXPECT_TRUE(script->IsContextInitialized());
            ScriptState::Scope worker_scope(script->GetScriptState());
            SerializedScriptValue::Unpack(serialized)
                ->Deserialize(worker_thread->GetIsolate());

            // Make sure this thread's references in the Oilpan heap are dropped
            // before the main thread continues.
            ThreadState::Current()->CollectAllGarbage();
          },
          CrossThreadUnretained(&worker_thread), serialized));

  // Wait for a subsequent task on the worker to finish, to ensure that the
  // references held by the task are dropped.
  WaitableEvent done;
  worker_thread.GetWorkerBackingThread().BackingThread().PostTask(
      FROM_HERE,
      CrossThreadBind(&WaitableEvent::Signal, CrossThreadUnretained(&done)));
  done.Wait();

  // Now destroy the value on the main thread.
  EXPECT_TRUE(serialized->HasOneRef());
  serialized = nullptr;

  // Finally, shut down the worker thread.
  worker_thread.Terminate();
  worker_thread.WaitForShutdownForTesting();
}

}  // namespace blink
