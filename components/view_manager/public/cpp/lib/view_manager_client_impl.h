// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIEW_MANAGER_PUBLIC_CPP_LIB_VIEW_MANAGER_CLIENT_IMPL_H_
#define COMPONENTS_VIEW_MANAGER_PUBLIC_CPP_LIB_VIEW_MANAGER_CLIENT_IMPL_H_

#include "components/view_manager/public/cpp/types.h"
#include "components/view_manager/public/cpp/view.h"
#include "components/view_manager/public/cpp/view_manager.h"
#include "components/view_manager/public/interfaces/view_manager.mojom.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/strong_binding.h"

namespace mojo {
class Shell;
class ViewManager;
class ViewManagerDelegate;
class ViewManagerTransaction;

// Manages the connection with the View Manager service.
class ViewManagerClientImpl : public ViewManager,
                              public ViewManagerClient,
                              public ErrorHandler {
 public:
  ViewManagerClientImpl(ViewManagerDelegate* delegate,
                        Shell* shell,
                        InterfaceRequest<ViewManagerClient> request);
  ~ViewManagerClientImpl() override;

  bool connected() const { return service_; }
  ConnectionSpecificId connection_id() const { return connection_id_; }

  // API exposed to the view implementations that pushes local changes to the
  // service.
  void DestroyView(Id view_id);

  // These methods take TransportIds. For views owned by the current connection,
  // the connection id high word can be zero. In all cases, the TransportId 0x1
  // refers to the root view.
  void AddChild(Id child_id, Id parent_id);
  void RemoveChild(Id child_id, Id parent_id);

  void Reorder(Id view_id, Id relative_view_id, OrderDirection direction);

  // Returns true if the specified view was created by this connection.
  bool OwnsView(Id id) const;

  void SetBounds(Id view_id, const Rect& bounds);
  void SetSurfaceId(Id view_id, SurfaceIdPtr surface_id);
  void SetFocus(Id view_id);
  void SetVisible(Id view_id, bool visible);
  void SetProperty(Id view_id,
                   const std::string& name,
                   const std::vector<uint8_t>& data);

  void Embed(const String& url, Id view_id);
  void Embed(mojo::URLRequestPtr request,
             Id view_id,
             InterfaceRequest<ServiceProvider> services,
             ServiceProviderPtr exposed_services);
  void Embed(Id view_id, ViewManagerClientPtr client);
  void EmbedAllowingReembed(mojo::URLRequestPtr request, Id view_id);

  void set_change_acked_callback(const Callback<void(void)>& callback) {
    change_acked_callback_ = callback;
  }
  void ClearChangeAckedCallback() { change_acked_callback_.reset(); }

  // Start/stop tracking views. While tracked, they can be retrieved via
  // ViewManager::GetViewById.
  void AddView(View* view);
  void RemoveView(Id view_id);

  bool is_embed_root() const { return is_embed_root_; }

  // Called after the root view's observers have been notified of destruction
  // (as the last step of ~View). This ordering ensures that the View Manager
  // is torn down after the root.
  void OnRootDestroyed(View* root);

 private:
  typedef std::map<Id, View*> IdToViewMap;

  Id CreateViewOnServer();

  // Overridden from ViewManager:
  View* GetRoot() override;
  View* GetViewById(Id id) override;
  View* GetFocusedView() override;
  View* CreateView() override;
  void SetEmbedRoot() override;

  // Overridden from ViewManagerClient:
  void OnEmbed(ConnectionSpecificId connection_id,
               ViewDataPtr root,
               ViewManagerServicePtr view_manager_service,
               Id focused_view_id) override;
  void OnEmbedForDescendant(
      Id view,
      mojo::URLRequestPtr request,
      const OnEmbedForDescendantCallback& callback) override;
  void OnEmbeddedAppDisconnected(Id view_id) override;
  void OnViewBoundsChanged(Id view_id,
                           RectPtr old_bounds,
                           RectPtr new_bounds) override;
  void OnViewViewportMetricsChanged(ViewportMetricsPtr old_metrics,
                                    ViewportMetricsPtr new_metrics) override;
  void OnViewHierarchyChanged(Id view_id,
                              Id new_parent_id,
                              Id old_parent_id,
                              Array<ViewDataPtr> views) override;
  void OnViewReordered(Id view_id,
                       Id relative_view_id,
                       OrderDirection direction) override;
  void OnViewDeleted(Id view_id) override;
  void OnViewVisibilityChanged(Id view_id, bool visible) override;
  void OnViewDrawnStateChanged(Id view_id, bool drawn) override;
  void OnViewSharedPropertyChanged(Id view_id,
                                   const String& name,
                                   Array<uint8_t> new_data) override;
  void OnViewInputEvent(Id view_id,
                        EventPtr event,
                        const Callback<void()>& callback) override;
  void OnViewFocused(Id focused_view_id) override;

  // ErrorHandler implementation.
  void OnConnectionError() override;

  void RootDestroyed(View* root);

  void OnActionCompleted(bool success);

  Callback<void(bool)> ActionCompletedCallback();

  ConnectionSpecificId connection_id_;
  ConnectionSpecificId next_id_;

  Callback<void(void)> change_acked_callback_;

  ViewManagerDelegate* delegate_;

  View* root_;

  IdToViewMap views_;

  View* capture_view_;
  View* focused_view_;
  View* activated_view_;

  Binding<ViewManagerClient> binding_;
  ViewManagerServicePtr service_;

  bool is_embed_root_;

  bool in_destructor_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(ViewManagerClientImpl);
};

}  // namespace mojo

#endif  // COMPONENTS_VIEW_MANAGER_PUBLIC_CPP_LIB_VIEW_MANAGER_CLIENT_IMPL_H_
