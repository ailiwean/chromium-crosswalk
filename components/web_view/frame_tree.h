// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_WEB_VIEW_FRAME_TREE_H_
#define COMPONENTS_WEB_VIEW_FRAME_TREE_H_

#include "components/view_manager/public/interfaces/view_tree.mojom.h"
#include "components/web_view/frame.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/array.h"

namespace mojo {
class String;
}

namespace web_view {

class FrameTreeClient;
class FrameTreeDelegate;
class FrameUserData;

// FrameTree manages the set of Frames that comprise a single url. FrameTree
// owns the root Frame and each Frame owns its children. Frames are
// automatically deleted and removed from the tree if the corresponding view is
// deleted. This happens if the creator of the view deletes it (say an iframe is
// destroyed).
class FrameTree {
 public:
  // |view| is the view to do the initial embedding in. It is assumed |view|
  // outlives FrameTree.
  // |client_properties| is the client properties for the root frame.
  // |root_app_id| is a unique identifier of the app providing |root_client|.
  // See Frame for details on app id's.
  FrameTree(uint32_t root_app_id,
            mojo::View* view,
            mojo::ViewTreeClientPtr view_tree_client,
            FrameTreeDelegate* delegate,
            FrameTreeClient* root_client,
            scoped_ptr<FrameUserData> user_data,
            const Frame::ClientPropertyMap& client_properties);
  ~FrameTree();

  // Returns true if there should be a distinct renderer per frame. This is
  // useful for testing.
  static bool AlwaysCreateNewFrameTree();

  Frame* root() { return &root_; }

  uint32_t change_id() const { return change_id_; }

  Frame* CreateAndAddFrame(mojo::View* view,
                           Frame* parent,
                           uint32_t app_id,
                           FrameTreeClient* client,
                           scoped_ptr<FrameUserData> user_data);

  // Creates a new Frame parented to |parent|. The Frame is considered shared in
  // that it is sharing the FrameTreeClient/FrameTreeServer of |parent|. There
  // may or may not be a View identified by |frame_id| yet. See Frame for
  // details.
  Frame* CreateSharedFrame(Frame* parent,
                           uint32_t frame_id,
                           uint32_t app_id,
                           const Frame::ClientPropertyMap& client_properties);

 private:
  friend class Frame;

  // Increments the change id, returning the new value.
  uint32_t AdvanceChangeID();

  Frame* CreateAndAddFrameImpl(
      mojo::View* view,
      uint32_t frame_id,
      uint32_t app_id,
      Frame* parent,
      FrameTreeClient* client,
      scoped_ptr<FrameUserData> user_data,
      const Frame::ClientPropertyMap& client_properties);

  void LoadingStateChanged();
  void ProgressChanged();
  void TitleChanged(const mojo::String& title);
  void ClientPropertyChanged(const Frame* source,
                             const mojo::String& name,
                             const mojo::Array<uint8_t>& value);

  mojo::View* view_;

  FrameTreeDelegate* delegate_;

  Frame root_;

  double progress_;

  uint32_t change_id_;

  DISALLOW_COPY_AND_ASSIGN(FrameTree);
};

}  // namespace web_view

#endif  // COMPONENTS_WEB_VIEW_FRAME_TREE_H_
