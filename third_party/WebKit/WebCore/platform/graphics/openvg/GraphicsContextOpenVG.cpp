/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
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
#include "GraphicsContext.h"

#include "GraphicsContextPrivate.h"
#include "NotImplemented.h"
#include "PainterOpenVG.h"
#include "SurfaceOpenVG.h"
#include "TransformationMatrix.h"

#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>
#include <wtf/UnusedParam.h>
#include <wtf/Vector.h>

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include <egl.h>
#endif

namespace WebCore {

// typedef'ing doesn't work, let's inherit from PainterOpenVG instead
class GraphicsContextPlatformPrivate : public PainterOpenVG {
public:
    GraphicsContextPlatformPrivate(SurfaceOpenVG* surface)
        : PainterOpenVG(surface)
    {
    }
};

GraphicsContext::GraphicsContext(SurfaceOpenVG* surface)
    : m_common(createGraphicsContextPrivate())
    , m_data(surface ? new GraphicsContextPlatformPrivate(surface) : 0)
{
    setPaintingDisabled(!surface);
}

GraphicsContext::~GraphicsContext()
{
    destroyGraphicsContextPrivate(m_common);
    delete m_data;
}

PlatformGraphicsContext* GraphicsContext::platformContext() const
{
    if (paintingDisabled())
        return 0;

    return m_data->baseSurface();
}

TransformationMatrix GraphicsContext::getCTM() const
{
    if (paintingDisabled())
        return TransformationMatrix();

    return m_data->transformationMatrix();
}

void GraphicsContext::savePlatformState()
{
    if (paintingDisabled())
        return;

    m_data->save();
}

void GraphicsContext::restorePlatformState()
{
    if (paintingDisabled())
        return;

    m_data->restore();
}

void GraphicsContext::drawRect(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawRect(rect);
}

void GraphicsContext::drawLine(const IntPoint& from, const IntPoint& to)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(from);
    UNUSED_PARAM(to);
}

/**
 * Draw the largest ellipse that fits into the given rectangle.
 */
void GraphicsContext::drawEllipse(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
}

void GraphicsContext::strokeArc(const IntRect& rect, int startAngle, int angleSpan)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
    UNUSED_PARAM(startAngle);
    UNUSED_PARAM(angleSpan);
}

void GraphicsContext::drawConvexPolygon(size_t numPoints, const FloatPoint* points, bool shouldAntialias)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(numPoints);
    UNUSED_PARAM(points);
    UNUSED_PARAM(shouldAntialias);
}

void GraphicsContext::fillPath()
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::strokePath()
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::fillRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawRect(rect, VG_FILL_PATH);
}

void GraphicsContext::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    PainterOpenVG* painter = m_data;
    Color oldColor = painter->fillColor();
    painter->setFillColor(color);
    painter->drawRect(rect, VG_FILL_PATH);
    painter->setFillColor(oldColor);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::fillRoundedRect(const IntRect& rect, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
    UNUSED_PARAM(topLeft);
    UNUSED_PARAM(topRight);
    UNUSED_PARAM(bottomLeft);
    UNUSED_PARAM(bottomRight);
    UNUSED_PARAM(color);
    UNUSED_PARAM(colorSpace);
}

void GraphicsContext::beginPath()
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::addPath(const Path& path)
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::clip(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->intersectClipRect(rect);
}

void GraphicsContext::clipPath(WindRule clipRule)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(clipRule);
}

void GraphicsContext::drawFocusRing(const Vector<IntRect>& rects, int width, int offset, const Color& color)
{
    if (paintingDisabled())
        return;

    if (rects.isEmpty())
        return;

    // FIXME: We just unite all focus ring rects into one for now.
    // We should outline the edge of the full region.
    offset += (width - 1) / 2;
    IntRect finalFocusRect;

    for (unsigned i = 0; i < rects.size(); i++) {
        IntRect focusRect = rects[i];
        focusRect.inflate(offset);
        finalFocusRect.unite(focusRect);
    }

    PainterOpenVG* painter = m_data;
    StrokeStyle oldStyle = painter->strokeStyle();
    Color oldStrokeColor = painter->strokeColor();
    painter->setStrokeStyle(DashedStroke);
    painter->setStrokeColor(color);
    strokeRect(FloatRect(finalFocusRect), 1.f);
    painter->setStrokeStyle(oldStyle);
    painter->setStrokeColor(oldStrokeColor);
}

void GraphicsContext::drawLineForText(const IntPoint& origin, int width, bool printing)
{
    if (paintingDisabled())
        return;

    if (width <= 0)
        return;

    notImplemented();
    UNUSED_PARAM(origin);
    UNUSED_PARAM(width);
    UNUSED_PARAM(printing);
}

void GraphicsContext::drawLineForMisspellingOrBadGrammar(const IntPoint& origin, int width, bool grammar)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(origin);
    UNUSED_PARAM(width);
    UNUSED_PARAM(grammar);
}

