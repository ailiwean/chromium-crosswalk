// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/app_window/native_app_window.h"

#import <Cocoa/Cocoa.h>

#import "base/mac/foundation_util.h"
#import "base/mac/mac_util.h"
#import "base/mac/scoped_nsobject.h"
#import "base/mac/sdk_forward_declarations.h"
#include "chrome/browser/apps/app_browsertest_util.h"
#include "chrome/browser/apps/app_shim/extension_app_shim_handler_mac.h"
#include "chrome/browser/apps/app_shim/test/app_shim_host_manager_test_api_mac.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/extensions/app_launch_params.h"
#include "chrome/browser/ui/extensions/application_launch.h"
#import "chrome/browser/ui/test/scoped_fake_nswindow_main_status.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/test_utils.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/common/constants.h"
#include "testing/gmock/include/gmock/gmock.h"
#import "testing/gtest_mac.h"
#import "ui/base/test/nswindow_fullscreen_notification_waiter.h"
#import "ui/base/test/scoped_fake_nswindow_fullscreen.h"
#import "ui/base/test/windowed_nsnotification_observer.h"
#import "ui/gfx/mac/nswindow_frame_controls.h"

using extensions::AppWindow;
using extensions::PlatformAppBrowserTest;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace {

// The param selects whether to use ChromeNativeAppWindowViewsMac, otherwise it
// will use NativeAppWindowCocoa.
class NativeAppWindowCocoaBrowserTest
    : public testing::WithParamInterface<bool>,
      public PlatformAppBrowserTest {
 protected:
  NativeAppWindowCocoaBrowserTest() {}

  void SetUpCommandLine(base::CommandLine* command_line) override {
    PlatformAppBrowserTest::SetUpCommandLine(command_line);
    command_line->AppendSwitch(
        GetParam() ? switches::kEnableMacViewsNativeAppWindows
                   : switches::kDisableMacViewsNativeAppWindows);
  }

  void SetUpAppWithWindows(int num_windows) {
    app_ = InstallExtension(
        test_data_dir_.AppendASCII("platform_apps").AppendASCII("minimal"), 1);
    EXPECT_TRUE(app_);

    for (int i = 0; i < num_windows; ++i) {
      content::WindowedNotificationObserver app_loaded_observer(
          content::NOTIFICATION_LOAD_COMPLETED_MAIN_FRAME,
          content::NotificationService::AllSources());
      OpenApplication(
          AppLaunchParams(profile(), app_, extensions::LAUNCH_CONTAINER_NONE,
                          NEW_WINDOW, extensions::SOURCE_TEST));
      app_loaded_observer.Wait();
    }
  }

  const extensions::Extension* app_;

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeAppWindowCocoaBrowserTest);
};

}  // namespace

