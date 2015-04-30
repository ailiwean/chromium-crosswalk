// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_LARGE_ICON_BRIDGE_H_
#define CHROME_BROWSER_ANDROID_LARGE_ICON_BRIDGE_H_

#include <jni.h>

#include "base/memory/scoped_ptr.h"
#include "base/task/cancelable_task_tracker.h"

// The C++ counterpart to Java's LargeIconBridge. Together these classes expose
// LargeIconService to Java.
class LargeIconBridge {
 public:
  LargeIconBridge();
  void Destroy(JNIEnv* env, jobject obj);
  jboolean GetLargeIconForURL(JNIEnv* env,
                              jobject obj,
                              jobject j_profile,
                              jstring j_page_url,
                              jint desired_size_px,
                              jobject j_callback);
  static bool RegisterLargeIconBridge(JNIEnv* env);

 private:
  virtual ~LargeIconBridge();

  base::CancelableTaskTracker cancelable_task_tracker_;

  DISALLOW_COPY_AND_ASSIGN(LargeIconBridge);
};

#endif  // CHROME_BROWSER_ANDROID_LARGE_ICON_BRIDGE_H_
