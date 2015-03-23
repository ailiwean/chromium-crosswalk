// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_METRICS_VARIATIONS_SESSION_H_
#define CHROME_BROWSER_ANDROID_METRICS_VARIATIONS_SESSION_H_

#include <jni.h>

namespace chrome {
namespace android {

// Registers the native methods through jni.
bool RegisterVariationsSession(JNIEnv* env);

}  // namespace android
}  // namespace chrome

#endif  // CHROME_BROWSER_ANDROID_METRICS_VARIATIONS_SESSION_H_