// Test interaction of Hide/Show() with Hide/ShowWithApp().
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, HideShowWithApp) {
  SetUpAppWithWindows(2);
  extensions::AppWindowRegistry::AppWindowList windows =
      extensions::AppWindowRegistry::Get(profile())->app_windows();

  AppWindow* app_window = windows.front();
  extensions::NativeAppWindow* native_window = app_window->GetBaseWindow();
  NSWindow* ns_window = native_window->GetNativeWindow();

  AppWindow* other_app_window = windows.back();
  extensions::NativeAppWindow* other_native_window =
      other_app_window->GetBaseWindow();
  NSWindow* other_ns_window = other_native_window->GetNativeWindow();

  // Normal Hide/Show.
  app_window->Hide();
  EXPECT_FALSE([ns_window isVisible]);
  app_window->Show(AppWindow::SHOW_ACTIVE);
  EXPECT_TRUE([ns_window isVisible]);

  // Normal Hide/ShowWithApp.
  native_window->HideWithApp();
  EXPECT_FALSE([ns_window isVisible]);
  native_window->ShowWithApp();
  EXPECT_TRUE([ns_window isVisible]);

  // HideWithApp, Hide, ShowWithApp does not show.
  native_window->HideWithApp();
  app_window->Hide();
  native_window->ShowWithApp();
  EXPECT_FALSE([ns_window isVisible]);

  // Hide, HideWithApp, ShowWithApp does not show.
  native_window->HideWithApp();
  native_window->ShowWithApp();
  EXPECT_FALSE([ns_window isVisible]);

  // Return to shown state.
  app_window->Show(AppWindow::SHOW_ACTIVE);
  EXPECT_TRUE([ns_window isVisible]);

  // HideWithApp the other window.
  EXPECT_TRUE([other_ns_window isVisible]);
  other_native_window->HideWithApp();
  EXPECT_FALSE([other_ns_window isVisible]);

  // HideWithApp, Show shows just one window since there's no shim.
  native_window->HideWithApp();
  EXPECT_FALSE([ns_window isVisible]);
  app_window->Show(AppWindow::SHOW_ACTIVE);
  EXPECT_TRUE([ns_window isVisible]);
  EXPECT_FALSE([other_ns_window isVisible]);

  // Hide the other window.
  other_app_window->Hide();
  EXPECT_FALSE([other_ns_window isVisible]);

  // HideWithApp, ShowWithApp does not show the other window.
  native_window->HideWithApp();
  EXPECT_FALSE([ns_window isVisible]);
  native_window->ShowWithApp();
  EXPECT_TRUE([ns_window isVisible]);
  EXPECT_FALSE([other_ns_window isVisible]);
}

namespace {

class MockAppShimHost : public apps::AppShimHandler::Host {
 public:
  MockAppShimHost() {}
  ~MockAppShimHost() override {}

  MOCK_METHOD1(OnAppLaunchComplete, void(apps::AppShimLaunchResult));
  MOCK_METHOD0(OnAppClosed, void());
  MOCK_METHOD0(OnAppHide, void());
  MOCK_METHOD0(OnAppUnhideWithoutActivation, void());
  MOCK_METHOD1(OnAppRequestUserAttention, void(apps::AppShimAttentionType));
  MOCK_CONST_METHOD0(GetProfilePath, base::FilePath());
  MOCK_CONST_METHOD0(GetAppId, std::string());
};

class MockExtensionAppShimHandler : public apps::ExtensionAppShimHandler {
 public:
  MockExtensionAppShimHandler() {
    ON_CALL(*this, FindHost(_, _))
        .WillByDefault(Invoke(this, &apps::ExtensionAppShimHandler::FindHost));
  }
  ~MockExtensionAppShimHandler() override {}

  MOCK_METHOD2(FindHost, AppShimHandler::Host*(Profile*, const std::string&));
};

}  // namespace

// Test Hide/Show and Hide/ShowWithApp() behavior when shims are enabled.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest,
                       HideShowWithAppWithShim) {
  test::AppShimHostManagerTestApi test_api(
      g_browser_process->platform_part()->app_shim_host_manager());
  MockExtensionAppShimHandler* mock = new MockExtensionAppShimHandler();
  test_api.SetExtensionAppShimHandler(
      scoped_ptr<apps::ExtensionAppShimHandler>(mock));  // Takes ownership.
  MockAppShimHost mock_host;

  SetUpAppWithWindows(1);
  extensions::AppWindowRegistry::AppWindowList windows =
      extensions::AppWindowRegistry::Get(profile())->app_windows();

  extensions::AppWindow* app_window = windows.front();
  extensions::NativeAppWindow* native_window = app_window->GetBaseWindow();
  NSWindow* ns_window = native_window->GetNativeWindow();

  // HideWithApp.
  native_window->HideWithApp();
  EXPECT_FALSE([ns_window isVisible]);

  // Show notifies the shim to unhide.
  EXPECT_CALL(mock_host, OnAppUnhideWithoutActivation());
  EXPECT_CALL(*mock, FindHost(_, _)).WillOnce(Return(&mock_host));
  app_window->Show(extensions::AppWindow::SHOW_ACTIVE);
  EXPECT_TRUE([ns_window isVisible]);
  testing::Mock::VerifyAndClearExpectations(mock);
  testing::Mock::VerifyAndClearExpectations(&mock_host);

  // HideWithApp
  native_window->HideWithApp();
  EXPECT_FALSE([ns_window isVisible]);

  // Activate does the same.
  EXPECT_CALL(mock_host, OnAppUnhideWithoutActivation());
  EXPECT_CALL(*mock, FindHost(_, _)).WillOnce(Return(&mock_host));
  native_window->Activate();
  EXPECT_TRUE([ns_window isVisible]);
  testing::Mock::VerifyAndClearExpectations(mock);
  testing::Mock::VerifyAndClearExpectations(&mock_host);
}

