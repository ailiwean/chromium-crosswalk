/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "platform/graphics/GraphicsLayer.h"

#include "SkImageFilter.h"
#include "SkMatrix44.h"
#include "platform/geometry/FloatRect.h"
#include "platform/geometry/LayoutRect.h"
#include "platform/graphics/GraphicsLayerFactory.h"
#include "platform/graphics/filters/SkiaImageFilterBuilder.h"
#include "platform/graphics/skia/NativeImageSkia.h"
#include "platform/scroll/ScrollableArea.h"
#include "platform/text/TextStream.h"
#include "public/platform/Platform.h"
#include "public/platform/WebAnimation.h"
#include "public/platform/WebCompositorSupport.h"
#include "public/platform/WebFilterOperations.h"
#include "public/platform/WebFloatPoint.h"
#include "public/platform/WebFloatRect.h"
#include "public/platform/WebGraphicsLayerDebugInfo.h"
#include "public/platform/WebLayer.h"
#include "public/platform/WebPoint.h"
#include "public/platform/WebSize.h"
#include "wtf/CurrentTime.h"
#include "wtf/HashMap.h"
#include "wtf/HashSet.h"
#include "wtf/text/WTFString.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

using blink::Platform;
using blink::WebAnimation;
using blink::WebFilterOperations;
using blink::WebLayer;
using blink::WebPoint;

