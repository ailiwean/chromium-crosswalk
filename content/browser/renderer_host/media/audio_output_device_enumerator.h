// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// AudioOutputDeviceEnumerator is used to enumerate audio output devices.
// It can return cached results of previous enumerations in order to boost
// performance.
// All its public methods must be called on the thread where the object is
// created.

#ifndef CONTENT_BROWSER_RENDERER_HOST_MEDIA_AUDIO_OUTPUT_DEVICE_ENUMERATOR_H_
#define CONTENT_BROWSER_RENDERER_HOST_MEDIA_AUDIO_OUTPUT_DEVICE_ENUMERATOR_H_

#include <stdint.h>
#include <list>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "content/common/content_export.h"
#include "media/audio/audio_parameters.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace media {
class AudioManager;
}

namespace content {

// AudioOutputDeviceInfo describes information about an audio output device.
// The enumerations returned by AudioOutputDeviceEnumerator::Enumerate() contain
// elements of this type. It is used only in the browser side.
struct AudioOutputDeviceInfo {
  std::string unique_id;
  std::string device_name;
  media::AudioParameters output_params;
};

typedef std::vector<AudioOutputDeviceInfo> AudioOutputDeviceEnumeration;
typedef base::Callback<void(const AudioOutputDeviceEnumeration&)>
    AudioOutputDeviceEnumerationCB;

class CONTENT_EXPORT AudioOutputDeviceEnumerator {
 public:
  enum CachePolicy {
    CACHE_POLICY_NO_CACHING,
    CACHE_POLICY_MANUAL_INVALIDATION
  };
  AudioOutputDeviceEnumerator(media::AudioManager* audio_manager,
                              CachePolicy cache_policy);
  ~AudioOutputDeviceEnumerator();

  // Must be called on the IO thread. |callback| is also invoked
  // on the IO thread.
  void Enumerate(const AudioOutputDeviceEnumerationCB& callback);

  // Invalidates the current cache. Must be called on the IO thread.
  void InvalidateCache();

  // Sets the cache policy. Must be called on the IO thread.
  void SetCachePolicy(CachePolicy cache_policy);

 private:
  void InitializeOnIOThread();
  void DoEnumerateDevices();
  AudioOutputDeviceEnumeration DoEnumerateDevicesOnDeviceThread();
  void DevicesEnumerated(const AudioOutputDeviceEnumeration& snapshot);
  int64_t NewEventSequence();
  bool IsLastEnumerationValid() const;

  media::AudioManager* const audio_manager_;
  CachePolicy cache_policy_;
  AudioOutputDeviceEnumeration cache_;
  std::list<AudioOutputDeviceEnumerationCB> pending_callbacks_;

  // sequential number that serves as logical clock
  int64_t current_event_sequence_;

  int64_t seq_last_enumeration_;
  int64_t seq_last_invalidation_;
  bool is_enumeration_ongoing_;

  base::ThreadChecker thread_checker_;
  base::WeakPtrFactory<AudioOutputDeviceEnumerator> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AudioOutputDeviceEnumerator);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_MEDIA_AUDIO_OUTPUT_DEVICE_ENUMERATOR_H_
