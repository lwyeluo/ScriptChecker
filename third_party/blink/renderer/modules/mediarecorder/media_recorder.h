// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIARECORDER_MEDIA_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIARECORDER_MEDIA_RECORDER_H_

#include <memory>
#include "third_party/blink/public/platform/web_media_recorder_handler.h"
#include "third_party/blink/public/platform/web_media_recorder_handler_client.h"
#include "third_party/blink/renderer/bindings/core/v8/active_script_wrappable.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/dom/pausable_object.h"
#include "third_party/blink/renderer/modules/event_target_modules.h"
#include "third_party/blink/renderer/modules/mediarecorder/media_recorder_options.h"
#include "third_party/blink/renderer/modules/mediastream/media_stream.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/async_method_runner.h"

namespace blink {

class Blob;
class BlobData;
class ExceptionState;

class MODULES_EXPORT MediaRecorder final
    : public EventTargetWithInlineData,
      public WebMediaRecorderHandlerClient,
      public ActiveScriptWrappable<MediaRecorder>,
      public PausableObject {
  USING_GARBAGE_COLLECTED_MIXIN(MediaRecorder);
  DEFINE_WRAPPERTYPEINFO();

 public:
  enum class State { kInactive = 0, kRecording, kPaused };

  static MediaRecorder* Create(ExecutionContext*,
                               MediaStream*,
                               ExceptionState&);
  static MediaRecorder* Create(ExecutionContext*,
                               MediaStream*,
                               const MediaRecorderOptions&,
                               ExceptionState&);

  virtual ~MediaRecorder() = default;

  MediaStream* stream() const { return stream_.Get(); }
  const String& mimeType() const { return mime_type_; }
  String state() const;
  unsigned long videoBitsPerSecond() const { return video_bits_per_second_; }
  unsigned long audioBitsPerSecond() const { return audio_bits_per_second_; }

  DEFINE_ATTRIBUTE_EVENT_LISTENER(start);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(stop);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(dataavailable);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(pause);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(resume);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(error);

  void start(ExceptionState&);
  void start(int time_slice, ExceptionState&);
  void stop(ExceptionState&);
  void pause(ExceptionState&);
  void resume(ExceptionState&);
  void requestData(ExceptionState&);

  static bool isTypeSupported(ExecutionContext*, const String& type);

  // EventTarget
  const AtomicString& InterfaceName() const override;
  ExecutionContext* GetExecutionContext() const override;

  // PausableObject
  void Pause() override;
  void Unpause() override;
  void ContextDestroyed(ExecutionContext*) override;

  // ScriptWrappable
  bool HasPendingActivity() const final { return !stopped_; }

  // WebMediaRecorderHandlerClient
  void WriteData(const char* data,
                 size_t length,
                 bool last_in_slice,
                 double timecode) override;
  void OnError(const WebString& message) override;

  virtual void Trace(blink::Visitor*);

 private:
  MediaRecorder(ExecutionContext*,
                MediaStream*,
                const MediaRecorderOptions&,
                ExceptionState&);

  void CreateBlobEvent(Blob*, double);

  void StopRecording();
  void ScheduleDispatchEvent(Event*);
  void DispatchScheduledEvent();

  Member<MediaStream> stream_;
  String mime_type_;
  bool stopped_;
  int audio_bits_per_second_;
  int video_bits_per_second_;

  State state_;

  std::unique_ptr<BlobData> blob_data_;

  std::unique_ptr<WebMediaRecorderHandler> recorder_handler_;

  Member<AsyncMethodRunner<MediaRecorder>> dispatch_scheduled_event_runner_;
  HeapVector<Member<Event>> scheduled_events_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIARECORDER_MEDIA_RECORDER_H_
