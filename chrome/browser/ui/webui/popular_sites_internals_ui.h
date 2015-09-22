// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_POPULAR_SITES_INTERNALS_UI_H_
#define CHROME_BROWSER_UI_WEBUI_POPULAR_SITES_INTERNALS_UI_H_

#include "base/macros.h"
#include "content/public/browser/web_ui_controller.h"

// The implementation for the chrome://popular-sites-internals page.
class PopularSitesInternalsUI : public content::WebUIController {
 public:
  explicit PopularSitesInternalsUI(content::WebUI* web_ui);
  ~PopularSitesInternalsUI() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(PopularSitesInternalsUI);
};

#endif  // CHROME_BROWSER_UI_WEBUI_POPULAR_SITES_INTERNALS_UI_H_
