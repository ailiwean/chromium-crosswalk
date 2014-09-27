// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/layers/video_layer_impl.h"

#include "base/bind.h"
#include "base/logging.h"
#include "cc/layers/video_frame_provider_client_impl.h"
#include "cc/quads/io_surface_draw_quad.h"
#include "cc/quads/stream_video_draw_quad.h"
#include "cc/quads/texture_draw_quad.h"
#include "cc/quads/yuv_video_draw_quad.h"
#include "cc/resources/resource_provider.h"
#include "cc/resources/single_release_callback_impl.h"
#include "cc/trees/layer_tree_impl.h"
#include "cc/trees/occlusion_tracker.h"
#include "cc/trees/proxy.h"
#include "media/base/video_frame.h"

#if defined(VIDEO_HOLE)
#include "cc/quads/solid_color_draw_quad.h"
#endif  // defined(VIDEO_HOLE)

namespace cc {

// static
scoped_ptr<VideoLayerImpl> VideoLayerImpl::Create(
    LayerTreeImpl* tree_impl,
    int id,
    VideoFrameProvider* provider,
    media::VideoRotation video_rotation) {
  scoped_ptr<VideoLayerImpl> layer(
      new VideoLayerImpl(tree_impl, id, video_rotation));
  layer->SetProviderClientImpl(VideoFrameProviderClientImpl::Create(provider));
  DCHECK(tree_impl->proxy()->IsImplThread());
  DCHECK(tree_impl->proxy()->IsMainThreadBlocked());
  return layer.Pass();
}

VideoLayerImpl::VideoLayerImpl(LayerTreeImpl* tree_impl,
                               int id,
                               media::VideoRotation video_rotation)
    : LayerImpl(tree_impl, id), frame_(NULL), video_rotation_(video_rotation) {
}

VideoLayerImpl::~VideoLayerImpl() {
  if (!provider_client_impl_->Stopped()) {
    // In impl side painting, we may have a pending and active layer
    // associated with the video provider at the same time. Both have a ref
    // on the VideoFrameProviderClientImpl, but we stop when the first
    // LayerImpl (the one on the pending tree) is destroyed since we know
    // the main thread is blocked for this commit.
    DCHECK(layer_tree_impl()->proxy()->IsImplThread());
    DCHECK(layer_tree_impl()->proxy()->IsMainThreadBlocked());
    provider_client_impl_->Stop();
  }
}

scoped_ptr<LayerImpl> VideoLayerImpl::CreateLayerImpl(
    LayerTreeImpl* tree_impl) {
  return make_scoped_ptr(new VideoLayerImpl(tree_impl, id(), video_rotation_));
}

void VideoLayerImpl::PushPropertiesTo(LayerImpl* layer) {
  LayerImpl::PushPropertiesTo(layer);

  VideoLayerImpl* other = static_cast<VideoLayerImpl*>(layer);
  other->SetProviderClientImpl(provider_client_impl_);
}

void VideoLayerImpl::DidBecomeActive() {
  provider_client_impl_->set_active_video_layer(this);
}

bool VideoLayerImpl::WillDraw(DrawMode draw_mode,
                              ResourceProvider* resource_provider) {
  if (draw_mode == DRAW_MODE_RESOURCELESS_SOFTWARE)
    return false;

  // Explicitly acquire and release the provider mutex so it can be held from
  // WillDraw to DidDraw. Since the compositor thread is in the middle of
  // drawing, the layer will not be destroyed before DidDraw is called.
  // Therefore, the only thing that will prevent this lock from being released
  // is the GPU process locking it. As the GPU process can't cause the
  // destruction of the provider (calling StopUsingProvider), holding this
  // lock should not cause a deadlock.
  frame_ = provider_client_impl_->AcquireLockAndCurrentFrame();

  if (!frame_.get()) {
    // Drop any resources used by the updater if there is no frame to display.
    updater_ = nullptr;

    provider_client_impl_->ReleaseLock();
    return false;
  }

  if (!LayerImpl::WillDraw(draw_mode, resource_provider))
    return false;

  if (!updater_) {
    updater_.reset(
        new VideoResourceUpdater(layer_tree_impl()->context_provider(),
                                 layer_tree_impl()->resource_provider()));
  }

  VideoFrameExternalResources external_resources =
      updater_->CreateExternalResourcesFromVideoFrame(frame_);
  frame_resource_type_ = external_resources.type;

  if (external_resources.type ==
      VideoFrameExternalResources::SOFTWARE_RESOURCE) {
    software_resources_ = external_resources.software_resources;
    software_release_callback_ =
        external_resources.software_release_callback;
    return true;
  }

  DCHECK_EQ(external_resources.mailboxes.size(),
            external_resources.release_callbacks.size());
  for (size_t i = 0; i < external_resources.mailboxes.size(); ++i) {
    unsigned resource_id = resource_provider->CreateResourceFromTextureMailbox(
        external_resources.mailboxes[i],
        SingleReleaseCallbackImpl::Create(
            external_resources.release_callbacks[i]));
    frame_resources_.push_back(resource_id);
  }

  return true;
}

void VideoLayerImpl::AppendQuads(
    RenderPass* render_pass,
    const OcclusionTracker<LayerImpl>& occlusion_tracker,
    AppendQuadsData* append_quads_data) {
  DCHECK(frame_.get());

  gfx::Transform transform = draw_transform();
  gfx::Size rotated_size = content_bounds();

  switch (video_rotation_) {
    case media::VIDEO_ROTATION_90:
      rotated_size = gfx::Size(rotated_size.height(), rotated_size.width());
      transform.Rotate(90.0);
      transform.Translate(0.0, -rotated_size.height());
      break;
    case media::VIDEO_ROTATION_180:
      transform.Rotate(180.0);
      transform.Translate(-rotated_size.width(), -rotated_size.height());
      break;
    case media::VIDEO_ROTATION_270:
      rotated_size = gfx::Size(rotated_size.height(), rotated_size.width());
      transform.Rotate(270.0);
      transform.Translate(-rotated_size.width(), 0);
    case media::VIDEO_ROTATION_0:
      break;
  }

  SharedQuadState* shared_quad_state =
      render_pass->CreateAndAppendSharedQuadState();
  shared_quad_state->SetAll(transform,
                            rotated_size,
                            visible_content_rect(),
                            clip_rect(),
                            is_clipped(),
                            draw_opacity(),
                            blend_mode(),
                            sorting_context_id());

  AppendDebugBorderQuad(
      render_pass, rotated_size, shared_quad_state, append_quads_data);

  gfx::Rect quad_rect(rotated_size);
  gfx::Rect opaque_rect(contents_opaque() ? quad_rect : gfx::Rect());
  gfx::Rect visible_rect = frame_->visible_rect();
  gfx::Size coded_size = frame_->coded_size();

  gfx::Rect visible_quad_rect =
      occlusion_tracker.GetCurrentOcclusionForLayer(transform)
          .GetUnoccludedContentRect(quad_rect);
  if (visible_quad_rect.IsEmpty())
    return;

  // Pixels for macroblocked formats.
  const float tex_width_scale =
      static_cast<float>(visible_rect.width()) / coded_size.width();
  const float tex_height_scale =
      static_cast<float>(visible_rect.height()) / coded_size.height();
  const float tex_x_offset =
      static_cast<float>(visible_rect.x()) / coded_size.width();
  const float tex_y_offset =
      static_cast<float>(visible_rect.y()) / coded_size.height();

  switch (frame_resource_type_) {
    // TODO(danakj): Remove this, hide it in the hardware path.
    case VideoFrameExternalResources::SOFTWARE_RESOURCE: {
      DCHECK_EQ(frame_resources_.size(), 0u);
      DCHECK_EQ(software_resources_.size(), 1u);
      if (software_resources_.size() < 1u)
        break;
      bool premultiplied_alpha = true;
      gfx::PointF uv_top_left(0.f, 0.f);
      gfx::PointF uv_bottom_right(tex_width_scale, tex_height_scale);
      float opacity[] = {1.0f, 1.0f, 1.0f, 1.0f};
      bool flipped = false;
      TextureDrawQuad* texture_quad =
          render_pass->CreateAndAppendDrawQuad<TextureDrawQuad>();
      texture_quad->SetNew(shared_quad_state,
                           quad_rect,
                           opaque_rect,
                           visible_quad_rect,
                           software_resources_[0],
                           premultiplied_alpha,
                           uv_top_left,
                           uv_bottom_right,
                           SK_ColorTRANSPARENT,
                           opacity,
                           flipped);
      break;
    }
    case VideoFrameExternalResources::YUV_RESOURCE: {
      DCHECK_GE(frame_resources_.size(), 3u);
      if (frame_resources_.size() < 3u)
        break;
      YUVVideoDrawQuad::ColorSpace color_space =
          frame_->format() == media::VideoFrame::YV12J
              ? YUVVideoDrawQuad::REC_601_JPEG
              : YUVVideoDrawQuad::REC_601;
      gfx::RectF tex_coord_rect(
          tex_x_offset, tex_y_offset, tex_width_scale, tex_height_scale);
      YUVVideoDrawQuad* yuv_video_quad =
          render_pass->CreateAndAppendDrawQuad<YUVVideoDrawQuad>();
      yuv_video_quad->SetNew(
          shared_quad_state,
          quad_rect,
          opaque_rect,
          visible_quad_rect,
          tex_coord_rect,
          frame_resources_[0],
          frame_resources_[1],
          frame_resources_[2],
          frame_resources_.size() > 3 ? frame_resources_[3] : 0,
          color_space);
      break;
    }
    case VideoFrameExternalResources::RGB_RESOURCE: {
      DCHECK_EQ(frame_resources_.size(), 1u);
      if (frame_resources_.size() < 1u)
        break;
      bool premultiplied_alpha = true;
      gfx::PointF uv_top_left(0.f, 0.f);
      gfx::PointF uv_bottom_right(tex_width_scale, tex_height_scale);
      float opacity[] = {1.0f, 1.0f, 1.0f, 1.0f};
      bool flipped = false;
      TextureDrawQuad* texture_quad =
          render_pass->CreateAndAppendDrawQuad<TextureDrawQuad>();
      texture_quad->SetNew(shared_quad_state,
                           quad_rect,
                           opaque_rect,
                           visible_quad_rect,
                           frame_resources_[0],
                           premultiplied_alpha,
                           uv_top_left,
                           uv_bottom_right,
                           SK_ColorTRANSPARENT,
                           opacity,
                           flipped);
      break;
    }
    case VideoFrameExternalResources::STREAM_TEXTURE_RESOURCE: {
      DCHECK_EQ(frame_resources_.size(), 1u);
      if (frame_resources_.size() < 1u)
        break;
      gfx::Transform scale;
      scale.Scale(tex_width_scale, tex_height_scale);
      StreamVideoDrawQuad* stream_video_quad =
          render_pass->CreateAndAppendDrawQuad<StreamVideoDrawQuad>();
      stream_video_quad->SetNew(
          shared_quad_state,
          quad_rect,
          opaque_rect,
          visible_quad_rect,
          frame_resources_[0],
          scale * provider_client_impl_->stream_texture_matrix());
      break;
    }
    case VideoFrameExternalResources::IO_SURFACE: {
      DCHECK_EQ(frame_resources_.size(), 1u);
      if (frame_resources_.size() < 1u)
        break;
      IOSurfaceDrawQuad* io_surface_quad =
          render_pass->CreateAndAppendDrawQuad<IOSurfaceDrawQuad>();
      io_surface_quad->SetNew(shared_quad_state,
                              quad_rect,
                              opaque_rect,
                              visible_quad_rect,
                              visible_rect.size(),
                              frame_resources_[0],
                              IOSurfaceDrawQuad::UNFLIPPED);
      break;
    }
#if defined(VIDEO_HOLE)
    // This block and other blocks wrapped around #if defined(GOOGLE_TV) is not
    // maintained by the general compositor team. Please contact the following
    // people instead:
    //
    // wonsik@chromium.org
    // ycheo@chromium.org
    case VideoFrameExternalResources::HOLE: {
      DCHECK_EQ(frame_resources_.size(), 0u);
      SolidColorDrawQuad* solid_color_draw_quad =
          render_pass->CreateAndAppendDrawQuad<SolidColorDrawQuad>();

      // Create a solid color quad with transparent black and force no
      // blending / no anti-aliasing.
      gfx::Rect opaque_rect = quad_rect;
      solid_color_draw_quad->SetAll(shared_quad_state,
                                    quad_rect,
                                    opaque_rect,
                                    visible_quad_rect,
                                    false,
                                    SK_ColorTRANSPARENT,
                                    true);
      break;
    }
#endif  // defined(VIDEO_HOLE)
    case VideoFrameExternalResources::NONE:
      NOTIMPLEMENTED();
      break;
  }
}

void VideoLayerImpl::DidDraw(ResourceProvider* resource_provider) {
  LayerImpl::DidDraw(resource_provider);

  DCHECK(frame_.get());

  if (frame_resource_type_ ==
      VideoFrameExternalResources::SOFTWARE_RESOURCE) {
    for (size_t i = 0; i < software_resources_.size(); ++i) {
      software_release_callback_.Run(
          0, false, layer_tree_impl()->BlockingMainThreadTaskRunner());
    }

    software_resources_.clear();
    software_release_callback_.Reset();
  } else {
    for (size_t i = 0; i < frame_resources_.size(); ++i)
      resource_provider->DeleteResource(frame_resources_[i]);
    frame_resources_.clear();
  }

  provider_client_impl_->PutCurrentFrame(frame_);
  frame_ = NULL;

  provider_client_impl_->ReleaseLock();
}

void VideoLayerImpl::ReleaseResources() {
  updater_ = nullptr;
}

void VideoLayerImpl::SetNeedsRedraw() {
  SetUpdateRect(gfx::UnionRects(update_rect(), gfx::RectF(bounds())));
  layer_tree_impl()->SetNeedsRedraw();
}

void VideoLayerImpl::SetProviderClientImpl(
    scoped_refptr<VideoFrameProviderClientImpl> provider_client_impl) {
  provider_client_impl_ = provider_client_impl;
}

const char* VideoLayerImpl::LayerTypeAsString() const {
  return "cc::VideoLayerImpl";
}

}  // namespace cc
