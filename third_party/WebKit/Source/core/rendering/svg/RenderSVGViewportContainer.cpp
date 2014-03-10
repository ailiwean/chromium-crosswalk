/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Google, Inc.
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

#include "core/rendering/svg/RenderSVGViewportContainer.h"

#include "SVGNames.h"
#include "core/svg/SVGElementInstance.h"
#include "core/svg/SVGSVGElement.h"
#include "core/svg/SVGUseElement.h"
#include "platform/graphics/GraphicsContext.h"

namespace WebCore {

RenderSVGViewportContainer::RenderSVGViewportContainer(SVGElement* node)
    : RenderSVGContainer(node)
    , m_didTransformToRootUpdate(false)
    , m_isLayoutSizeChanged(false)
    , m_needsTransformUpdate(true)
{
}

void RenderSVGViewportContainer::determineIfLayoutSizeChanged()
{
    ASSERT(element());
    if (!isSVGSVGElement(*element()))
        return;

    m_isLayoutSizeChanged = toSVGSVGElement(element())->hasRelativeLengths() && selfNeedsLayout();
}

void RenderSVGViewportContainer::applyViewportClip(PaintInfo& paintInfo)
{
    if (SVGRenderSupport::isOverflowHidden(this))
        paintInfo.context->clip(m_viewport);
}

void RenderSVGViewportContainer::calcViewport()
{
    SVGElement* element = this->element();
    ASSERT(element);
    if (!isSVGSVGElement(*element))
        return;
    SVGSVGElement* svg = toSVGSVGElement(element);
    FloatRect oldViewport = m_viewport;

    SVGLengthContext lengthContext(element);
    m_viewport = FloatRect(svg->x()->currentValue()->value(lengthContext), svg->y()->currentValue()->value(lengthContext), svg->width()->currentValue()->value(lengthContext), svg->height()->currentValue()->value(lengthContext));

    SVGElement* correspondingElement = svg->correspondingElement();
    if (correspondingElement && svg->isInShadowTree()) {
        const HashSet<SVGElementInstance*>& instances = correspondingElement->instancesForElement();
        ASSERT(!instances.isEmpty());

        SVGUseElement* useElement = 0;
        const HashSet<SVGElementInstance*>::const_iterator end = instances.end();
        for (HashSet<SVGElementInstance*>::const_iterator it = instances.begin(); it != end; ++it) {
            const SVGElementInstance* instance = (*it);
            ASSERT(isSVGSVGElement(instance->correspondingElement()) || isSVGSymbolElement(instance->correspondingElement()));
            if (instance->shadowTreeElement() == svg) {
                ASSERT(correspondingElement == instance->correspondingElement());
                useElement = instance->directUseElement();
                if (!useElement)
                    useElement = instance->correspondingUseElement();
                break;
            }
        }

        ASSERT(useElement);
        bool isSymbolElement = isSVGSymbolElement(*correspondingElement);

        // Spec (<use> on <symbol>): This generated 'svg' will always have explicit values for attributes width and height.
        // If attributes width and/or height are provided on the 'use' element, then these attributes
        // will be transferred to the generated 'svg'. If attributes width and/or height are not specified,
        // the generated 'svg' element will use values of 100% for these attributes.

        // Spec (<use> on <svg>): If attributes width and/or height are provided on the 'use' element, then these
        // values will override the corresponding attributes on the 'svg' in the generated tree.

        SVGLengthContext lengthContext(element);
        if (useElement->hasAttribute(SVGNames::widthAttr))
            m_viewport.setWidth(useElement->width()->currentValue()->value(lengthContext));
        else if (isSymbolElement && svg->hasAttribute(SVGNames::widthAttr)) {
            RefPtr<SVGLength> containerWidth = SVGLength::create(LengthModeWidth);
            containerWidth->setValueAsString("100%", ASSERT_NO_EXCEPTION);
            m_viewport.setWidth(containerWidth->value(lengthContext));
        }

        if (useElement->hasAttribute(SVGNames::heightAttr))
            m_viewport.setHeight(useElement->height()->currentValue()->value(lengthContext));
        else if (isSymbolElement && svg->hasAttribute(SVGNames::heightAttr)) {
            RefPtr<SVGLength> containerHeight = SVGLength::create(LengthModeHeight);
            containerHeight->setValueAsString("100%", ASSERT_NO_EXCEPTION);
            m_viewport.setHeight(containerHeight->value(lengthContext));
        }
    }

    if (oldViewport != m_viewport) {
        setNeedsBoundariesUpdate();
        setNeedsTransformUpdate();
    }
}

bool RenderSVGViewportContainer::calculateLocalTransform()
{
    m_didTransformToRootUpdate = m_needsTransformUpdate || SVGRenderSupport::transformToRootChanged(parent());
    if (!m_needsTransformUpdate)
        return false;

    m_localToParentTransform = AffineTransform::translation(m_viewport.x(), m_viewport.y()) * viewportTransform();
    m_needsTransformUpdate = false;
    return true;
}

AffineTransform RenderSVGViewportContainer::viewportTransform() const
{
    ASSERT(element());
    if (isSVGSVGElement(*element())) {
        SVGSVGElement* svg = toSVGSVGElement(element());
        return svg->viewBoxToViewTransform(m_viewport.width(), m_viewport.height());
    }

    return AffineTransform();
}

bool RenderSVGViewportContainer::pointIsInsideViewportClip(const FloatPoint& pointInParent)
{
    // Respect the viewport clip (which is in parent coords)
    if (!SVGRenderSupport::isOverflowHidden(this))
        return true;

    return m_viewport.contains(pointInParent);
}

void RenderSVGViewportContainer::paint(PaintInfo& paintInfo, const LayoutPoint& paintOffset)
{
    ASSERT(element());
    // An empty viewBox disables rendering.
    if (isSVGSVGElement(*element()) && toSVGSVGElement(*element()).hasEmptyViewBox())
        return;

    RenderSVGContainer::paint(paintInfo, paintOffset);
}

}