// Test that NativeAppWindow and AppWindow fullscreen state is updated when
// the window is fullscreened natively.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, Fullscreen) {
  if (!base::mac::IsOSLionOrLater())
    return;

  ui::test::ScopedFakeNSWindowFullscreen fake_fullscreen;

  extensions::AppWindow* app_window =
      CreateTestAppWindow("{\"alwaysOnTop\": true }");
  extensions::NativeAppWindow* window = app_window->GetBaseWindow();
  NSWindow* ns_window = app_window->GetNativeWindow();
  base::scoped_nsobject<NSWindowFullscreenNotificationWaiter> waiter(
      [[NSWindowFullscreenNotificationWaiter alloc] initWithWindow:ns_window]);

  EXPECT_EQ(AppWindow::FULLSCREEN_TYPE_NONE,
            app_window->fullscreen_types_for_test());
  EXPECT_FALSE(window->IsFullscreen());
  EXPECT_FALSE([ns_window styleMask] & NSFullScreenWindowMask);
  EXPECT_TRUE(gfx::IsNSWindowAlwaysOnTop(ns_window));

  [ns_window toggleFullScreen:nil];
  [waiter waitForEnterCount:1 exitCount:0];
  EXPECT_TRUE(app_window->fullscreen_types_for_test() &
              AppWindow::FULLSCREEN_TYPE_OS);
  EXPECT_TRUE(window->IsFullscreen());
  EXPECT_TRUE([ns_window styleMask] & NSFullScreenWindowMask);
  EXPECT_FALSE(gfx::IsNSWindowAlwaysOnTop(ns_window));

  app_window->Restore();
  EXPECT_FALSE(window->IsFullscreenOrPending());
  [waiter waitForEnterCount:1 exitCount:1];
  EXPECT_EQ(AppWindow::FULLSCREEN_TYPE_NONE,
            app_window->fullscreen_types_for_test());
  EXPECT_FALSE(window->IsFullscreen());
  EXPECT_FALSE([ns_window styleMask] & NSFullScreenWindowMask);
  EXPECT_TRUE(gfx::IsNSWindowAlwaysOnTop(ns_window));

  app_window->Fullscreen();
  EXPECT_TRUE(window->IsFullscreenOrPending());
  [waiter waitForEnterCount:2 exitCount:1];
  EXPECT_TRUE(app_window->fullscreen_types_for_test() &
              AppWindow::FULLSCREEN_TYPE_WINDOW_API);
  EXPECT_TRUE(window->IsFullscreen());
  EXPECT_TRUE([ns_window styleMask] & NSFullScreenWindowMask);
  EXPECT_FALSE(gfx::IsNSWindowAlwaysOnTop(ns_window));

  [ns_window toggleFullScreen:nil];
  [waiter waitForEnterCount:2 exitCount:2];
  EXPECT_EQ(AppWindow::FULLSCREEN_TYPE_NONE,
            app_window->fullscreen_types_for_test());
  EXPECT_FALSE(window->IsFullscreen());
  EXPECT_FALSE([ns_window styleMask] & NSFullScreenWindowMask);
  EXPECT_TRUE(gfx::IsNSWindowAlwaysOnTop(ns_window));
}

