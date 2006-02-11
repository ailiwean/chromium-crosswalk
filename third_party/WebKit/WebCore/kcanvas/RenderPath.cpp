/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
                  2005 Eric Seidel <eric.seidel@kdemail.net>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "config.h"
#if SVG_SUPPORT
#include "IntRect.h"
#include <kdebug.h>
#include <kxmlcore/Assertions.h>

#include "kcanvas/KCanvas.h"
#include "kcanvas/RenderPath.h"
#include "KCanvasMatrix.h"
#include "KRenderingDevice.h"
#include "KCanvasContainer.h"
#include "KRenderingFillPainter.h"
#include "KRenderingStrokePainter.h"

#include "KCanvasRenderingStyle.h"
#include "SVGRenderStyle.h"
#include "SVGStyledElementImpl.h"

namespace WebCore {

class RenderPath::Private
{
public:
    RefPtr<KCanvasPath> path;

    FloatRect fillBBox, strokeBbox;
    QMatrix matrix;
};        

// RenderPath
RenderPath::RenderPath(RenderStyle *style, SVGStyledElementImpl *node) : RenderObject((DOM::NodeImpl *)node), d(new Private())
{
    ASSERT(style != 0);
}

RenderPath::~RenderPath()
{
    delete d;
}

QMatrix RenderPath::localTransform() const
{
    return d->matrix;
}

void RenderPath::setLocalTransform(const QMatrix &matrix)
{
    d->matrix = matrix;
}


FloatPoint RenderPath::mapAbsolutePointToLocal(const FloatPoint& point) const
{
    // FIXME: does it make sense to map incoming points with the inverse of the
    // absolute transform? 
    double localX;
    double localY;
    absoluteTransform().invert().map(point.x(), point.y(), &localX, &localY);
    return FloatPoint(localX, localY);
}

bool RenderPath::fillContains(const FloatPoint& point) const
{
    if (!d->path)
        return false;

    if (!KSVGPainterFactory::fillPaintServer(style(), this))
        return false;

    return path()->containsPoint(mapAbsolutePointToLocal(point),
                                 KSVGPainterFactory::fillPainter(style(), this).fillRule());
}

bool RenderPath::strokeContains(const FloatPoint& point) const
{
    if (!d->path)
        return false;

    if (!KSVGPainterFactory::strokePaintServer(style(), this))
        return false;

    return path()->strokeContainsPoint(mapAbsolutePointToLocal(point));
}

FloatRect RenderPath::strokeBBox() const
{
    if (KSVGPainterFactory::isStroked(style())) {
        KRenderingStrokePainter strokePainter = KSVGPainterFactory::strokePainter(style(), this);
        return path()->strokeBoundingBox(strokePainter);
    }

    return path()->boundingBox();
}

FloatRect RenderPath::relativeBBox(bool includeStroke) const
{
    if (!d->path)
        return FloatRect();

    if (includeStroke) {
        if (d->strokeBbox.isEmpty())
            d->strokeBbox = strokeBBox();
        return d->strokeBbox;
    }

    if (d->fillBBox.isEmpty())
        d->fillBBox = path()->boundingBox();
    return d->fillBBox;
}

void RenderPath::setPath(KCanvasPath* newPath)
{
    d->path = newPath;
}

KCanvasPath* RenderPath::path() const
{
    return d->path.get();
}

void RenderPath::layout()
{
    // FIXME: Currently the DOM does all of the % length calculations, so we
    // pretend that one of the attributes of the element has changed on the DOM
    // to force the DOM object to update this render object with new aboslute position values.

    static_cast<SVGStyledElementImpl*>(element())->notifyAttributeChange();
    IntRect layoutRect = getAbsoluteRepaintRect();
    setWidth(layoutRect.width());
    setHeight(layoutRect.height());
    setNeedsLayout(false);
}

IntRect RenderPath::getAbsoluteRepaintRect()
{
    FloatRect repaintRect = absoluteTransform().mapRect(relativeBBox(true));
    
    // Filters can expand the bounding box
    KCanvasFilter *filter = getFilterById(document(), style()->svgStyle()->filter().mid(1));
    if (filter)
        repaintRect.unite(filter->filterBBoxForItemBBox(repaintRect));
    
    if (!repaintRect.isEmpty())
        repaintRect.inflate(1); // inflate 1 pixel for antialiasing
    return enclosingIntRect(repaintRect);
}

bool RenderPath::requiresLayer()
{
    return false;
}

short RenderPath::lineHeight(bool b, bool isRootLineBox) const
{
    return static_cast<short>(relativeBBox(true).height());
}

short RenderPath::baselinePosition(bool b, bool isRootLineBox) const
{
    return static_cast<short>(relativeBBox(true).height());
}

void RenderPath::paint(PaintInfo &paintInfo, int parentX, int parentY)
{
    // No one should be transforming us via these.
    //ASSERT(parentX == 0);
    //ASSERT(parentY == 0);

    if (paintInfo.p->paintingDisabled() || (paintInfo.phase != PaintActionForeground) || style()->visibility() == HIDDEN)
        return;
    
    KRenderingDevice *renderingDevice = QPainter::renderingDevice();
    KRenderingDeviceContext *deviceContext = renderingDevice->currentContext();
    bool shouldPopContext = false;
    if (!deviceContext) {
        // I only need to setup for KCanvas rendering if it hasn't already been done.
        deviceContext = paintInfo.p->createRenderingDeviceContext();
        renderingDevice->pushContext(deviceContext);
        shouldPopContext = true;
    } else
        paintInfo.p->save();

    deviceContext->concatCTM(localTransform());

    // setup to apply filters
    KCanvasFilter *filter = getFilterById(document(), style()->svgStyle()->filter().mid(1));
    if (filter) {
        filter->prepareFilter(relativeBBox(true));
        deviceContext = renderingDevice->currentContext();
    }

    if (KCanvasClipper *clipper = getClipperById(document(), style()->svgStyle()->clipPath().mid(1)))
        clipper->applyClip(relativeBBox(true));

    if (KCanvasMasker *masker = getMaskerById(document(), style()->svgStyle()->maskElement().mid(1)))
        masker->applyMask(relativeBBox(true));

    deviceContext->clearPath();
    
    KRenderingPaintServer *fillPaintServer = KSVGPainterFactory::fillPaintServer(style(), this);
    if (fillPaintServer) {
        deviceContext->addPath(path());
        fillPaintServer->setActiveClient(this);
        fillPaintServer->draw(deviceContext, this, APPLY_TO_FILL);
    }
    KRenderingPaintServer *strokePaintServer = KSVGPainterFactory::strokePaintServer(style(), this);
    if (strokePaintServer) {
        deviceContext->addPath(path()); // path is cleared when filled.
        strokePaintServer->setActiveClient(this);
        strokePaintServer->draw(deviceContext, this, APPLY_TO_STROKE);
    }

    drawMarkersIfNeeded(paintInfo.r, path());

    // actually apply the filter
    if (filter)
        filter->applyFilter(relativeBBox(true));

    // restore drawing state
    if (shouldPopContext) {
        renderingDevice->popContext();
        delete deviceContext;
    } else
        paintInfo.p->restore();
}

bool RenderPath::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction)
{
    // We only draw in the forground phase, so we only hit-test then.
    if (hitTestAction != HitTestForeground)
        return false;

    if (strokeContains(FloatPoint(_x, _y)) || fillContains(FloatPoint(_x, _y))) {
        setInnerNode(info);
        return true;
    }
    return false;
}

}

// vim:ts=4:noet
#endif // SVG_SUPPORT

