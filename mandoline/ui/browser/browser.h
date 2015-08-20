// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MANDOLINE_UI_BROWSER_BROWSER_H_
#define MANDOLINE_UI_BROWSER_BROWSER_H_

#include "base/gtest_prod_util.h"
#include "components/view_manager/public/cpp/view_manager.h"
#include "components/view_manager/public/cpp/view_manager_delegate.h"
#include "components/view_manager/public/cpp/view_manager_init.h"
#include "components/view_manager/public/interfaces/view_manager_root.mojom.h"
#include "mandoline/tab/public/cpp/web_view.h"
#include "mandoline/tab/public/interfaces/web_view.mojom.h"
#include "mandoline/ui/browser/public/interfaces/omnibox.mojom.h"
#include "mandoline/ui/browser/public/interfaces/view_embedder.mojom.h"
#include "mojo/application/public/cpp/application_delegate.h"
#include "mojo/application/public/cpp/application_impl.h"
#include "mojo/application/public/cpp/connect.h"
#include "mojo/common/weak_binding_set.h"
#include "ui/mojo/events/input_events.mojom.h"
#include "url/gurl.h"

namespace mojo {
class ViewManagerInit;
}

namespace mandoline {

FORWARD_DECLARE_TEST(BrowserTest, ClosingBrowserClosesAppConnection);

class BrowserDelegate;
class BrowserUI;

class Browser : public mojo::ViewManagerDelegate,
                public mojo::ViewManagerRootClient,
                public web_view::mojom::WebViewClient,
                public ViewEmbedder,
                public mojo::InterfaceFactory<ViewEmbedder> {
 public:
  Browser(mojo::ApplicationImpl* app, BrowserUI* ui);
  ~Browser() override;

  mojo::View* content() { return content_; }

  const GURL& current_url() const { return current_url_; }

  void LoadURL(const GURL& url);

  // Starts the Omnibox application (if necessary) and shows it.
  void ShowOmnibox();

 private:
  FRIEND_TEST_ALL_PREFIXES(BrowserTest, ClosingBrowserClosesAppConnection);
  FRIEND_TEST_ALL_PREFIXES(BrowserTest, TwoBrowsers);

  friend class TestBrowser;

  mojo::ApplicationConnection* GetViewManagerConnectionForTesting();

  // Overridden from mojo::ViewManagerDelegate:
  void OnEmbed(mojo::View* root) override;
  void OnViewManagerDestroyed(mojo::ViewManager* view_manager) override;

  // Overridden from ViewManagerRootClient:
  void OnAccelerator(mojo::EventPtr event) override;

  // Overridden from web_view::mojom::WebViewClient:
  void TopLevelNavigate(mojo::URLRequestPtr request) override;
  void LoadingStateChanged(bool is_loading) override;
  void ProgressChanged(double progress) override;

  // Overridden from ViewEmbedder:
  void Embed(mojo::URLRequestPtr request) override;

  // Overridden from mojo::InterfaceFactory<ViewEmbedder>:
  void Create(mojo::ApplicationConnection* connection,
              mojo::InterfaceRequest<ViewEmbedder> request) override;

  mojo::ApplicationImpl* app_;
  BrowserUI* ui_;

  mojo::ViewManagerInit view_manager_init_;

  // Only support being embedded once, so both application-level
  // and embedding-level state are shared on the same object.
  mojo::View* root_;
  mojo::View* content_;
  GURL default_url_;

  mojo::WeakBindingSet<ViewEmbedder> view_embedder_bindings_;

  GURL current_url_;

  web_view::WebView web_view_;

  OmniboxPtr omnibox_;
  scoped_ptr<mojo::ApplicationConnection> omnibox_connection_;

  DISALLOW_COPY_AND_ASSIGN(Browser);
};

}  // namespace mandoline

#endif  // MANDOLINE_UI_BROWSER_BROWSER_H_
