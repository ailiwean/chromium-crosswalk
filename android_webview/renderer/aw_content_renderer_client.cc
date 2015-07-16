// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/renderer/aw_content_renderer_client.h"

#include "android_webview/common/aw_resource.h"
#include "android_webview/common/render_view_messages.h"
#include "android_webview/common/url_constants.h"
#include "android_webview/renderer/aw_content_settings_client.h"
#include "android_webview/renderer/aw_key_systems.h"
#include "android_webview/renderer/aw_message_port_client.h"
#include "android_webview/renderer/aw_print_web_view_helper_delegate.h"
#include "android_webview/renderer/aw_render_frame_ext.h"
#include "android_webview/renderer/aw_render_view_ext.h"
#include "android_webview/renderer/print_render_frame_observer.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/content/renderer/autofill_agent.h"
#include "components/autofill/content/renderer/password_autofill_agent.h"
#include "components/printing/renderer/print_web_view_helper.h"
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "net/base/escape.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "url/gurl.h"

using content::RenderThread;

namespace android_webview {

AwContentRendererClient::AwContentRendererClient() {
}

AwContentRendererClient::~AwContentRendererClient() {
}

void AwContentRendererClient::RenderThreadStarted() {
  blink::WebString content_scheme(
      base::ASCIIToUTF16(android_webview::kContentScheme));
  blink::WebSecurityPolicy::registerURLSchemeAsLocal(content_scheme);

  blink::WebString aw_scheme(
      base::ASCIIToUTF16(android_webview::kAndroidWebViewVideoPosterScheme));
  blink::WebSecurityPolicy::registerURLSchemeAsSecure(aw_scheme);

  RenderThread* thread = RenderThread::Get();

  aw_render_process_observer_.reset(new AwRenderProcessObserver);
  thread->AddObserver(aw_render_process_observer_.get());

  visited_link_slave_.reset(new visitedlink::VisitedLinkSlave);
  thread->AddObserver(visited_link_slave_.get());
}

void AwContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new AwContentSettingsClient(render_frame);
  new PrintRenderFrameObserver(render_frame);
  new AwRenderFrameExt(render_frame);
  new AwMessagePortClient(render_frame);

  // TODO(jam): when the frame tree moves into content and parent() works at
  // RenderFrame construction, simplify this by just checking parent().
  content::RenderFrame* parent_frame =
      render_frame->GetRenderView()->GetMainRenderFrame();
  if (parent_frame && parent_frame != render_frame) {
    // Avoid any race conditions from having the browser's UI thread tell the IO
    // thread that a subframe was created.
    RenderThread::Get()->Send(new AwViewHostMsg_SubFrameCreated(
        parent_frame->GetRoutingID(), render_frame->GetRoutingID()));
  }

  // TODO(sgurun) do not create a password autofill agent (change
  // autofill agent to store a weakptr).
  autofill::PasswordAutofillAgent* password_autofill_agent =
      new autofill::PasswordAutofillAgent(render_frame);
  new autofill::AutofillAgent(render_frame, password_autofill_agent, NULL);
}

void AwContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  AwRenderViewExt::RenderViewCreated(render_view);

  new printing::PrintWebViewHelper(
      render_view,
      scoped_ptr<printing::PrintWebViewHelper::Delegate>(
          new AwPrintWebViewHelperDelegate()));
}

bool AwContentRendererClient::HasErrorPage(int http_status_code,
                          std::string* error_domain) {
  return http_status_code >= 400;
}

void AwContentRendererClient::GetNavigationErrorStrings(
    content::RenderView* /* render_view */,
    blink::WebFrame* /* frame */,
    const blink::WebURLRequest& failed_request,
    const blink::WebURLError& error,
    std::string* error_html,
    base::string16* error_description) {
  if (error_html) {
    GURL error_url(failed_request.url());
    std::string err = base::UTF16ToUTF8(error.localizedDescription);
    std::string contents;
    if (err.empty()) {
      contents = AwResource::GetNoDomainPageContent();
    } else {
      contents = AwResource::GetLoadErrorPageContent();
      base::ReplaceSubstringsAfterOffset(&contents, 0, "%e", err);
    }

    base::ReplaceSubstringsAfterOffset(&contents, 0, "%s",
        net::EscapeForHTML(error_url.possibly_invalid_spec()));
    *error_html = contents;
  }
  if (error_description) {
    if (error.localizedDescription.isEmpty())
      *error_description = base::ASCIIToUTF16(net::ErrorToString(error.reason));
    else
      *error_description = error.localizedDescription;
  }
}

unsigned long long AwContentRendererClient::VisitedLinkHash(
    const char* canonical_url,
    size_t length) {
  return visited_link_slave_->ComputeURLFingerprint(canonical_url, length);
}

bool AwContentRendererClient::IsLinkVisited(unsigned long long link_hash) {
  return visited_link_slave_->IsVisited(link_hash);
}

void AwContentRendererClient::AddKeySystems(
    std::vector<media::KeySystemInfo>* key_systems) {
  AwAddKeySystems(key_systems);
}

bool AwContentRendererClient::ShouldOverridePageVisibilityState(
    const content::RenderFrame* render_frame,
    blink::WebPageVisibilityState* override_state) {
  // webview is always visible due to rendering requirements.
  *override_state = blink::WebPageVisibilityStateVisible;
  return true;
}

}  // namespace android_webview
