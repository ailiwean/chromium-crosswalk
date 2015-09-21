// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/web_view/web_view_impl.h"

#include "base/command_line.h"
#include "components/devtools_service/public/cpp/switches.h"
#include "components/mus/public/cpp/scoped_view_ptr.h"
#include "components/mus/public/cpp/view.h"
#include "components/mus/public/cpp/view_tree_connection.h"
#include "components/web_view/client_initiated_frame_connection.h"
#include "components/web_view/frame.h"
#include "components/web_view/frame_connection.h"
#include "components/web_view/frame_devtools_agent.h"
#include "components/web_view/frame_tree.h"
#include "components/web_view/navigation_entry.h"
#include "components/web_view/pending_web_view_load.h"
#include "components/web_view/url_request_cloneable.h"
#include "mojo/application/public/cpp/application_impl.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "url/gurl.h"

namespace web_view {
namespace {

bool EnableRemoteDebugging() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      devtools_service::kRemoteDebuggingPort);
}

}  // namespace

using web_view::mojom::ButtonState;

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, public:

WebViewImpl::WebViewImpl(mojo::ApplicationImpl* app,
                         mojom::WebViewClientPtr client,
                         mojo::InterfaceRequest<mojom::WebView> request)
    : app_(app),
      client_(client.Pass()),
      binding_(this, request.Pass()),
      root_(nullptr),
      content_(nullptr),
      navigation_controller_(this) {
  if (EnableRemoteDebugging())
    devtools_agent_.reset(new FrameDevToolsAgent(app_, this));
  OnDidNavigate();
}

WebViewImpl::~WebViewImpl() {
  if (content_)
    content_->RemoveObserver(this);
  if (root_) {
    root_->RemoveObserver(this);
    mus::ScopedViewPtr::DeleteViewOrViewManager(root_);
  }
}

void WebViewImpl::OnLoad() {
  scoped_ptr<PendingWebViewLoad> pending_load(pending_load_.Pass());
  scoped_ptr<FrameConnection> frame_connection(
      pending_load->frame_connection());
  mojo::ViewTreeClientPtr view_tree_client =
      frame_connection->GetViewTreeClient();

  Frame::ClientPropertyMap client_properties;
  if (devtools_agent_) {
    devtools_service::DevToolsAgentPtr forward_agent;
    frame_connection->application_connection()->ConnectToService(
        &forward_agent);
    devtools_agent_->AttachFrame(forward_agent.Pass(), &client_properties);
  }

  mojom::FrameClient* frame_client = frame_connection->frame_client();
  const uint32_t content_handler_id = frame_connection->GetContentHandlerID();
  frame_tree_.reset(new FrameTree(content_handler_id, content_,
                                  view_tree_client.Pass(), this, frame_client,
                                  frame_connection.Pass(), client_properties));
}

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, WebView implementation:

void WebViewImpl::LoadRequest(mojo::URLRequestPtr request) {
  navigation_controller_.LoadURL(request.Pass());
}

void WebViewImpl::GetViewTreeClient(
    mojo::InterfaceRequest<mojo::ViewTreeClient> view_tree_client) {
  mus::ViewTreeConnection::Create(this, view_tree_client.Pass());
}

void WebViewImpl::GoBack() {
  if (!navigation_controller_.CanGoBack())
    return;
  navigation_controller_.GoBack();
}

void WebViewImpl::GoForward() {
  if (!navigation_controller_.CanGoForward())
    return;
  navigation_controller_.GoForward();
}

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, mus::ViewTreeDelegate implementation:

void WebViewImpl::OnEmbed(mus::View* root) {
  // We must have been granted embed root priviledges, otherwise we can't
  // Embed() in any descendants.
  DCHECK(root->connection()->IsEmbedRoot());
  root->AddObserver(this);
  root_ = root;
  content_ = root->connection()->CreateView();
  content_->SetBounds(*mojo::Rect::From(gfx::Rect(0, 0, root->bounds().width,
                                                  root->bounds().height)));
  root->AddChild(content_);
  content_->SetVisible(true);
  content_->AddObserver(this);

  if (pending_load_ && pending_load_->is_content_handler_id_valid())
    OnLoad();
}

void WebViewImpl::OnConnectionLost(mus::ViewTreeConnection* connection) {
  root_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, mus::ViewObserver implementation:

void WebViewImpl::OnViewBoundsChanged(mus::View* view,
                                      const mojo::Rect& old_bounds,
                                      const mojo::Rect& new_bounds) {
  if (view != content_) {
    mojo::Rect rect;
    rect.width = new_bounds.width;
    rect.height = new_bounds.height;
    content_->SetBounds(rect);
  }
}

void WebViewImpl::OnViewDestroyed(mus::View* view) {
  // |FrameTree| cannot outlive the content view.
  if (view == content_) {
    frame_tree_.reset();
    content_ = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, FrameTreeDelegate implementation:

scoped_ptr<FrameUserData> WebViewImpl::CreateUserDataForNewFrame(
    mojom::FrameClientPtr frame_client) {
  return make_scoped_ptr(
      new ClientInitiatedFrameConnection(frame_client.Pass()));
}

bool WebViewImpl::CanPostMessageEventToFrame(const Frame* source,
                                             const Frame* target,
                                             mojom::HTMLMessageEvent* event) {
  return true;
}

void WebViewImpl::LoadingStateChanged(bool loading, double progress) {
  client_->LoadingStateChanged(loading, progress);
}

void WebViewImpl::TitleChanged(const mojo::String& title) {
  client_->TitleChanged(title);
}

void WebViewImpl::NavigateTopLevel(Frame* source, mojo::URLRequestPtr request) {
  client_->TopLevelNavigate(request.Pass());
}

void WebViewImpl::CanNavigateFrame(Frame* target,
                                   mojo::URLRequestPtr request,
                                   const CanNavigateFrameCallback& callback) {
  FrameConnection::CreateConnectionForCanNavigateFrame(
      app_, target, request.Pass(), callback);
}

void WebViewImpl::DidStartNavigation(Frame* frame) {}

void WebViewImpl::DidCommitProvisionalLoad(Frame* frame) {
  navigation_controller_.FrameDidCommitProvisionalLoad(frame);
}

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, FrameDevToolsAgentDelegate implementation:

void WebViewImpl::HandlePageNavigateRequest(const GURL& url) {
  mojo::URLRequestPtr request(mojo::URLRequest::New());
  request->url = url.spec();
  client_->TopLevelNavigate(request.Pass());
}

////////////////////////////////////////////////////////////////////////////////
// WebViewImpl, NavigationControllerDelegate implementation:

void WebViewImpl::OnNavigate(mojo::URLRequestPtr request) {
  pending_load_.reset(new PendingWebViewLoad(this));
  pending_load_->Init(request.Pass());
}

void WebViewImpl::OnDidNavigate() {
  client_->BackForwardChanged(navigation_controller_.CanGoBack()
                                  ? ButtonState::BUTTON_STATE_ENABLED
                                  : ButtonState::BUTTON_STATE_DISABLED,
                              navigation_controller_.CanGoForward()
                                  ? ButtonState::BUTTON_STATE_ENABLED
                                  : ButtonState::BUTTON_STATE_DISABLED);
}

}  // namespace web_view
