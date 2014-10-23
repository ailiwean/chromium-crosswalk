// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/command_buffer/common/capabilities.h"

namespace gpu {

Capabilities::Capabilities()
    : post_sub_buffer(false),
      egl_image_external(false),
      texture_format_bgra8888(false),
      texture_format_etc1(false),
      texture_format_etc1_npot(false),
      texture_rectangle(false),
      iosurface(false),
      texture_usage(false),
      texture_storage(false),
      discard_framebuffer(false),
      sync_query(false),
      image(false),
      future_sync_points(false),
      blend_minmax(false),
      blend_equation_advanced(false),
      blend_equation_advanced_coherent(false) {
}

}  // namespace gpu