// Test Minimize, Restore combinations with their native equivalents.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, Minimize) {
  SetUpAppWithWindows(1);
  AppWindow* app_window = GetFirstAppWindow();
  extensions::NativeAppWindow* window = app_window->GetBaseWindow();
  NSWindow* ns_window = app_window->GetNativeWindow();

  NSRect initial_frame = [ns_window frame];

  EXPECT_FALSE(window->IsMinimized());
  EXPECT_FALSE([ns_window isMiniaturized]);

  // Native minimize, Restore.
  [ns_window miniaturize:nil];
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMinimized());
  EXPECT_TRUE([ns_window isMiniaturized]);

  app_window->Restore();
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMinimized());
  EXPECT_FALSE([ns_window isMiniaturized]);

  // Minimize, native restore.
  app_window->Minimize();
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMinimized());
  EXPECT_TRUE([ns_window isMiniaturized]);

  [ns_window deminiaturize:nil];
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMinimized());
  EXPECT_FALSE([ns_window isMiniaturized]);
}

// Test Maximize, Restore combinations with their native equivalents.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, Maximize) {
  // This test is flaky on 10.6. Disable it until we're sure we need MacViews on
  // 10.6. See http://crbug.com/503208
  if (GetParam() && base::mac::IsOSSnowLeopard())
    return;

  SetUpAppWithWindows(1);
  AppWindow* app_window = GetFirstAppWindow();
  extensions::NativeAppWindow* window = app_window->GetBaseWindow();
  NSWindow* ns_window = app_window->GetNativeWindow();
  base::scoped_nsobject<WindowedNSNotificationObserver> watcher;

  gfx::Rect initial_restored_bounds = window->GetRestoredBounds();
  NSRect initial_frame = [ns_window frame];
  NSRect maximized_frame = [[ns_window screen] visibleFrame];

  EXPECT_FALSE(window->IsMaximized());

  // Native maximize, Restore.
  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  [ns_window zoom:nil];
  [watcher wait];
  EXPECT_EQ(initial_restored_bounds, window->GetRestoredBounds());
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());

  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  app_window->Restore();
  [watcher wait];
  EXPECT_EQ(initial_restored_bounds, window->GetRestoredBounds());
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());

  // Maximize, native restore.
  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  app_window->Maximize();
  [watcher wait];
  EXPECT_EQ(initial_restored_bounds, window->GetRestoredBounds());
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());

  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  [ns_window zoom:nil];
  [watcher wait];
  EXPECT_EQ(initial_restored_bounds, window->GetRestoredBounds());
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());
}

// Test Maximize when the window has a maximum size. The maximum size means that
// the window is not user-maximizable. However, calling Maximize() via the
// javascript API should still maximize and since the zoom button is removed,
// the codepath changes.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, MaximizeConstrained) {
  AppWindow* app_window = CreateTestAppWindow(
      "{\"outerBounds\": {\"maxWidth\":200, \"maxHeight\":300}}");
  extensions::NativeAppWindow* window = app_window->GetBaseWindow();
  NSWindow* ns_window = app_window->GetNativeWindow();
  base::scoped_nsobject<WindowedNSNotificationObserver> watcher;

  gfx::Rect initial_restored_bounds = window->GetRestoredBounds();
  NSRect initial_frame = [ns_window frame];
  NSRect maximized_frame = [[ns_window screen] visibleFrame];

  EXPECT_FALSE(window->IsMaximized());

  // Maximize, Restore.
  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  app_window->Maximize();
  [watcher wait];
  EXPECT_EQ(initial_restored_bounds, window->GetRestoredBounds());
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());

  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  app_window->Restore();
  [watcher wait];
  EXPECT_EQ(initial_restored_bounds, window->GetRestoredBounds());
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());
}

