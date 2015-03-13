// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CONTENT_SHELL_BROWSER_SHELL_H_
#define CONTENT_SHELL_BROWSER_SHELL_H_


#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "ipc/ipc_channel.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"

#if defined(OS_ANDROID)
#include "base/android/scoped_java_ref.h"
#elif defined(USE_AURA)
#if defined(OS_CHROMEOS)
namespace gfx {
class Screen;
}
namespace wm {
class WMTestHelper;
}
#endif  // defined(OS_CHROMEOS)
namespace views {
class Widget;
class ViewsDelegate;
}
#endif  // defined(USE_AURA)

class GURL;
namespace content {

#if defined(USE_AURA)
class ShellPlatformDataAura;
#endif

class BrowserContext;
class ShellDevToolsFrontend;
class ShellJavaScriptDialogManager;
class SiteInstance;
class WebContents;

// This represents one window of the Content Shell, i.e. all the UI including
// buttons and url bar, as well as the web content area.
class Shell : public WebContentsDelegate,
              public WebContentsObserver {
 public:
  ~Shell() override;

  void LoadURL(const GURL& url);
  void LoadURLForFrame(const GURL& url, const std::string& frame_name);
  void LoadDataWithBaseURL(const GURL& url,
                           const std::string& data,
                           const GURL& base_url);
  void GoBackOrForward(int offset);
  void Reload();
  void Stop();
  void UpdateNavigationControls(bool to_different_document);
  void Close();
  void ShowDevTools();
  void ShowDevToolsForElementAt(int x, int y);
  void CloseDevTools();
#if defined(OS_MACOSX)
  // Resizes the web content view to the given dimensions.
  void SizeTo(const gfx::Size& content_size);
#endif

  // Do one time initialization at application startup.
  static void Initialize();

  static Shell* CreateNewWindow(BrowserContext* browser_context,
                                const GURL& url,
                                SiteInstance* site_instance,
                                const gfx::Size& initial_size);

  // Returns the Shell object corresponding to the given RenderViewHost.
  static Shell* FromRenderViewHost(RenderViewHost* rvh);

  // Returns the currently open windows.
  static std::vector<Shell*>& windows() { return windows_; }

  // Closes all windows and returns. This runs a message loop.
  static void CloseAllWindows();

  // Used for content_browsertests. Called once.
  static void SetShellCreatedCallback(
      base::Callback<void(Shell*)> shell_created_callback);

  WebContents* web_contents() const { return web_contents_.get(); }
  gfx::NativeWindow window() { return window_; }

#if defined(OS_MACOSX)
  // Public to be called by an ObjC bridge object.
  void ActionPerformed(int control);
  void URLEntered(std::string url_string);
#elif defined(OS_ANDROID)
  // Registers the Android Java to native methods.
  static bool Register(JNIEnv* env);
#endif

  // WebContentsDelegate
  WebContents* OpenURLFromTab(WebContents* source,
                              const OpenURLParams& params) override;
  void AddNewContents(WebContents* source,
                      WebContents* new_contents,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_rect,
                      bool user_gesture,
                      bool* was_blocked) override;
  void LoadingStateChanged(WebContents* source,
                           bool to_different_document) override;
#if defined(OS_ANDROID)
  void LoadProgressChanged(WebContents* source, double progress) override;
#endif
  void EnterFullscreenModeForTab(WebContents* web_contents,
                                 const GURL& origin) override;
  void ExitFullscreenModeForTab(WebContents* web_contents) override;
  bool IsFullscreenForTabOrPending(
      const WebContents* web_contents) const override;
  void RequestToLockMouse(WebContents* web_contents,
                          bool user_gesture,
                          bool last_unlocked_by_target) override;
  void CloseContents(WebContents* source) override;
  bool CanOverscrollContent() const override;
  void DidNavigateMainFramePostCommit(WebContents* web_contents) override;
  JavaScriptDialogManager* GetJavaScriptDialogManager(
      WebContents* source) override;
#if defined(OS_MACOSX)
  void HandleKeyboardEvent(WebContents* source,
                           const NativeWebKeyboardEvent& event) override;
#endif
  bool AddMessageToConsole(WebContents* source,
                           int32 level,
                           const base::string16& message,
                           int32 line_no,
                           const base::string16& source_id) override;
  void RendererUnresponsive(WebContents* source) override;
  void ActivateContents(WebContents* contents) override;
  void DeactivateContents(WebContents* contents) override;
  void WorkerCrashed(WebContents* source) override;
  bool HandleContextMenu(const content::ContextMenuParams& params) override;
  void WebContentsFocused(WebContents* contents) override;

  static gfx::Size GetShellDefaultSize();

 private:
  enum UIControl {
    BACK_BUTTON,
    FORWARD_BUTTON,
    STOP_BUTTON
  };

  class DevToolsWebContentsObserver;

  explicit Shell(WebContents* web_contents);

  // Helper to create a new Shell given a newly created WebContents.
  static Shell* CreateShell(WebContents* web_contents,
                            const gfx::Size& initial_size);

  // Helper for one time initialization of application
  static void PlatformInitialize(const gfx::Size& default_window_size);
  // Helper for one time deinitialization of platform specific state.
  static void PlatformExit();

  // Adjust the size when Blink sends 0 for width and/or height.
  // This happens when Blink requests a default-sized window.
  static gfx::Size AdjustWindowSize(const gfx::Size& initial_size);

  // All the methods that begin with Platform need to be implemented by the
  // platform specific Shell implementation.
  // Called from the destructor to let each platform do any necessary cleanup.
  void PlatformCleanUp();
  // Creates the main window GUI.
  void PlatformCreateWindow(int width, int height);
  // Links the WebContents into the newly created window.
  void PlatformSetContents();
  // Resize the content area and GUI.
  void PlatformResizeSubViews();
  // Enable/disable a button.
  void PlatformEnableUIControl(UIControl control, bool is_enabled);
  // Updates the url in the url bar.
  void PlatformSetAddressBarURL(const GURL& url);
  // Sets whether the spinner is spinning.
  void PlatformSetIsLoading(bool loading);
  // Set the title of shell window
  void PlatformSetTitle(const base::string16& title);
  // User right-clicked on the web view
  bool PlatformHandleContextMenu(const content::ContextMenuParams& params);
#if defined(OS_ANDROID)
  void PlatformToggleFullscreenModeForTab(WebContents* web_contents,
                                          bool enter_fullscreen);
  bool PlatformIsFullscreenForTabOrPending(
      const WebContents* web_contents) const;
#endif
#if defined(TOOLKIT_VIEWS)
  void PlatformWebContentsFocused(WebContents* contents);
#endif

  gfx::NativeView GetContentView();

  void ToggleFullscreenModeForTab(WebContents* web_contents,
                                  bool enter_fullscreen);
  // WebContentsObserver
  void TitleWasSet(NavigationEntry* entry, bool explicit_set) override;

  void InnerShowDevTools();
  void OnDevToolsWebContentsDestroyed();

  scoped_ptr<ShellJavaScriptDialogManager> dialog_manager_;

  scoped_ptr<WebContents> web_contents_;

  scoped_ptr<DevToolsWebContentsObserver> devtools_observer_;
  ShellDevToolsFrontend* devtools_frontend_;

  bool is_fullscreen_;

  gfx::NativeWindow window_;
  gfx::NativeEditView url_edit_view_;

  gfx::Size content_size_;

#if defined(OS_ANDROID)
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
#elif defined(USE_AURA)
#if defined(OS_CHROMEOS)
  static wm::WMTestHelper* wm_test_helper_;
  static gfx::Screen* test_screen_;
#endif
#if defined(TOOLKIT_VIEWS)
  static views::ViewsDelegate* views_delegate_;

  views::Widget* window_widget_;
#endif // defined(TOOLKIT_VIEWS)
  static ShellPlatformDataAura* platform_;
#endif  // defined(USE_AURA)

  bool headless_;

  // A container of all the open windows. We use a vector so we can keep track
  // of ordering.
  static std::vector<Shell*> windows_;

  static base::Callback<void(Shell*)> shell_created_callback_;

  // True if the destructur of Shell should post a quit closure on the current
  // message loop if the destructed Shell object was the last one.
  static bool quit_message_loop_;
};

}  // namespace content

#endif  // CONTENT_SHELL_BROWSER_SHELL_H_
