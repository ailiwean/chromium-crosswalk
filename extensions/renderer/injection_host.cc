// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/injection_host.h"

InjectionHost::InjectionHost(const HostID& host_id) :
    id_(host_id) {
}

InjectionHost::~InjectionHost() {
}
