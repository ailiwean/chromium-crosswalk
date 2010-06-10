/*
 * Copyright (C) 2004, 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
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
#include "HTMLCanvasElement.h"

#include "Attribute.h"
#include "CanvasContextAttributes.h"
#include "CanvasGradient.h"
#include "CanvasPattern.h"
#include "CanvasRenderingContext2D.h"
#include "CanvasStyle.h"
#include "Chrome.h"
#include "Document.h"
#include "Frame.h"
#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "ImageBuffer.h"
#include "Page.h"
#include "RenderHTMLCanvas.h"
#include "Settings.h"
#include <math.h>
#include <stdio.h>

#if ENABLE(3D_CANVAS)    
#include "WebGLContextAttributes.h"
#include "WebGLRenderingContext.h"
#endif

namespace WebCore {

using namespace HTMLNames;

HTMLCanvasElement::HTMLCanvasElement(const QualifiedName& tagName, Document* document)
    : HTMLElement(tagName, document)
    , CanvasSurface(document->frame() ? document->frame()->page()->chrome()->scaleFactor() : 1)
    , m_observer(0)
    , m_ignoreReset(false)
{
    ASSERT(hasTagName(canvasTag));
}

PassRefPtr<HTMLCanvasElement> HTMLCanvasElement::create(Document* document)
{
    return new HTMLCanvasElement(canvasTag, document);
}

PassRefPtr<HTMLCanvasElement> HTMLCanvasElement::create(const QualifiedName& tagName, Document* document)
{
    return new HTMLCanvasElement(tagName, document);
}

HTMLCanvasElement::~HTMLCanvasElement()
{
    if (m_observer)
        m_observer->canvasDestroyed(this);
}

#if ENABLE(DASHBOARD_SUPPORT)

HTMLTagStatus HTMLCanvasElement::endTagRequirement() const 
{
    Settings* settings = document()->settings();
    if (settings && settings->usesDashboardBackwardCompatibilityMode())
        return TagStatusForbidden; 

    return HTMLElement::endTagRequirement();
}

int HTMLCanvasElement::tagPriority() const 
{ 
    Settings* settings = document()->settings();
    if (settings && settings->usesDashboardBackwardCompatibilityMode())
        return 0; 

    return HTMLElement::tagPriority();
}

#endif

void HTMLCanvasElement::parseMappedAttribute(Attribute* attr)
{
    const QualifiedName& attrName = attr->name();
    if (attrName == widthAttr || attrName == heightAttr)
        reset();
    HTMLElement::parseMappedAttribute(attr);
}

RenderObject* HTMLCanvasElement::createRenderer(RenderArena* arena, RenderStyle* style)
{
    Frame* frame = document()->frame();
    if (frame && frame->script()->canExecuteScripts(NotAboutToExecuteScript)) {
        m_rendererIsCanvas = true;
        return new (arena) RenderHTMLCanvas(this);
    }

    m_rendererIsCanvas = false;
    return HTMLElement::createRenderer(arena, style);
}

void HTMLCanvasElement::setHeight(int value)
{
    setAttribute(heightAttr, String::number(value));
}

void HTMLCanvasElement::setWidth(int value)
{
    setAttribute(widthAttr, String::number(value));
}

CanvasRenderingContext* HTMLCanvasElement::getContext(const String& type, CanvasContextAttributes* attrs)
{
    // A Canvas can either be "2D" or "webgl" but never both. If you request a 2D canvas and the existing
    // context is already 2D, just return that. If the existing context is WebGL, then destroy it
    // before creating a new 2D context. Vice versa when requesting a WebGL canvas. Requesting a
    // context with any other type string will destroy any existing context.
    
    // FIXME - The code depends on the context not going away once created, to prevent JS from
    // seeing a dangling pointer. So for now we will disallow the context from being changed
    // once it is created.
    if (type == "2d") {
        if (m_context && !m_context->is2d())
            return 0;
        if (!m_context) {
            bool usesDashbardCompatibilityMode = false;
#if ENABLE(DASHBOARD_SUPPORT)
            if (Settings* settings = document()->settings())
                usesDashbardCompatibilityMode = settings->usesDashboardBackwardCompatibilityMode();
#endif
            m_context = new CanvasRenderingContext2D(this, document()->inCompatMode(), usesDashbardCompatibilityMode);
        }
        return m_context.get();
    }
#if ENABLE(3D_CANVAS)    
    Settings* settings = document()->settings();
    if (settings && settings->webGLEnabled()
#if !PLATFORM(CHROMIUM) && !PLATFORM(QT)
        && settings->acceleratedCompositingEnabled()
#endif
        ) {
        // Accept the legacy "webkit-3d" name as well as the provisional "experimental-webgl" name.
        // Once ratified, we will also accept "webgl" as the context name.
        if ((type == "webkit-3d") ||
            (type == "experimental-webgl")) {
            if (m_context && !m_context->is3d())
                return 0;
            if (!m_context) {
                m_context = WebGLRenderingContext::create(this, static_cast<WebGLContextAttributes*>(attrs));
                if (m_context) {
                    // Need to make sure a RenderLayer and compositing layer get created for the Canvas
                    setNeedsStyleRecalc(SyntheticStyleChange);
                }
            }
            return m_context.get();
        }
    }
#else
    UNUSED_PARAM(attrs);
#endif
    return 0;
}

void HTMLCanvasElement::willDraw(const FloatRect& rect)
{
    CanvasSurface::willDraw(rect);

    if (RenderBox* ro = renderBox()) {
        FloatRect destRect = ro->contentBoxRect();
        FloatRect r = mapRect(rect, FloatRect(0, 0, size().width(), size().height()), destRect);
        r.intersect(destRect);
        if (m_dirtyRect.contains(r))
            return;

        m_dirtyRect.unite(r);
        ro->repaintRectangle(enclosingIntRect(m_dirtyRect));
    }
    
    if (m_observer)
        m_observer->canvasChanged(this, rect);
}

void HTMLCanvasElement::reset()
{
    if (m_ignoreReset)
        return;

    bool ok;
    int w = getAttribute(widthAttr).toInt(&ok);
    if (!ok)
        w = DefaultWidth;
    int h = getAttribute(heightAttr).toInt(&ok);
    if (!ok)
        h = DefaultHeight;

    IntSize oldSize = size();
    setSurfaceSize(IntSize(w, h));

#if ENABLE(3D_CANVAS)
    if (m_context && m_context->is3d())
        static_cast<WebGLRenderingContext*>(m_context.get())->reshape(width(), height());
#endif

    bool hadImageBuffer = hasCreatedImageBuffer();
    if (m_context && m_context->is2d())
        static_cast<CanvasRenderingContext2D*>(m_context.get())->reset();

    if (RenderObject* renderer = this->renderer()) {
        if (m_rendererIsCanvas) {
            if (oldSize != size())
                toRenderHTMLCanvas(renderer)->canvasSizeChanged();
            if (hadImageBuffer)
                renderer->repaint();
        }
    }

    if (m_observer)
        m_observer->canvasResized(this);
}

void HTMLCanvasElement::paint(GraphicsContext* context, const IntRect& r)
{
    // Clear the dirty rect
    m_dirtyRect = FloatRect();

    if (context->paintingDisabled())
        return;
    
#if ENABLE(3D_CANVAS)
    WebGLRenderingContext* context3D = 0;
    if (m_context && m_context->is3d()) {
        context3D = static_cast<WebGLRenderingContext*>(m_context.get());
        context3D->beginPaint();
    }
#endif

    if (hasCreatedImageBuffer()) {
        ImageBuffer* imageBuffer = buffer();
        if (imageBuffer) {
            Image* image = imageBuffer->image();
            if (image)
                context->drawImage(image, DeviceColorSpace, r);
        }
    }

#if ENABLE(3D_CANVAS)
    if (context3D)
        context3D->endPaint();
#endif
}

#if ENABLE(3D_CANVAS)    
bool HTMLCanvasElement::is3D() const
{
    return m_context && m_context->is3d();
}
#endif

}