FloatRect GraphicsContext::roundToDevicePixels(const FloatRect& rect)
{
    if (paintingDisabled())
        return FloatRect();

    return FloatRect(enclosingIntRect(m_data->transformationMatrix().mapRect(rect)));
}

void GraphicsContext::setPlatformShadow(const IntSize& size, int blur, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(size);
    UNUSED_PARAM(blur);
    UNUSED_PARAM(color);
    UNUSED_PARAM(colorSpace);
}

void GraphicsContext::clearPlatformShadow()
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::beginTransparencyLayer(float opacity)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(opacity);
}

void GraphicsContext::endTransparencyLayer()
{
    if (paintingDisabled())
        return;

    notImplemented();
}

void GraphicsContext::clearRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    PainterOpenVG* painter = m_data;

    CompositeOperator op = painter->compositeOperation();
    painter->setCompositeOperation(CompositeClear);
    painter->drawRect(rect, VG_FILL_PATH);
    painter->setCompositeOperation(op);
}

void GraphicsContext::strokeRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    m_data->drawRect(rect, VG_STROKE_PATH);
}

void GraphicsContext::strokeRect(const FloatRect& rect, float lineWidth)
{
    if (paintingDisabled())
        return;

    PainterOpenVG* painter = m_data;

    float oldThickness = painter->strokeThickness();
    painter->setStrokeThickness(lineWidth);
    painter->drawRect(rect, VG_STROKE_PATH);
    painter->setStrokeThickness(oldThickness);
}

void GraphicsContext::setLineCap(LineCap lc)
{
    if (paintingDisabled())
        return;

    m_data->setLineCap(lc);
}

void GraphicsContext::setLineDash(const DashArray& dashes, float dashOffset)
{
    if (paintingDisabled())
        return;

    m_data->setLineDash(dashes, dashOffset);
}

void GraphicsContext::setLineJoin(LineJoin lj)
{
    if (paintingDisabled())
        return;

    m_data->setLineJoin(lj);
}

void GraphicsContext::setMiterLimit(float limit)
{
    if (paintingDisabled())
        return;

    m_data->setMiterLimit(limit);
}

void GraphicsContext::setAlpha(float opacity)
{
    if (paintingDisabled())
        return;

    m_data->setOpacity(opacity);
}

void GraphicsContext::setCompositeOperation(CompositeOperator op)
{
    if (paintingDisabled())
        return;

    m_data->setCompositeOperation(op);
}

void GraphicsContext::clip(const Path& path)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(path);
}

void GraphicsContext::canvasClip(const Path& path)
{
    clip(path);
}

void GraphicsContext::clipOut(const Path& path)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(path);
}

void GraphicsContext::scale(const FloatSize& scaleFactors)
{
    if (paintingDisabled())
        return;

    m_data->scale(scaleFactors);
}

void GraphicsContext::rotate(float radians)
{
    if (paintingDisabled())
        return;

    m_data->rotate(radians);
}

void GraphicsContext::translate(float dx, float dy)
{
    if (paintingDisabled())
        return;

    m_data->translate(dx, dy);
}

IntPoint GraphicsContext::origin()
{
    if (paintingDisabled())
        return IntPoint();

    TransformationMatrix matrix = m_data->transformationMatrix();
    return IntPoint(roundf(matrix.m41()), roundf(matrix.m42()));
}

void GraphicsContext::clipOut(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
}

void GraphicsContext::clipOutEllipseInRect(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
}

void GraphicsContext::clipToImageBuffer(const FloatRect& rect, const ImageBuffer* imageBuffer)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
    UNUSED_PARAM(imageBuffer);
}

void GraphicsContext::addInnerRoundedRectClip(const IntRect& rect, int thickness)
{
    if (paintingDisabled())
        return;

    notImplemented();
    UNUSED_PARAM(rect);
    UNUSED_PARAM(thickness);
}

void GraphicsContext::concatCTM(const TransformationMatrix& transform)
{
    if (paintingDisabled())
        return;

    m_data->concatTransformationMatrix(transform);
}

void GraphicsContext::setURLForRect(const KURL& link, const IntRect& destRect)
{
    notImplemented();
    UNUSED_PARAM(link);
    UNUSED_PARAM(destRect);
}

void GraphicsContext::setPlatformStrokeColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeColor(color);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::setPlatformStrokeStyle(const StrokeStyle& strokeStyle)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeStyle(strokeStyle);
}

void GraphicsContext::setPlatformStrokeThickness(float thickness)
{
    if (paintingDisabled())
        return;

    m_data->setStrokeThickness(thickness);
}

void GraphicsContext::setPlatformFillColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    m_data->setFillColor(color);

    UNUSED_PARAM(colorSpace); // FIXME
}

void GraphicsContext::setPlatformShouldAntialias(bool enable)
{
    if (paintingDisabled())
        return;

    m_data->setAntialiasingEnabled(enable);
}

void GraphicsContext::setImageInterpolationQuality(InterpolationQuality)
{
    notImplemented();
}

InterpolationQuality GraphicsContext::imageInterpolationQuality() const
{
    notImplemented();
    return InterpolationDefault;
}

}
