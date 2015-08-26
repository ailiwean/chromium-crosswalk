// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ERROR_PAGE_RENDERER_NET_ERROR_HELPER_CORE_H_
#define COMPONENTS_ERROR_PAGE_RENDERER_NET_ERROR_HELPER_CORE_H_

#include <string>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"
#include "components/error_page/common/net_error_info.h"
#include "url/gurl.h"

namespace base {
class ListValue;
}

namespace blink {
struct WebURLError;
}

namespace error_page {

struct ErrorPageParams;

// Class that contains the logic for how the NetErrorHelper.  This allows for
// testing the logic without a RenderView or WebFrame, which are difficult to
// mock, and for testing races which are impossible to reliably reproduce
// with real RenderViews or WebFrames.
class NetErrorHelperCore {
 public:
  enum FrameType {
    MAIN_FRAME,
    SUB_FRAME,
  };

  enum PageType {
    NON_ERROR_PAGE,
    ERROR_PAGE,
  };

  enum Button {
    NO_BUTTON,
    RELOAD_BUTTON,
    SHOW_SAVED_COPY_BUTTON,
    MORE_BUTTON,
    EASTER_EGG,
    SHOW_CACHED_COPY_BUTTON,  // "Google cached copy" button label experiment.
    SHOW_CACHED_PAGE_BUTTON,  // "Google cached page" button label experiment.
    DIAGNOSE_ERROR,
  };

  // The Delegate handles all interaction with the RenderView, WebFrame, and
  // the network, as well as the generation of error pages.
  class Delegate {
   public:
    // Generates an error page's HTML for the given error.
    virtual void GenerateLocalizedErrorPage(
        const blink::WebURLError& error,
        bool is_failed_post,
        bool can_show_network_diagnostics_dialog,
        scoped_ptr<ErrorPageParams> params,
        bool* reload_button_shown,
        bool* show_saved_copy_button_shown,
        bool* show_cached_copy_button_shown,
        bool* show_cached_page_button_shown,
        std::string* html) const = 0;

    // Loads the given HTML in the main frame for use as an error page.
    virtual void LoadErrorPageInMainFrame(const std::string& html,
                                          const GURL& failed_url) = 0;

    // Create extra Javascript bindings in the error page. Will only be invoked
    // after an error page has finished loading.
    virtual void EnablePageHelperFunctions() = 0;

    // Updates the currently displayed error page with a new error code.  The
    // currently displayed error page must have finished loading, and must have
    // been generated by a call to GenerateLocalizedErrorPage.
    virtual void UpdateErrorPage(const blink::WebURLError& error,
                                 bool is_failed_post,
                                 bool can_show_network_diagnostics_dialog) = 0;

    // Fetches an error page and calls into OnErrorPageFetched when done.  Any
    // previous fetch must either be canceled or finished before calling.  Can't
    // be called synchronously after a previous fetch completes.
    virtual void FetchNavigationCorrections(
        const GURL& navigation_correction_url,
        const std::string& navigation_correction_request_body) = 0;

    // Cancels fetching navigation corrections.  Does nothing if no fetch is
    // ongoing.
    virtual void CancelFetchNavigationCorrections() = 0;

    // Sends an HTTP request used to track which link on the page was clicked to
    // the navigation correction service.
    virtual void SendTrackingRequest(
        const GURL& tracking_url,
        const std::string& tracking_request_body) = 0;

    // Starts a reload of the page in the observed frame.
    virtual void ReloadPage() = 0;

    // Load the original page from cache.
    virtual void LoadPageFromCache(const GURL& page_url) = 0;

    // Run the platform diagnostics too for the specified URL.
    virtual void DiagnoseError(const GURL& page_url) = 0;

   protected:
    virtual ~Delegate() {}
  };

  struct NavigationCorrectionParams {
    NavigationCorrectionParams();
    ~NavigationCorrectionParams();

    // URL used both for getting the suggestions and tracking clicks.
    GURL url;

    std::string language;
    std::string country_code;
    std::string api_key;
    GURL search_url;
  };

  NetErrorHelperCore(Delegate* delegate,
                     bool auto_reload_enabled,
                     bool auto_reload_visible_only,
                     bool is_visible);
  ~NetErrorHelperCore();

  // Examines |frame| and |error| to see if this is an error worthy of a DNS
  // probe.  If it is, initializes |error_strings| based on |error|,
  // |is_failed_post|, and |locale| with suitable strings and returns true.
  // If not, returns false, in which case the caller should look up error
  // strings directly using LocalizedError::GetNavigationErrorStrings.
  //
  // Updates the NetErrorHelper with the assumption the page will be loaded
  // immediately.
  void GetErrorHTML(FrameType frame_type,
                    const blink::WebURLError& error,
                    bool is_failed_post,
                    std::string* error_html);

