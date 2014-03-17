// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ui/native_app_window.h"
#include "chrome/browser/apps/app_browsertest_util.h"

using extensions::Extension;

namespace apps {

namespace {

typedef extensions::PlatformAppBrowserTest AppWindowBrowserTest;

#if defined(TOOLKIT_GTK)
#define MAYBE_FrameInsetsForDefaultFrame DISABLED_FrameInsetsForDefaultFrame
#else
#define MAYBE_FrameInsetsForDefaultFrame FrameInsetsForDefaultFrame
#endif

// Verifies that the NativeAppWindows implement GetFrameInsets() correctly.
// See crbug.com/346115
IN_PROC_BROWSER_TEST_F(AppWindowBrowserTest, MAYBE_FrameInsetsForDefaultFrame) {
  AppWindow* app_window = CreateTestAppWindow("{}");
  NativeAppWindow* native_window = app_window->GetBaseWindow();
  gfx::Insets insets = native_window->GetFrameInsets();

  // It is a reasonable assumption that the top padding must be greater than
  // the bottom padding due to the title bar.
  EXPECT_TRUE(insets.top() > insets.bottom());

  CloseAppWindow(app_window);
}

#if defined(TOOLKIT_GTK)
#define MAYBE_FrameInsetsForColoredFrame DISABLED_FrameInsetsForColoredFrame
#else
#define MAYBE_FrameInsetsForColoredFrame FrameInsetsForColoredFrame
#endif

// Verifies that the NativeAppWindows implement GetFrameInsets() correctly.
// See crbug.com/346115
IN_PROC_BROWSER_TEST_F(AppWindowBrowserTest, MAYBE_FrameInsetsForColoredFrame) {
  AppWindow* app_window =
      CreateTestAppWindow("{ \"frame\": { \"color\": \"#ffffff\" } }");
  NativeAppWindow* native_window = app_window->GetBaseWindow();
  gfx::Insets insets = native_window->GetFrameInsets();

  // It is a reasonable assumption that the top padding must be greater than
  // the bottom padding due to the title bar.
  EXPECT_TRUE(insets.top() > insets.bottom());

  CloseAppWindow(app_window);
}

// Verifies that the NativeAppWindows implement GetFrameInsets() correctly for
// frameless windows.
IN_PROC_BROWSER_TEST_F(AppWindowBrowserTest, FrameInsetsForNoFrame) {
  AppWindow* app_window = CreateTestAppWindow("{ \"frame\": \"none\" }");
  NativeAppWindow* native_window = app_window->GetBaseWindow();
  gfx::Insets insets = native_window->GetFrameInsets();

  // All insets must be zero.
  EXPECT_EQ(0, insets.top());
  EXPECT_EQ(0, insets.bottom());
  EXPECT_EQ(0, insets.left());
  EXPECT_EQ(0, insets.right());

  CloseAppWindow(app_window);
}

}  // namespace

}  // namespace apps
