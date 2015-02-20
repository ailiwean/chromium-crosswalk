// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_PEPPER_PEPPER_PLATFORM_VIDEO_CAPTURE_H_
#define CONTENT_RENDERER_PEPPER_PEPPER_PLATFORM_VIDEO_CAPTURE_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "content/common/media/video_capture.h"
#include "media/video/capture/video_capture_types.h"

class GURL;

namespace content {
class PepperMediaDeviceManager;
class PepperImageCaptureHost;

// This object must only be used on the thread it's constructed on.
class PepperPlatformImageCapture {
 public:
  PepperPlatformImageCapture(int render_frame_id,
                             const std::string& device_id,
                             const GURL& document_url,
                             PepperImageCaptureHost* handler);
  ~PepperPlatformImageCapture();

  // Detaches the event handler and stops sending notifications to it.
  void DetachEventHandler();

  void GetSupportedVideoCaptureFormats();

 private:
  void OnDeviceOpened(int request_id, bool succeeded, const std::string& label);

  // Called by VideoCaptureImplManager.
  void OnDeviceSupportedFormatsEnumerated(
      const media::VideoCaptureFormats& formats);

  // Can return NULL if the RenderFrame referenced by |render_frame_id_| has
  // gone away.
  PepperMediaDeviceManager* GetMediaDeviceManager();

  const int render_frame_id_;
  const std::string device_id_;

  std::string label_;
  int session_id_;
  base::Closure release_device_cb_;

  PepperImageCaptureHost* handler_;

  // Whether we have a pending request to open a device. We have to make sure
  // there isn't any pending request before this object goes away.
  bool pending_open_device_;
  int pending_open_device_id_;

  base::ThreadChecker thread_checker_;

  base::WeakPtrFactory<PepperPlatformImageCapture> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PepperPlatformImageCapture);
};

}  // namespace content

#endif  // CONTENT_RENDERER_PEPPER_PEPPER_PLATFORM_VIDEO_CAPTURE_H_