// Test Minimize, Maximize, Restore combinations with their native equivalents.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, MinimizeMaximize) {
  SetUpAppWithWindows(1);
  AppWindow* app_window = GetFirstAppWindow();
  extensions::NativeAppWindow* window = app_window->GetBaseWindow();
  NSWindow* ns_window = app_window->GetNativeWindow();
  base::scoped_nsobject<WindowedNSNotificationObserver> watcher;

  NSRect initial_frame = [ns_window frame];
  NSRect maximized_frame = [[ns_window screen] visibleFrame];

  EXPECT_FALSE(window->IsMaximized());
  EXPECT_FALSE(window->IsMinimized());
  EXPECT_FALSE([ns_window isMiniaturized]);

  // Maximize, Minimize, Restore.
  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  app_window->Maximize();
  [watcher wait];
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());

  app_window->Minimize();
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());
  EXPECT_TRUE(window->IsMinimized());
  EXPECT_TRUE([ns_window isMiniaturized]);

  app_window->Restore();
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());
  EXPECT_FALSE(window->IsMinimized());
  EXPECT_FALSE([ns_window isMiniaturized]);

  // Minimize, Maximize.
  app_window->Minimize();
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMinimized());
  EXPECT_TRUE([ns_window isMiniaturized]);

  app_window->Maximize();
  EXPECT_TRUE([ns_window isVisible]);
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());
  EXPECT_FALSE(window->IsMinimized());
  EXPECT_FALSE([ns_window isMiniaturized]);
}

// Test Maximize, Fullscreen, Restore combinations.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, MaximizeFullscreen) {
  if (base::mac::IsOSSnowLeopard())
    return;

  ui::test::ScopedFakeNSWindowFullscreen fake_fullscreen;

  SetUpAppWithWindows(1);
  AppWindow* app_window = GetFirstAppWindow();
  extensions::NativeAppWindow* window = app_window->GetBaseWindow();
  NSWindow* ns_window = app_window->GetNativeWindow();
  base::scoped_nsobject<WindowedNSNotificationObserver> watcher;
  base::scoped_nsobject<NSWindowFullscreenNotificationWaiter> waiter(
      [[NSWindowFullscreenNotificationWaiter alloc] initWithWindow:ns_window]);

  NSRect initial_frame = [ns_window frame];
  NSRect maximized_frame = [[ns_window screen] visibleFrame];

  EXPECT_FALSE(window->IsMaximized());
  EXPECT_FALSE(window->IsFullscreen());

  // Maximize, Fullscreen, Restore, Restore.
  watcher.reset([[WindowedNSNotificationObserver alloc]
      initForNotification:NSWindowDidResizeNotification
                   object:ns_window]);
  app_window->Maximize();
  [watcher wait];
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());

  EXPECT_EQ(0, [waiter enterCount]);
  app_window->Fullscreen();
  [waiter waitForEnterCount:1 exitCount:0];
  EXPECT_FALSE(window->IsMaximized());
  EXPECT_TRUE(window->IsFullscreen());

  app_window->Restore();
  [waiter waitForEnterCount:1 exitCount:1];
  EXPECT_NSEQ(maximized_frame, [ns_window frame]);
  EXPECT_TRUE(window->IsMaximized());
  EXPECT_FALSE(window->IsFullscreen());

  app_window->Restore();
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());

  // Fullscreen, Maximize, Restore.
  app_window->Fullscreen();
  [waiter waitForEnterCount:2 exitCount:1];
  EXPECT_FALSE(window->IsMaximized());
  EXPECT_TRUE(window->IsFullscreen());

  app_window->Maximize();
  EXPECT_FALSE(window->IsMaximized());
  EXPECT_TRUE(window->IsFullscreen());

  app_window->Restore();
  [waiter waitForEnterCount:2 exitCount:2];
  EXPECT_NSEQ(initial_frame, [ns_window frame]);
  EXPECT_FALSE(window->IsMaximized());
  EXPECT_FALSE(window->IsFullscreen());
}

