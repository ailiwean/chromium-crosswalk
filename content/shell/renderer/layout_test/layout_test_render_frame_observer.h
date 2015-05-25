// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_RENDERER_LAYOUT_TEST_LAYOUT_TEST_RENDER_FRAME_OBSERVER_H_
#define CONTENT_SHELL_RENDERER_LAYOUT_TEST_LAYOUT_TEST_RENDER_FRAME_OBSERVER_H_

#include "content/public/renderer/render_frame_observer.h"

namespace content {
class RenderFrame;

class LayoutTestRenderFrameObserver : public RenderFrameObserver {
 public:
  explicit LayoutTestRenderFrameObserver(RenderFrame* render_frame);
  ~LayoutTestRenderFrameObserver() override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(LayoutTestRenderFrameObserver);
};

}  // namespace content

#endif  // CONTENT_SHELL_RENDERER_LAYOUT_TEST_LAYOUT_TEST_RENDER_FRAME_OBSERVER_H_
