/*
 * Copyright (C) 2006 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007, 2008, 2009 Rob Buis <buis@kde.org>
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "core/rendering/svg/RenderSVGImage.h"

#include "core/platform/graphics/GraphicsContextStateSaver.h"
#include "core/rendering/ImageQualityController.h"
#include "core/rendering/LayoutRepainter.h"
#include "core/rendering/PointerEventsHitRules.h"
#include "core/rendering/RenderImageResource.h"
#include "core/rendering/svg/RenderSVGResource.h"
#include "core/rendering/svg/SVGRenderingContext.h"
#include "core/rendering/svg/SVGResources.h"
#include "core/rendering/svg/SVGResourcesCache.h"
#include "core/svg/SVGImageElement.h"

namespace WebCore {

RenderSVGImage::RenderSVGImage(SVGImageElement* impl)
    : RenderSVGModelObject(impl)
    , m_needsBoundariesUpdate(true)
    , m_needsTransformUpdate(true)
    , m_imageResource(RenderImageResource::create())
{
    m_imageResource->initialize(this);
}

RenderSVGImage::~RenderSVGImage()
{
    ImageQualityController::remove(this);
    m_imageResource->shutdown();
}

bool RenderSVGImage::updateImageViewport()
{
    SVGImageElement* image = static_cast<SVGImageElement*>(node());
    FloatRect oldBoundaries = m_objectBoundingBox;
    bool updatedViewport = false;

    SVGLengthContext lengthContext(image);
    m_objectBoundingBox = FloatRect(image->x().value(lengthContext), image->y().value(lengthContext), image->width().value(lengthContext), image->height().value(lengthContext));

    // Images with preserveAspectRatio=none should force non-uniform scaling. This can be achieved
    // by setting the image's container size to its intrinsic size.
    // See: http://www.w3.org/TR/SVG/single-page.html, 7.8 The ‘preserveAspectRatio’ attribute.
    if (image->preserveAspectRatio().align() == SVGPreserveAspectRatio::SVG_PRESERVEASPECTRATIO_NONE) {
        if (CachedImage* cachedImage = m_imageResource->cachedImage()) {
            LayoutSize intrinsicSize = cachedImage->imageSizeForRenderer(0, style()->effectiveZoom());
            if (intrinsicSize != m_imageResource->imageSize(style()->effectiveZoom())) {
                m_imageResource->setContainerSizeForRenderer(roundedIntSize(intrinsicSize));
                updatedViewport = true;
            }
        }
    }

    if (oldBoundaries != m_objectBoundingBox) {
        if (!updatedViewport)
            m_imageResource->setContainerSizeForRenderer(enclosingIntRect(m_objectBoundingBox).size());
        updatedViewport = true;
        m_needsBoundariesUpdate = true;
    }

    return updatedViewport;
}

void RenderSVGImage::layout()
{
    StackStats::LayoutCheckPoint layoutCheckPoint;
    ASSERT(needsLayout());

    LayoutRepainter repainter(*this, SVGRenderSupport::checkForSVGRepaintDuringLayout(this) && selfNeedsLayout());
    updateImageViewport();

    bool transformOrBoundariesUpdate = m_needsTransformUpdate || m_needsBoundariesUpdate;
    if (m_needsTransformUpdate) {
        m_localTransform = static_cast<SVGImageElement*>(node())->animatedLocalTransform();
        m_needsTransformUpdate = false;
    }

    if (m_needsBoundariesUpdate) {
        m_repaintBoundingBoxExcludingShadow = m_objectBoundingBox;
        SVGRenderSupport::intersectRepaintRectWithResources(this, m_repaintBoundingBoxExcludingShadow);

        m_repaintBoundingBox = m_repaintBoundingBoxExcludingShadow;
        SVGRenderSupport::intersectRepaintRectWithShadows(this, m_repaintBoundingBox);

        m_needsBoundariesUpdate = false;
    }

    // Invalidate all resources of this client if our layout changed.
    if (everHadLayout() && selfNeedsLayout())
        SVGResourcesCache::clientLayoutChanged(this);

    // If our bounds changed, notify the parents.
    if (transformOrBoundariesUpdate)
        RenderSVGModelObject::setNeedsBoundariesUpdate();

    repainter.repaintAfterLayout();
    setNeedsLayout(false);
}

void RenderSVGImage::paint(PaintInfo& paintInfo, const LayoutPoint&)
{
    ANNOTATE_GRAPHICS_CONTEXT(paintInfo, this);

    if (paintInfo.context->paintingDisabled() || style()->visibility() == HIDDEN || !m_imageResource->hasImage())
        return;

    FloatRect boundingBox = repaintRectInLocalCoordinates();
    if (!SVGRenderSupport::paintInfoIntersectsRepaintRect(boundingBox, m_localTransform, paintInfo))
        return;

    PaintInfo childPaintInfo(paintInfo);
    bool drawsOutline = style()->outlineWidth() && (childPaintInfo.phase == PaintPhaseOutline || childPaintInfo.phase == PaintPhaseSelfOutline);
    if (drawsOutline || childPaintInfo.phase == PaintPhaseForeground) {
        GraphicsContextStateSaver stateSaver(*childPaintInfo.context);
        childPaintInfo.applyTransform(m_localTransform);

        if (childPaintInfo.phase == PaintPhaseForeground) {
            SVGRenderingContext renderingContext(this, childPaintInfo);

            if (renderingContext.isRenderingPrepared()) {
                if (style()->svgStyle()->bufferedRendering() == BR_STATIC  && renderingContext.bufferForeground(m_bufferedForeground))
                    return;

                paintForeground(childPaintInfo);
            }
        }

        if (drawsOutline)
            paintOutline(childPaintInfo, IntRect(boundingBox));
    }
}

void RenderSVGImage::paintForeground(PaintInfo& paintInfo)
{
    RefPtr<Image> image = m_imageResource->image();
    FloatRect destRect = m_objectBoundingBox;
    FloatRect srcRect(0, 0, image->width(), image->height());

    SVGImageElement* imageElement = static_cast<SVGImageElement*>(node());
    imageElement->preserveAspectRatio().transformRect(destRect, srcRect);

    bool useLowQualityScaling = false;
    if (style()->svgStyle()->bufferedRendering() != BR_STATIC)
        useLowQualityScaling = ImageQualityController::imageQualityController()->shouldPaintAtLowQuality(paintInfo.context, this, image.get(), image.get(), LayoutSize(destRect.size()));

    paintInfo.context->drawImage(image.get(), style()->colorSpace(), destRect, srcRect, CompositeSourceOver, DoNotRespectImageOrientation, useLowQualityScaling);
}

void RenderSVGImage::invalidateBufferedForeground()
{
    m_bufferedForeground.clear();
}

bool RenderSVGImage::nodeAtFloatPoint(const HitTestRequest& request, HitTestResult& result, const FloatPoint& pointInParent, HitTestAction hitTestAction)
{
    // We only draw in the forground phase, so we only hit-test then.
    if (hitTestAction != HitTestForeground)
        return false;

    PointerEventsHitRules hitRules(PointerEventsHitRules::SVG_IMAGE_HITTESTING, request, style()->pointerEvents());
    bool isVisible = (style()->visibility() == VISIBLE);
    if (isVisible || !hitRules.requireVisible) {
        FloatPoint localPoint = localToParentTransform().inverse().mapPoint(pointInParent);
            
        if (!SVGRenderSupport::pointInClippingArea(this, localPoint))
            return false;

        if (hitRules.canHitFill) {
            if (m_objectBoundingBox.contains(localPoint)) {
                updateHitTestResult(result, roundedLayoutPoint(localPoint));
                return true;
            }
        }
    }

    return false;
}

void RenderSVGImage::imageChanged(WrappedImagePtr, const IntRect*)
{
    // The image resource defaults to nullImage until the resource arrives.
    // This empty image may be cached by SVG resources which must be invalidated.
    if (SVGResources* resources = SVGResourcesCache::cachedResourcesForRenderObject(this))
        resources->removeClientFromCache(this);

    // Eventually notify parent resources, that we've changed.
    RenderSVGResource::markForLayoutAndParentResourceInvalidation(this, false);

    // Update the SVGImageCache sizeAndScales entry in case image loading finished after layout.
    // (https://bugs.webkit.org/show_bug.cgi?id=99489)
    m_objectBoundingBox = FloatRect();
    updateImageViewport();

    invalidateBufferedForeground();

    repaint();
}

void RenderSVGImage::addFocusRingRects(Vector<IntRect>& rects, const LayoutPoint&, const RenderLayerModelObject*)
{
    // this is called from paint() after the localTransform has already been applied
    IntRect contentRect = enclosingIntRect(repaintRectInLocalCoordinates());
    if (!contentRect.isEmpty())
        rects.append(contentRect);
}

} // namespace WebCore