// Test that, in frameless windows, the web contents has the same size as the
// window.
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, Frameless) {
  AppWindow* app_window = CreateTestAppWindow("{\"frame\": \"none\"}");
  NSWindow* ns_window = app_window->GetNativeWindow();
  NSView* web_contents = app_window->web_contents()->GetNativeView();
  EXPECT_TRUE(NSEqualSizes(NSMakeSize(512, 384), [web_contents frame].size));
  // Move and resize the window.
  NSRect new_frame = NSMakeRect(50, 50, 200, 200);
  [ns_window setFrame:new_frame display:YES];
  EXPECT_TRUE(NSEqualSizes(new_frame.size, [web_contents frame].size));

  // Windows created with NSBorderlessWindowMask by default don't have shadow,
  // but packaged apps should always have one.
  EXPECT_TRUE([ns_window hasShadow]);

  // Since the window has no constraints, it should have all of the following
  // style mask bits.
  NSUInteger style_mask = NSTitledWindowMask | NSClosableWindowMask |
                          NSMiniaturizableWindowMask | NSResizableWindowMask |
                          NSTexturedBackgroundWindowMask;
  EXPECT_EQ(style_mask, [ns_window styleMask]);

  CloseAppWindow(app_window);
}

namespace {

// Test that resize and fullscreen controls are correctly enabled/disabled.
void TestControls(AppWindow* app_window) {
  NSWindow* ns_window = app_window->GetNativeWindow();

  // The window is resizable.
  EXPECT_TRUE([ns_window styleMask] & NSResizableWindowMask);
  if (base::mac::IsOSSnowLeopard())
    EXPECT_TRUE([ns_window showsResizeIndicator]);

  // Due to this bug: http://crbug.com/362039, which manifests on the Cocoa
  // implementation but not the views one, frameless windows should have
  // fullscreen controls disabled.
  BOOL can_fullscreen =
      ![NSStringFromClass([ns_window class]) isEqualTo:@"AppFramelessNSWindow"];
  // The window can fullscreen and maximize.
  if (base::mac::IsOSLionOrLater()) {
    EXPECT_EQ(can_fullscreen, !!([ns_window collectionBehavior] &
                                 NSWindowCollectionBehaviorFullScreenPrimary));
  }

  // In OSX 10.10+, the zoom button performs the zoom action rather than the
  // fullscreen action. The above check that collectionBehavior does not include
  // NSWindowCollectionBehaviorFullScreenPrimary is sufficient to determine that
  // the window can't be fullscreened.
  if (base::mac::IsOSMavericksOrEarlier()) {
    EXPECT_EQ(can_fullscreen,
              [[ns_window standardWindowButton:NSWindowZoomButton] isEnabled]);
  }

  // Set a maximum size.
  app_window->SetContentSizeConstraints(gfx::Size(), gfx::Size(200, 201));
  EXPECT_EQ(200, [ns_window contentMaxSize].width);
  EXPECT_EQ(201, [ns_window contentMaxSize].height);
  NSView* web_contents = app_window->web_contents()->GetNativeView();
  EXPECT_EQ(200, [web_contents frame].size.width);
  EXPECT_EQ(201, [web_contents frame].size.height);

  // Still resizable.
  EXPECT_TRUE([ns_window styleMask] & NSResizableWindowMask);
  if (base::mac::IsOSSnowLeopard())
    EXPECT_TRUE([ns_window showsResizeIndicator]);

  // Fullscreen and maximize are disabled.
  if (base::mac::IsOSLionOrLater())
    EXPECT_FALSE([ns_window collectionBehavior] &
                 NSWindowCollectionBehaviorFullScreenPrimary);
  EXPECT_FALSE([[ns_window standardWindowButton:NSWindowZoomButton] isEnabled]);

  // Set a minimum size equal to the maximum size.
  app_window->SetContentSizeConstraints(gfx::Size(200, 201),
                                        gfx::Size(200, 201));
  EXPECT_EQ(200, [ns_window contentMinSize].width);
  EXPECT_EQ(201, [ns_window contentMinSize].height);

  // No longer resizable.
  EXPECT_FALSE([ns_window styleMask] & NSResizableWindowMask);
  if (base::mac::IsOSSnowLeopard())
    EXPECT_FALSE([ns_window showsResizeIndicator]);

  // If a window is made fullscreen by the API, fullscreen should be enabled so
  // the user can exit fullscreen.
  if (base::mac::IsOSLionOrLater()) {
    ui::test::ScopedFakeNSWindowFullscreen fake_fullscreen;
    base::scoped_nsobject<NSWindowFullscreenNotificationWaiter> waiter([
        [NSWindowFullscreenNotificationWaiter alloc] initWithWindow:ns_window]);
    app_window->SetFullscreen(AppWindow::FULLSCREEN_TYPE_WINDOW_API, true);
    [waiter waitForEnterCount:1 exitCount:0];
    EXPECT_TRUE([ns_window collectionBehavior] &
                NSWindowCollectionBehaviorFullScreenPrimary);
    EXPECT_EQ(NSWidth([[ns_window contentView] frame]),
              NSWidth([ns_window frame]));
    // Once it leaves fullscreen, it is disabled again.
    app_window->SetFullscreen(AppWindow::FULLSCREEN_TYPE_WINDOW_API, false);
    [waiter waitForEnterCount:1 exitCount:1];
    EXPECT_FALSE([ns_window collectionBehavior] &
                 NSWindowCollectionBehaviorFullScreenPrimary);
  }
}

}  // namespace

IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, Controls) {
  TestControls(CreateTestAppWindow("{}"));
}

IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, ControlsFrameless) {
  TestControls(CreateTestAppWindow("{\"frame\": \"none\"}"));
}

// Test that the colored frames have the correct color when active and inactive.
// TODO(tapted): Fix this test. http://crbug.com/508722
IN_PROC_BROWSER_TEST_P(NativeAppWindowCocoaBrowserTest, DISABLED_FrameColor) {
  // The hex values indicate an RGB color. When we get the NSColor later, the
  // components are CGFloats in the range [0, 1].
  extensions::AppWindow* app_window = CreateTestAppWindow(
      "{\"frame\": {\"color\": \"#FF0000\", \"inactiveColor\": \"#0000FF\"}}");
  NSWindow* ns_window = app_window->GetNativeWindow();
  // Disable color correction so we can read unmodified values from the bitmap.
  [ns_window setColorSpace:[NSColorSpace sRGBColorSpace]];

  NSView* frame_view = [[ns_window contentView] superview];
  NSRect bounds = [frame_view bounds];
  NSBitmapImageRep* bitmap =
      [frame_view bitmapImageRepForCachingDisplayInRect:bounds];

  [frame_view cacheDisplayInRect:bounds toBitmapImageRep:bitmap];
  NSColor* color = [bitmap colorAtX:NSMidX(bounds) y:5];
  // The window is currently inactive so it should be blue (#0000FF).
  EXPECT_EQ(0, [color redComponent]);
  EXPECT_EQ(0, [color greenComponent]);
  EXPECT_EQ(1, [color blueComponent]);

  ScopedFakeNSWindowMainStatus fake_main(ns_window);

  [frame_view cacheDisplayInRect:bounds toBitmapImageRep:bitmap];
  color = [bitmap colorAtX:NSMidX(bounds) y:5];
  // The window is now active so it should be red (#FF0000).
  EXPECT_EQ(1, [color redComponent]);
  EXPECT_EQ(0, [color greenComponent]);
  EXPECT_EQ(0, [color blueComponent]);
}

INSTANTIATE_TEST_CASE_P(NativeAppWindowCocoaBrowserTestInstance,
                        NativeAppWindowCocoaBrowserTest,
                        ::testing::Bool());