namespace WebCore {

typedef HashMap<const GraphicsLayer*, Vector<FloatRect> > RepaintMap;
static RepaintMap& repaintRectMap()
{
    DEFINE_STATIC_LOCAL(RepaintMap, map, ());
    return map;
}

PassOwnPtr<GraphicsLayer> GraphicsLayer::create(GraphicsLayerFactory* factory, GraphicsLayerClient* client)
{
    return factory->createGraphicsLayer(client);
}

GraphicsLayer::GraphicsLayer(GraphicsLayerClient* client)
    : m_client(client)
    , m_anchorPoint(0.5f, 0.5f, 0)
    , m_opacity(1)
    , m_zPosition(0)
    , m_blendMode(blink::WebBlendModeNormal)
    , m_contentsOpaque(false)
    , m_preserves3D(false)
    , m_backfaceVisibility(true)
    , m_masksToBounds(false)
    , m_drawsContent(false)
    , m_contentsVisible(true)
    , m_isRootForIsolatedGroup(false)
    , m_hasScrollParent(false)
    , m_hasClipParent(false)
    , m_paintingPhase(GraphicsLayerPaintAllWithOverflowClip)
    , m_contentsOrientation(CompositingCoordinatesTopDown)
    , m_parent(0)
    , m_maskLayer(0)
    , m_contentsClippingMaskLayer(0)
    , m_replicaLayer(0)
    , m_replicatedLayer(0)
    , m_paintCount(0)
    , m_contentsLayer(0)
    , m_contentsLayerId(0)
    , m_scrollableArea(0)
{
#ifndef NDEBUG
    if (m_client)
        m_client->verifyNotPainting();
#endif

    m_opaqueRectTrackingContentLayerDelegate = adoptPtr(new OpaqueRectTrackingContentLayerDelegate(this));
    m_layer = adoptPtr(Platform::current()->compositorSupport()->createContentLayer(m_opaqueRectTrackingContentLayerDelegate.get()));
    m_layer->layer()->setDrawsContent(m_drawsContent && m_contentsVisible);
    m_layer->layer()->setWebLayerClient(this);
    m_layer->setAutomaticallyComputeRasterScale(true);
}

GraphicsLayer::~GraphicsLayer()
{
    for (size_t i = 0; i < m_linkHighlights.size(); ++i)
        m_linkHighlights[i]->clearCurrentGraphicsLayer();
    m_linkHighlights.clear();

#ifndef NDEBUG
    if (m_client)
        m_client->verifyNotPainting();
#endif

    if (m_replicaLayer)
        m_replicaLayer->setReplicatedLayer(0);

    if (m_replicatedLayer)
        m_replicatedLayer->setReplicatedByLayer(0);

    removeAllChildren();
    removeFromParent();

    resetTrackedRepaints();
    ASSERT(!m_parent);
}

void GraphicsLayer::setParent(GraphicsLayer* layer)
{
    ASSERT(!layer || !layer->hasAncestor(this));
    m_parent = layer;
}

bool GraphicsLayer::hasAncestor(GraphicsLayer* ancestor) const
{
    for (GraphicsLayer* curr = parent(); curr; curr = curr->parent()) {
        if (curr == ancestor)
            return true;
    }

    return false;
}

bool GraphicsLayer::setChildren(const Vector<GraphicsLayer*>& newChildren)
{
    // If the contents of the arrays are the same, nothing to do.
    if (newChildren == m_children)
        return false;

    removeAllChildren();

    size_t listSize = newChildren.size();
    for (size_t i = 0; i < listSize; ++i)
        addChildInternal(newChildren[i]);

    updateChildList();

    return true;
}

void GraphicsLayer::addChildInternal(GraphicsLayer* childLayer)
{
    ASSERT(childLayer != this);

    if (childLayer->parent())
        childLayer->removeFromParent();

    childLayer->setParent(this);
    m_children.append(childLayer);

    // Don't call updateChildList here, this function is used in cases where it
    // should not be called until all children are processed.
}

void GraphicsLayer::addChild(GraphicsLayer* childLayer)
{
    addChildInternal(childLayer);
    updateChildList();
}

void GraphicsLayer::addChildAtIndex(GraphicsLayer* childLayer, int index)
{
    ASSERT(childLayer != this);

    if (childLayer->parent())
        childLayer->removeFromParent();

    childLayer->setParent(this);
    m_children.insert(index, childLayer);

    updateChildList();
}

void GraphicsLayer::addChildBelow(GraphicsLayer* childLayer, GraphicsLayer* sibling)
{
    ASSERT(childLayer != this);
    childLayer->removeFromParent();

    bool found = false;
    for (unsigned i = 0; i < m_children.size(); i++) {
        if (sibling == m_children[i]) {
            m_children.insert(i, childLayer);
            found = true;
            break;
        }
    }

    childLayer->setParent(this);

    if (!found)
        m_children.append(childLayer);

    updateChildList();
}

void GraphicsLayer::addChildAbove(GraphicsLayer* childLayer, GraphicsLayer* sibling)
{
    childLayer->removeFromParent();
    ASSERT(childLayer != this);

    bool found = false;
    for (unsigned i = 0; i < m_children.size(); i++) {
        if (sibling == m_children[i]) {
            m_children.insert(i+1, childLayer);
            found = true;
            break;
        }
    }

    childLayer->setParent(this);

    if (!found)
        m_children.append(childLayer);

    updateChildList();
}

bool GraphicsLayer::replaceChild(GraphicsLayer* oldChild, GraphicsLayer* newChild)
{
    ASSERT(!newChild->parent());
    bool found = false;
    for (unsigned i = 0; i < m_children.size(); i++) {
        if (oldChild == m_children[i]) {
            m_children[i] = newChild;
            found = true;
            break;
        }
    }

    if (found) {
        oldChild->setParent(0);

        newChild->removeFromParent();
        newChild->setParent(this);

        updateChildList();
        return true;
    }

    return false;
}

void GraphicsLayer::removeAllChildren()
{
    while (m_children.size()) {
        GraphicsLayer* curLayer = m_children[0];
        ASSERT(curLayer->parent());
        curLayer->removeFromParent();
    }
}

void GraphicsLayer::removeFromParent()
{
    if (m_parent) {
        unsigned i;
        for (i = 0; i < m_parent->m_children.size(); i++) {
            if (this == m_parent->m_children[i]) {
                m_parent->m_children.remove(i);
                break;
            }
        }

        setParent(0);
    }

    platformLayer()->removeFromParent();
}

void GraphicsLayer::setReplicatedByLayer(GraphicsLayer* layer)
{
    // FIXME: this could probably be a full early exit.
    if (m_replicaLayer != layer) {
        if (m_replicaLayer)
            m_replicaLayer->setReplicatedLayer(0);

        if (layer)
            layer->setReplicatedLayer(this);

        m_replicaLayer = layer;
    }

    WebLayer* webReplicaLayer = layer ? layer->platformLayer() : 0;
    platformLayer()->setReplicaLayer(webReplicaLayer);
}

void GraphicsLayer::setOffsetFromRenderer(const IntSize& offset, ShouldSetNeedsDisplay shouldSetNeedsDisplay)
{
    if (offset == m_offsetFromRenderer)
        return;

    m_offsetFromRenderer = offset;

    // If the compositing layer offset changes, we need to repaint.
    if (shouldSetNeedsDisplay == SetNeedsDisplay)
        setNeedsDisplay();
}

void GraphicsLayer::paintGraphicsLayerContents(GraphicsContext& context, const IntRect& clip)
{
    if (!m_client)
        return;
    incrementPaintCount();
    m_client->paintContents(this, context, m_paintingPhase, clip);
}

void GraphicsLayer::setZPosition(float position)
{
    m_zPosition = position;
}

float GraphicsLayer::accumulatedOpacity() const
{
    if (!preserves3D())
        return 1;

    return m_opacity * (parent() ? parent()->accumulatedOpacity() : 1);
}

void GraphicsLayer::distributeOpacity(float accumulatedOpacity)
{
    // If this is a transform layer we need to distribute our opacity to all our children

    // Incoming accumulatedOpacity is the contribution from our parent(s). We mutiply this by our own
    // opacity to get the total contribution
    accumulatedOpacity *= m_opacity;

    if (preserves3D()) {
        size_t numChildren = children().size();
        for (size_t i = 0; i < numChildren; ++i)
            children()[i]->distributeOpacity(accumulatedOpacity);
    }
}

void GraphicsLayer::updateChildList()
{
    WebLayer* childHost = m_layer->layer();
    childHost->removeAllChildren();

    clearContentsLayerIfUnregistered();

    if (m_contentsLayer) {
        // FIXME: add the contents layer in the correct order with negative z-order children.
        // This does not cause visible rendering issues because currently contents layers are only used
        // for replaced elements that don't have children.
        childHost->addChild(m_contentsLayer);
    }

    const Vector<GraphicsLayer*>& childLayers = children();
    size_t numChildren = childLayers.size();
    for (size_t i = 0; i < numChildren; ++i) {
        GraphicsLayer* curChild = childLayers[i];

        childHost->addChild(curChild->platformLayer());
    }

    for (size_t i = 0; i < m_linkHighlights.size(); ++i)
        childHost->addChild(m_linkHighlights[i]->layer());
}

void GraphicsLayer::updateLayerIsDrawable()
{
    // For the rest of the accelerated compositor code, there is no reason to make a
    // distinction between drawsContent and contentsVisible. So, for m_layer->layer(), these two
    // flags are combined here. m_contentsLayer shouldn't receive the drawsContent flag
    // so it is only given contentsVisible.

    m_layer->layer()->setDrawsContent(m_drawsContent && m_contentsVisible);
    if (WebLayer* contentsLayer = contentsLayerIfRegistered())
        contentsLayer->setDrawsContent(m_contentsVisible);

    if (m_drawsContent) {
        m_layer->layer()->invalidate();
        for (size_t i = 0; i < m_linkHighlights.size(); ++i)
            m_linkHighlights[i]->invalidate();
    }
}

void GraphicsLayer::updateContentsRect()
{
    WebLayer* contentsLayer = contentsLayerIfRegistered();
    if (!contentsLayer)
        return;

    contentsLayer->setPosition(FloatPoint(m_contentsRect.x(), m_contentsRect.y()));
    contentsLayer->setBounds(IntSize(m_contentsRect.width(), m_contentsRect.height()));

    if (m_contentsClippingMaskLayer) {
        if (m_contentsClippingMaskLayer->size() != m_contentsRect.size()) {
            m_contentsClippingMaskLayer->setSize(m_contentsRect.size());
            m_contentsClippingMaskLayer->setNeedsDisplay();
        }
        m_contentsClippingMaskLayer->setPosition(FloatPoint());
        m_contentsClippingMaskLayer->setOffsetFromRenderer(offsetFromRenderer() + IntSize(m_contentsRect.location().x(), m_contentsRect.location().y()));
    }
}

static HashSet<int>* s_registeredLayerSet;

void GraphicsLayer::registerContentsLayer(WebLayer* layer)
{
    if (!s_registeredLayerSet)
        s_registeredLayerSet = new HashSet<int>;
    if (s_registeredLayerSet->contains(layer->id()))
        CRASH();
    s_registeredLayerSet->add(layer->id());
}

void GraphicsLayer::unregisterContentsLayer(WebLayer* layer)
{
    ASSERT(s_registeredLayerSet);
    if (!s_registeredLayerSet->contains(layer->id()))
        CRASH();
    s_registeredLayerSet->remove(layer->id());
}

void GraphicsLayer::setContentsTo(WebLayer* layer)
{
    bool childrenChanged = false;
    if (layer) {
        ASSERT(s_registeredLayerSet);
        if (!s_registeredLayerSet->contains(layer->id()))
            CRASH();
        if (m_contentsLayerId != layer->id()) {
            setupContentsLayer(layer);
            childrenChanged = true;
        }
        updateContentsRect();
    } else {
        if (m_contentsLayer) {
            childrenChanged = true;

            // The old contents layer will be removed via updateChildList.
            m_contentsLayer = 0;
            m_contentsLayerId = 0;
        }
    }

    if (childrenChanged)
        updateChildList();
}

void GraphicsLayer::setupContentsLayer(WebLayer* contentsLayer)
{
    ASSERT(contentsLayer);
    m_contentsLayer = contentsLayer;
    m_contentsLayerId = m_contentsLayer->id();

    m_contentsLayer->setWebLayerClient(this);
    m_contentsLayer->setAnchorPoint(FloatPoint(0, 0));
    m_contentsLayer->setUseParentBackfaceVisibility(true);

    // It is necessary to call setDrawsContent as soon as we receive the new contentsLayer, for
    // the correctness of early exit conditions in setDrawsContent() and setContentsVisible().
    m_contentsLayer->setDrawsContent(m_contentsVisible);

    // Insert the content layer first. Video elements require this, because they have
    // shadow content that must display in front of the video.
    m_layer->layer()->insertChild(m_contentsLayer, 0);
    WebLayer* borderWebLayer = m_contentsClippingMaskLayer ? m_contentsClippingMaskLayer->platformLayer() : 0;
    m_contentsLayer->setMaskLayer(borderWebLayer);
}

void GraphicsLayer::clearContentsLayerIfUnregistered()
{
    if (!m_contentsLayerId || s_registeredLayerSet->contains(m_contentsLayerId))
        return;

    m_contentsLayer = 0;
    m_contentsLayerId = 0;
}

GraphicsLayerDebugInfo& GraphicsLayer::debugInfo()
{
    return m_debugInfo;
}

blink::WebGraphicsLayerDebugInfo* GraphicsLayer::takeDebugInfoFor(WebLayer* layer)
{
    GraphicsLayerDebugInfo* clone = m_debugInfo.clone();
    clone->setDebugName(debugName(layer));
    return clone;
}

WebLayer* GraphicsLayer::contentsLayerIfRegistered()
{
    clearContentsLayerIfUnregistered();
    return m_contentsLayer;
}

double GraphicsLayer::backingStoreMemoryEstimate() const
{
    if (!drawsContent())
        return 0;

    // Effects of page and device scale are ignored; subclasses should override to take these into account.
    return static_cast<double>(4 * size().width()) * size().height();
}

void GraphicsLayer::resetTrackedRepaints()
{
    repaintRectMap().remove(this);
}

void GraphicsLayer::addRepaintRect(const FloatRect& repaintRect)
{
    if (m_client->isTrackingRepaints()) {
        FloatRect largestRepaintRect(FloatPoint(), m_size);
        largestRepaintRect.intersect(repaintRect);
        RepaintMap::iterator repaintIt = repaintRectMap().find(this);
        if (repaintIt == repaintRectMap().end()) {
            Vector<FloatRect> repaintRects;
            repaintRects.append(largestRepaintRect);
            repaintRectMap().set(this, repaintRects);
        } else {
            Vector<FloatRect>& repaintRects = repaintIt->value;
            repaintRects.append(largestRepaintRect);
        }
    }
}

void GraphicsLayer::collectTrackedRepaintRects(Vector<FloatRect>& rects) const
{
    if (!m_client->isTrackingRepaints())
        return;

    RepaintMap::iterator repaintIt = repaintRectMap().find(this);
    if (repaintIt != repaintRectMap().end())
        rects.append(repaintIt->value);
}

void GraphicsLayer::dumpLayer(TextStream& ts, int indent, LayerTreeFlags flags) const
{
    writeIndent(ts, indent);
    ts << "(" << "GraphicsLayer";

    if (flags & LayerTreeIncludesDebugInfo) {
        ts << " " << static_cast<void*>(const_cast<GraphicsLayer*>(this));
        ts << " \"" << m_client->debugName(this) << "\"";
    }

    ts << "\n";
    dumpProperties(ts, indent, flags);
    writeIndent(ts, indent);
    ts << ")\n";
}

void GraphicsLayer::dumpProperties(TextStream& ts, int indent, LayerTreeFlags flags) const
{
    if (m_position != FloatPoint()) {
        writeIndent(ts, indent + 1);
        ts << "(position " << m_position.x() << " " << m_position.y() << ")\n";
    }

    if (m_boundsOrigin != FloatPoint()) {
        writeIndent(ts, indent + 1);
        ts << "(bounds origin " << m_boundsOrigin.x() << " " << m_boundsOrigin.y() << ")\n";
    }

    if (m_anchorPoint != FloatPoint3D(0.5f, 0.5f, 0)) {
        writeIndent(ts, indent + 1);
        ts << "(anchor " << m_anchorPoint.x() << " " << m_anchorPoint.y() << ")\n";
    }

    if (m_size != IntSize()) {
        writeIndent(ts, indent + 1);
        ts << "(bounds " << m_size.width() << " " << m_size.height() << ")\n";
    }

    if (m_opacity != 1) {
        writeIndent(ts, indent + 1);
        ts << "(opacity " << m_opacity << ")\n";
    }

    if (m_blendMode != blink::WebBlendModeNormal) {
        writeIndent(ts, indent + 1);
        ts << "(blendMode " << compositeOperatorName(CompositeSourceOver, m_blendMode) << ")\n";
    }

    if (m_isRootForIsolatedGroup) {
        writeIndent(ts, indent + 1);
        ts << "(isolate " << m_isRootForIsolatedGroup << ")\n";
    }

    if (m_contentsOpaque) {
        writeIndent(ts, indent + 1);
        ts << "(contentsOpaque " << m_contentsOpaque << ")\n";
    }

    if (m_preserves3D) {
        writeIndent(ts, indent + 1);
        ts << "(preserves3D " << m_preserves3D << ")\n";
    }

    if (m_drawsContent) {
        writeIndent(ts, indent + 1);
        ts << "(drawsContent " << m_drawsContent << ")\n";
    }

    if (!m_contentsVisible) {
        writeIndent(ts, indent + 1);
        ts << "(contentsVisible " << m_contentsVisible << ")\n";
    }

    if (!m_backfaceVisibility) {
        writeIndent(ts, indent + 1);
        ts << "(backfaceVisibility " << (m_backfaceVisibility ? "visible" : "hidden") << ")\n";
    }

    if (flags & LayerTreeIncludesDebugInfo) {
        writeIndent(ts, indent + 1);
        ts << "(";
        if (m_client)
            ts << "client " << static_cast<void*>(m_client);
        else
            ts << "no client";
        ts << ")\n";
    }

    if (m_backgroundColor.isValid() && m_backgroundColor != Color::transparent) {
        writeIndent(ts, indent + 1);
        ts << "(backgroundColor " << m_backgroundColor.nameForRenderTreeAsText() << ")\n";
    }

    if (!m_transform.isIdentity()) {
        writeIndent(ts, indent + 1);
        ts << "(transform ";
        ts << "[" << m_transform.m11() << " " << m_transform.m12() << " " << m_transform.m13() << " " << m_transform.m14() << "] ";
        ts << "[" << m_transform.m21() << " " << m_transform.m22() << " " << m_transform.m23() << " " << m_transform.m24() << "] ";
        ts << "[" << m_transform.m31() << " " << m_transform.m32() << " " << m_transform.m33() << " " << m_transform.m34() << "] ";
        ts << "[" << m_transform.m41() << " " << m_transform.m42() << " " << m_transform.m43() << " " << m_transform.m44() << "])\n";
    }

    // Avoid dumping the sublayer transform on the root layer, because it's used for geometry flipping, whose behavior
    // differs between platforms.
    if (parent() && !m_childrenTransform.isIdentity()) {
        writeIndent(ts, indent + 1);
        ts << "(childrenTransform ";
        ts << "[" << m_childrenTransform.m11() << " " << m_childrenTransform.m12() << " " << m_childrenTransform.m13() << " " << m_childrenTransform.m14() << "] ";
        ts << "[" << m_childrenTransform.m21() << " " << m_childrenTransform.m22() << " " << m_childrenTransform.m23() << " " << m_childrenTransform.m24() << "] ";
        ts << "[" << m_childrenTransform.m31() << " " << m_childrenTransform.m32() << " " << m_childrenTransform.m33() << " " << m_childrenTransform.m34() << "] ";
        ts << "[" << m_childrenTransform.m41() << " " << m_childrenTransform.m42() << " " << m_childrenTransform.m43() << " " << m_childrenTransform.m44() << "])\n";
    }

    if (m_replicaLayer) {
        writeIndent(ts, indent + 1);
        ts << "(replica layer";
        if (flags & LayerTreeIncludesDebugInfo)
            ts << " " << m_replicaLayer;
        ts << ")\n";
        m_replicaLayer->dumpLayer(ts, indent + 2, flags);
    }

    if (m_replicatedLayer) {
        writeIndent(ts, indent + 1);
        ts << "(replicated layer";
        if (flags & LayerTreeIncludesDebugInfo)
            ts << " " << m_replicatedLayer;
        ts << ")\n";
    }

    if ((flags & LayerTreeIncludesRepaintRects) && repaintRectMap().contains(this) && !repaintRectMap().get(this).isEmpty()) {
        writeIndent(ts, indent + 1);
        ts << "(repaint rects\n";
        for (size_t i = 0; i < repaintRectMap().get(this).size(); ++i) {
            if (repaintRectMap().get(this)[i].isEmpty())
                continue;
            writeIndent(ts, indent + 2);
            ts << "(rect ";
            ts << repaintRectMap().get(this)[i].x() << " ";
            ts << repaintRectMap().get(this)[i].y() << " ";
            ts << repaintRectMap().get(this)[i].width() << " ";
            ts << repaintRectMap().get(this)[i].height();
            ts << ")\n";
        }
        writeIndent(ts, indent + 1);
        ts << ")\n";
    }

    if ((flags & LayerTreeIncludesPaintingPhases) && paintingPhase()) {
        writeIndent(ts, indent + 1);
        ts << "(paintingPhases\n";
        if (paintingPhase() & GraphicsLayerPaintBackground) {
            writeIndent(ts, indent + 2);
            ts << "GraphicsLayerPaintBackground\n";
        }
        if (paintingPhase() & GraphicsLayerPaintForeground) {
            writeIndent(ts, indent + 2);
            ts << "GraphicsLayerPaintForeground\n";
        }
        if (paintingPhase() & GraphicsLayerPaintMask) {
            writeIndent(ts, indent + 2);
            ts << "GraphicsLayerPaintMask\n";
        }
        if (paintingPhase() & GraphicsLayerPaintChildClippingMask) {
            writeIndent(ts, indent + 2);
            ts << "GraphicsLayerPaintChildClippingMask\n";
        }
        if (paintingPhase() & GraphicsLayerPaintOverflowContents) {
            writeIndent(ts, indent + 2);
            ts << "GraphicsLayerPaintOverflowContents\n";
        }
        if (paintingPhase() & GraphicsLayerPaintCompositedScroll) {
            writeIndent(ts, indent + 2);
            ts << "GraphicsLayerPaintCompositedScroll\n";
        }
        writeIndent(ts, indent + 1);
        ts << ")\n";
    }

    if (flags & LayerTreeIncludesClipAndScrollParents) {
        if (m_hasScrollParent) {
            writeIndent(ts, indent + 1);
            ts << "(hasScrollParent 1)\n";
        }
        if (m_hasClipParent) {
            writeIndent(ts, indent + 1);
            ts << "(hasClipParent 1)\n";
        }
    }

    if (m_children.size()) {
        writeIndent(ts, indent + 1);
        ts << "(children " << m_children.size() << "\n";

        unsigned i;
        for (i = 0; i < m_children.size(); i++)
            m_children[i]->dumpLayer(ts, indent + 2, flags);
        writeIndent(ts, indent + 1);
        ts << ")\n";
    }
}

String GraphicsLayer::layerTreeAsText(LayerTreeFlags flags) const
{
    TextStream ts;

    dumpLayer(ts, 0, flags);
    return ts.release();
}

String GraphicsLayer::debugName(blink::WebLayer* webLayer) const
{
    String name;
    if (!m_client)
        return name;

    String highlightDebugName;
    for (size_t i = 0; i < m_linkHighlights.size(); ++i) {
        if (webLayer == m_linkHighlights[i]->layer()) {
            highlightDebugName = "LinkHighlight[" + String::number(i) + "] for " + m_client->debugName(this);
            break;
        }
    }

    if (webLayer == m_contentsLayer) {
        name = "ContentsLayer for " + m_client->debugName(this);
    } else if (!highlightDebugName.isEmpty()) {
        name = highlightDebugName;
    } else if (webLayer == m_layer->layer()) {
        name = m_client->debugName(this);
    } else {
        ASSERT_NOT_REACHED();
    }
    return name;
}

void GraphicsLayer::setCompositingReasons(CompositingReasons reasons)
{
    m_debugInfo.setCompositingReasons(reasons);
}

void GraphicsLayer::setPosition(const FloatPoint& point)
{
    m_position = point;
    platformLayer()->setPosition(m_position);
}

void GraphicsLayer::setAnchorPoint(const FloatPoint3D& point)
{
    m_anchorPoint = point;
    platformLayer()->setAnchorPoint(FloatPoint(m_anchorPoint.x(), m_anchorPoint.y()));
    platformLayer()->setAnchorPointZ(m_anchorPoint.z());
}

void GraphicsLayer::setSize(const FloatSize& size)
{
    // We are receiving negative sizes here that cause assertions to fail in the compositor. Clamp them to 0 to
    // avoid those assertions.
    // FIXME: This should be an ASSERT instead, as negative sizes should not exist in WebCore.
    FloatSize clampedSize = size;
    if (clampedSize.width() < 0 || clampedSize.height() < 0)
        clampedSize = FloatSize();

    if (clampedSize == m_size)
        return;

    m_size = clampedSize;

    m_layer->layer()->setBounds(flooredIntSize(m_size));
    // Note that we don't resize m_contentsLayer. It's up the caller to do that.
}

void GraphicsLayer::setTransform(const TransformationMatrix& transform)
{
    m_transform = transform;
    platformLayer()->setTransform(TransformationMatrix::toSkMatrix44(m_transform));
}

void GraphicsLayer::setChildrenTransform(const TransformationMatrix& transform)
{
    m_childrenTransform = transform;
    platformLayer()->setSublayerTransform(TransformationMatrix::toSkMatrix44(m_childrenTransform));
}

void GraphicsLayer::setPreserves3D(bool preserves3D)
{
    if (preserves3D == m_preserves3D)
        return;

    m_preserves3D = preserves3D;
    m_layer->layer()->setPreserves3D(m_preserves3D);
}

void GraphicsLayer::setMasksToBounds(bool masksToBounds)
{
    m_masksToBounds = masksToBounds;
    m_layer->layer()->setMasksToBounds(m_masksToBounds);
}

void GraphicsLayer::setDrawsContent(bool drawsContent)
{
    // Note carefully this early-exit is only correct because we also properly call
    // WebLayer::setDrawsContent whenever m_contentsLayer is set to a new layer in setupContentsLayer().
    if (drawsContent == m_drawsContent)
        return;

    m_drawsContent = drawsContent;
    updateLayerIsDrawable();
}

void GraphicsLayer::setContentsVisible(bool contentsVisible)
{
    // Note carefully this early-exit is only correct because we also properly call
    // WebLayer::setDrawsContent whenever m_contentsLayer is set to a new layer in setupContentsLayer().
    if (contentsVisible == m_contentsVisible)
        return;

    m_contentsVisible = contentsVisible;
    updateLayerIsDrawable();
}

void GraphicsLayer::setClipParent(blink::WebLayer* parent)
{
    m_hasClipParent = !!parent;
    m_layer->layer()->setClipParent(parent);
}

void GraphicsLayer::setScrollParent(blink::WebLayer* parent)
{
    m_hasScrollParent = !!parent;
    m_layer->layer()->setScrollParent(parent);
}

void GraphicsLayer::setBackgroundColor(const Color& color)
{
    if (color == m_backgroundColor)
        return;

    m_backgroundColor = color;
    m_layer->layer()->setBackgroundColor(m_backgroundColor.rgb());
}

void GraphicsLayer::setContentsOpaque(bool opaque)
{
    m_contentsOpaque = opaque;
    m_layer->layer()->setOpaque(m_contentsOpaque);
    m_opaqueRectTrackingContentLayerDelegate->setOpaque(m_contentsOpaque);
}

void GraphicsLayer::setMaskLayer(GraphicsLayer* maskLayer)
{
    if (maskLayer == m_maskLayer)
        return;

    m_maskLayer = maskLayer;
    WebLayer* maskWebLayer = m_maskLayer ? m_maskLayer->platformLayer() : 0;
    m_layer->layer()->setMaskLayer(maskWebLayer);
}

void GraphicsLayer::setContentsClippingMaskLayer(GraphicsLayer* contentsClippingMaskLayer)
{
    if (contentsClippingMaskLayer == m_contentsClippingMaskLayer)
        return;

    m_contentsClippingMaskLayer = contentsClippingMaskLayer;
    WebLayer* contentsLayer = contentsLayerIfRegistered();
    if (!contentsLayer)
        return;
    WebLayer* contentsClippingMaskWebLayer = m_contentsClippingMaskLayer ? m_contentsClippingMaskLayer->platformLayer() : 0;
    contentsLayer->setMaskLayer(contentsClippingMaskWebLayer);
    updateContentsRect();
}

void GraphicsLayer::setBackfaceVisibility(bool visible)
{
    m_backfaceVisibility = visible;
    m_layer->setDoubleSided(m_backfaceVisibility);
}

void GraphicsLayer::setOpacity(float opacity)
{
    float clampedOpacity = std::max(std::min(opacity, 1.0f), 0.0f);
    m_opacity = clampedOpacity;
    platformLayer()->setOpacity(opacity);
}

void GraphicsLayer::setBlendMode(blink::WebBlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;
    m_blendMode = blendMode;
    platformLayer()->setBlendMode(blink::WebBlendMode(blendMode));
}

void GraphicsLayer::setIsRootForIsolatedGroup(bool isolated)
{
    if (m_isRootForIsolatedGroup == isolated)
        return;
    m_isRootForIsolatedGroup = isolated;
    platformLayer()->setIsRootForIsolatedGroup(isolated);
}

void GraphicsLayer::setContentsNeedsDisplay()
{
    if (WebLayer* contentsLayer = contentsLayerIfRegistered()) {
        contentsLayer->invalidate();
        addRepaintRect(contentsRect());
    }
}

void GraphicsLayer::setNeedsDisplay()
{
    if (drawsContent()) {
        m_layer->layer()->invalidate();
        addRepaintRect(FloatRect(FloatPoint(), m_size));
        for (size_t i = 0; i < m_linkHighlights.size(); ++i)
            m_linkHighlights[i]->invalidate();
    }
}

void GraphicsLayer::setNeedsDisplayInRect(const FloatRect& rect)
{
    if (drawsContent()) {
        m_layer->layer()->invalidateRect(rect);
        addRepaintRect(rect);
        for (size_t i = 0; i < m_linkHighlights.size(); ++i)
            m_linkHighlights[i]->invalidate();
    }
}

void GraphicsLayer::setContentsRect(const IntRect& rect)
{
    if (rect == m_contentsRect)
        return;

    m_contentsRect = rect;
    updateContentsRect();
}

void GraphicsLayer::setContentsToImage(Image* image)
{
    RefPtr<NativeImageSkia> nativeImage = image ? image->nativeImageForCurrentFrame() : 0;
    if (nativeImage) {
        if (!m_imageLayer) {
            m_imageLayer = adoptPtr(Platform::current()->compositorSupport()->createImageLayer());
            registerContentsLayer(m_imageLayer->layer());
        }
        m_imageLayer->setBitmap(nativeImage->bitmap());
        m_imageLayer->layer()->setOpaque(image->currentFrameKnownToBeOpaque());
        updateContentsRect();
    } else {
        if (m_imageLayer) {
            unregisterContentsLayer(m_imageLayer->layer());
            m_imageLayer.clear();
        }
    }

    setContentsTo(m_imageLayer ? m_imageLayer->layer() : 0);
}

void GraphicsLayer::setContentsToNinePatch(Image* image, const IntRect& aperture)
{
    if (m_ninePatchLayer) {
        unregisterContentsLayer(m_ninePatchLayer->layer());
        m_ninePatchLayer.clear();
    }
    RefPtr<NativeImageSkia> nativeImage = image ? image->nativeImageForCurrentFrame() : 0;
    if (nativeImage) {
        m_ninePatchLayer = adoptPtr(Platform::current()->compositorSupport()->createNinePatchLayer());
        m_ninePatchLayer->setBitmap(nativeImage->bitmap(), aperture);
        m_ninePatchLayer->layer()->setOpaque(image->currentFrameKnownToBeOpaque());
        registerContentsLayer(m_ninePatchLayer->layer());
    }
    setContentsTo(m_ninePatchLayer ? m_ninePatchLayer->layer() : 0);
}

void GraphicsLayer::setContentsToSolidColor(const Color& color)
{
    if (color == m_contentsSolidColor)
        return;

    m_contentsSolidColor = color;
    if (color.isValid() && color.alpha()) {
        if (!m_solidColorLayer) {
            m_solidColorLayer = adoptPtr(Platform::current()->compositorSupport()->createSolidColorLayer());
            registerContentsLayer(m_solidColorLayer->layer());
        }
        m_solidColorLayer->setBackgroundColor(color.rgb());
    } else {
        if (!m_solidColorLayer)
            return;
        unregisterContentsLayer(m_solidColorLayer->layer());
        m_solidColorLayer.clear();
    }
    setContentsTo(m_solidColorLayer ? m_solidColorLayer->layer() : 0);
}

bool GraphicsLayer::addAnimation(PassOwnPtr<WebAnimation> popAnimation)
{
    OwnPtr<WebAnimation> animation(popAnimation);
    ASSERT(animation);
    platformLayer()->setAnimationDelegate(this);

    // Remove any existing animations with the same animation id and target property.
    platformLayer()->removeAnimation(animation->id(), animation->targetProperty());
    return platformLayer()->addAnimation(animation.leakPtr());
}

void GraphicsLayer::pauseAnimation(int animationId, double timeOffset)
{
    platformLayer()->pauseAnimation(animationId, timeOffset);
}

void GraphicsLayer::removeAnimation(int animationId)
{
    platformLayer()->removeAnimation(animationId);
}

WebLayer* GraphicsLayer::platformLayer() const
{
    return m_layer->layer();
}

static bool copyWebCoreFilterOperationsToWebFilterOperations(const FilterOperations& filters, WebFilterOperations& webFilters)
{
    for (size_t i = 0; i < filters.size(); ++i) {
        const FilterOperation& op = *filters.at(i);
        switch (op.type()) {
        case FilterOperation::REFERENCE:
            return false; // Not supported.
        case FilterOperation::GRAYSCALE:
        case FilterOperation::SEPIA:
        case FilterOperation::SATURATE:
        case FilterOperation::HUE_ROTATE: {
            float amount = toBasicColorMatrixFilterOperation(op).amount();
            switch (op.type()) {
            case FilterOperation::GRAYSCALE:
                webFilters.appendGrayscaleFilter(amount);
                break;
            case FilterOperation::SEPIA:
                webFilters.appendSepiaFilter(amount);
                break;
            case FilterOperation::SATURATE:
                webFilters.appendSaturateFilter(amount);
                break;
            case FilterOperation::HUE_ROTATE:
                webFilters.appendHueRotateFilter(amount);
                break;
            default:
                ASSERT_NOT_REACHED();
            }
            break;
        }
        case FilterOperation::INVERT:
        case FilterOperation::OPACITY:
        case FilterOperation::BRIGHTNESS:
        case FilterOperation::CONTRAST: {
            float amount = toBasicComponentTransferFilterOperation(op).amount();
            switch (op.type()) {
            case FilterOperation::INVERT:
                webFilters.appendInvertFilter(amount);
                break;
            case FilterOperation::OPACITY:
                webFilters.appendOpacityFilter(amount);
                break;
            case FilterOperation::BRIGHTNESS:
                webFilters.appendBrightnessFilter(amount);
                break;
            case FilterOperation::CONTRAST:
                webFilters.appendContrastFilter(amount);
                break;
            default:
                ASSERT_NOT_REACHED();
            }
            break;
        }
        case FilterOperation::BLUR: {
            float pixelRadius = toBlurFilterOperation(op).stdDeviation().getFloatValue();
            webFilters.appendBlurFilter(pixelRadius);
            break;
        }
        case FilterOperation::DROP_SHADOW: {
            const DropShadowFilterOperation& dropShadowOp = toDropShadowFilterOperation(op);
            webFilters.appendDropShadowFilter(WebPoint(dropShadowOp.x(), dropShadowOp.y()), dropShadowOp.stdDeviation(), dropShadowOp.color().rgb());
            break;
        }
        case FilterOperation::CUSTOM:
        case FilterOperation::VALIDATED_CUSTOM:
            return false; // Not supported.
        case FilterOperation::NONE:
            break;
        }
    }
    return true;
}

bool GraphicsLayer::setFilters(const FilterOperations& filters)
{
    SkiaImageFilterBuilder builder;
    OwnPtr<WebFilterOperations> webFilters = adoptPtr(Platform::current()->compositorSupport()->createFilterOperations());
    FilterOutsets outsets = filters.outsets();
    builder.setCropOffset(FloatSize(outsets.left(), outsets.top()));
    if (!builder.buildFilterOperations(filters, webFilters.get())) {
        // Make sure the filters are removed from the platform layer, as they are
        // going to fallback to software mode.
        webFilters->clear();
        m_layer->layer()->setFilters(*webFilters);
        m_filters = FilterOperations();
        return false;
    }

    m_layer->layer()->setFilters(*webFilters);
    m_filters = filters;
    return true;
}

void GraphicsLayer::setBackgroundFilters(const FilterOperations& filters)
{
    OwnPtr<WebFilterOperations> webFilters = adoptPtr(Platform::current()->compositorSupport()->createFilterOperations());
    if (!copyWebCoreFilterOperationsToWebFilterOperations(filters, *webFilters))
        return;
    m_layer->layer()->setBackgroundFilters(*webFilters);
}

void GraphicsLayer::addLinkHighlight(LinkHighlightClient* linkHighlight)
{
    ASSERT(linkHighlight && !m_linkHighlights.contains(linkHighlight));
    m_linkHighlights.append(linkHighlight);
    linkHighlight->layer()->setWebLayerClient(this);
    updateChildList();
}

void GraphicsLayer::removeLinkHighlight(LinkHighlightClient* linkHighlight)
{
    m_linkHighlights.remove(m_linkHighlights.find(linkHighlight));
    updateChildList();
}

void GraphicsLayer::setScrollableArea(ScrollableArea* scrollableArea, bool isMainFrame)
{
    if (m_scrollableArea == scrollableArea)
        return;

    m_scrollableArea = scrollableArea;

    // Main frame scrolling may involve pinch zoom and gets routed through
    // WebViewImpl explicitly rather than via GraphicsLayer::didScroll.
    if (isMainFrame)
        m_layer->layer()->setScrollClient(0);
    else
        m_layer->layer()->setScrollClient(this);
}

void GraphicsLayer::paint(GraphicsContext& context, const IntRect& clip)
{
    paintGraphicsLayerContents(context, clip);
}


void GraphicsLayer::notifyAnimationStarted(double wallClockTime, double monotonicTime, WebAnimation::TargetProperty)
{
    if (m_client)
        m_client->notifyAnimationStarted(this, wallClockTime, monotonicTime);
}

void GraphicsLayer::notifyAnimationFinished(double, double, WebAnimation::TargetProperty)
{
    // Do nothing.
}

void GraphicsLayer::didScroll()
{
    if (m_scrollableArea)
        m_scrollableArea->scrollToOffsetWithoutAnimation(m_scrollableArea->minimumScrollPosition() + toIntSize(m_layer->layer()->scrollPosition()));
}

} // namespace WebCore

#ifndef NDEBUG
void showGraphicsLayerTree(const WebCore::GraphicsLayer* layer)
{
    if (!layer)
        return;

    String output = layer->layerTreeAsText(WebCore::LayerTreeIncludesDebugInfo);
    fprintf(stderr, "%s\n", output.utf8().data());
}
#endif