  // These methods handle tracking the actual state of the page.
  void OnStartLoad(FrameType frame_type, PageType page_type);
  void OnCommitLoad(FrameType frame_type, const GURL& url);
  void OnFinishLoad(FrameType frame_type);
  void OnStop();
  void OnWasShown();
  void OnWasHidden();

  void CancelPendingFetches();

  // Called when an error page have has been retrieved over the network.  |html|
  // must be an empty string on error.
  void OnNavigationCorrectionsFetched(const std::string& corrections,
                                      const std::string& accept_languages,
                                      bool is_rtl);

  // Notifies |this| that network error information from the browser process
  // has been received.
  void OnNetErrorInfo(DnsProbeStatus status);

  // Notifies |this| if it can use a local error diagnostics service through its
  // delegate.
  void OnSetCanShowNetworkDiagnosticsDialog(
      bool can_show_network_diagnostics_dialog);

  void OnSetNavigationCorrectionInfo(const GURL& navigation_correction_url,
                                     const std::string& language,
                                     const std::string& country_code,
                                     const std::string& api_key,
                                     const GURL& search_url);
  // Notifies |this| that the network's online status changed.
  // Handler for NetworkStateChanged notification from the browser process. If
  // the network state changes to online, this method is responsible for
  // starting the auto-reload process.
  //
  // Warning: if there are many tabs sitting at an error page, this handler will
  // be run at the same time for each of their top-level renderframes, which can
  // cause many requests to be started at the same time. There's no current
  // protection against this kind of "reload storm".
  //
  // TODO(rdsmith): prevent the reload storm.
  void NetworkStateChanged(bool online);

  int auto_reload_count() const { return auto_reload_count_; }

  bool ShouldSuppressErrorPage(FrameType frame_type, const GURL& url);

  void set_timer_for_testing(scoped_ptr<base::Timer> timer) {
    auto_reload_timer_.reset(timer.release());
  }

  // Execute the effect of pressing the specified button.
  // Note that the visual effects of the 'MORE' button are taken
  // care of in JavaScript.
  void ExecuteButtonPress(Button button);

  // Reports to the correction service that the link with the given tracking
  // ID was clicked.  Only pages generated with information from the service
  // have links with tracking IDs.  Duplicate requests from the same page with
  // the same tracking ID are ignored.
  void TrackClick(int tracking_id);

 private:
  struct ErrorPageInfo;

  // Gets HTML for a main frame error page.  Depending on
  // |pending_error_page_info|, may use the navigation correction service, or
  // show a DNS probe error page.  May modify |pending_error_page_info|.
  void GetErrorHtmlForMainFrame(ErrorPageInfo* pending_error_page_info,
                                std::string* error_html);

  // Updates the currently displayed error page with a new error based on the
  // most recently received DNS probe result.  The page must have finished
  // loading before this is called.
  void UpdateErrorPage();

  blink::WebURLError GetUpdatedError(const blink::WebURLError& error) const;

  void Reload();
  bool MaybeStartAutoReloadTimer();
  void StartAutoReloadTimer();
  void AutoReloadTimerFired();
  void PauseAutoReloadTimer();

  static bool IsReloadableError(const ErrorPageInfo& info);

  Delegate* delegate_;

  // The last DnsProbeStatus received from the browser.
  DnsProbeStatus last_probe_status_;

  // Information for the provisional / "pre-provisional" error page.  NULL when
  // there's no page pending, or the pending page is not an error page.
  scoped_ptr<ErrorPageInfo> pending_error_page_info_;

  // Information for the committed error page.  NULL when the committed page is
  // not an error page.
  scoped_ptr<ErrorPageInfo> committed_error_page_info_;

  bool can_show_network_diagnostics_dialog_;

  NavigationCorrectionParams navigation_correction_params_;

  // True if auto-reload is enabled at all.
  const bool auto_reload_enabled_;

  // True if auto-reload should only run when the observed frame is visible.
  const bool auto_reload_visible_only_;

  // Timer used to wait for auto-reload attempts.
  scoped_ptr<base::Timer> auto_reload_timer_;

  // True if the auto-reload timer would be running but is waiting for an
  // offline->online network transition.
  bool auto_reload_paused_;

  // Whether an auto-reload-initiated Reload() attempt is in flight.
  bool auto_reload_in_flight_;

  // True if there is an uncommitted-but-started load, error page or not. This
  // is used to inhibit starting auto-reload when an error page finishes, in
  // case this happens:
  //   Error page starts
  //   Error page commits
  //   Non-error page starts
  //   Error page finishes
  bool uncommitted_load_started_;

  // Is the browser online?
  bool online_;

  // Is the RenderFrame this object is observing visible?
  bool visible_;

  int auto_reload_count_;

  // This value is set only when a navigation has been initiated from
  // the error page.  It is used to detect when such navigations result
  // in errors.
  Button navigation_from_button_;
};

}  // namespace error_page

#endif  // COMPONENTS_ERROR_PAGE_RENDERER_NET_ERROR_HELPER_CORE_H_
