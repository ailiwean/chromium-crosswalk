// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_SOLID_COLOR_LAYER_IMPL_H_
#define CC_LAYERS_SOLID_COLOR_LAYER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "cc/base/cc_export.h"
#include "cc/layers/layer_impl.h"

namespace cc {

class CC_EXPORT SolidColorLayerImpl : public LayerImpl {
 public:
  static scoped_ptr<SolidColorLayerImpl> Create(LayerTreeImpl* tree_impl,
                                                int id) {
    return make_scoped_ptr(new SolidColorLayerImpl(tree_impl, id));
  }

  static void AppendSolidQuads(
      RenderPass* render_pass,
      const OcclusionTracker<LayerImpl>& occlusion_tracker,
      SharedQuadState* shared_quad_state,
      const gfx::Size& content_bounds,
      const gfx::Transform& target_space_transform,
      SkColor color,
      AppendQuadsData* append_quads_data);

  virtual ~SolidColorLayerImpl();

  // LayerImpl overrides.
  virtual scoped_ptr<LayerImpl> CreateLayerImpl(LayerTreeImpl* tree_impl)
      OVERRIDE;
  virtual void AppendQuads(RenderPass* render_pass,
                           const OcclusionTracker<LayerImpl>& occlusion_tracker,
                           AppendQuadsData* append_quads_data) OVERRIDE;

 protected:
  SolidColorLayerImpl(LayerTreeImpl* tree_impl, int id);

 private:
  virtual const char* LayerTypeAsString() const OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(SolidColorLayerImpl);
};

}  // namespace cc

#endif  // CC_LAYERS_SOLID_COLOR_LAYER_IMPL_H_
