/*
 * Copyright (C) 2006 Rob Buis <buis@kde.org>
 *           (C) 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
#include "CSSCursorImageValue.h"

#include "DocLoader.h"
#include "Document.h"
#include "PlatformString.h"
#include <wtf/MathExtras.h>
#include <wtf/UnusedParam.h>

#if ENABLE(SVG)
#include "SVGCursorElement.h"
#include "SVGURIReference.h"
#endif

namespace WebCore {

#if ENABLE(SVG)
static inline bool isSVGCursorIdentifier(const String& url)
{
    KURL kurl(ParsedURLString, url);
    return kurl.hasFragmentIdentifier();
}

static inline SVGCursorElement* resourceReferencedByCursorElement(const String& fragmentId, Document* document)
{
    Element* element = document->getElementById(SVGURIReference::getTarget(fragmentId));
    if (element && element->hasTagName(SVGNames::cursorTag))
        return static_cast<SVGCursorElement*>(element);

    return 0;
}
#endif

CSSCursorImageValue::CSSCursorImageValue(const String& url, const IntPoint& hotspot)
    : CSSImageValue(url)
    , m_hotspot(hotspot)
{
}

CSSCursorImageValue::~CSSCursorImageValue()
{
#if ENABLE(SVG)
    const String& url = getStringValue();
    if (!isSVGCursorIdentifier(url))
        return;

    HashSet<SVGElement*>::const_iterator it = m_referencedElements.begin();
    HashSet<SVGElement*>::const_iterator end = m_referencedElements.end();

    for (; it != end; ++it) {
        SVGElement* referencedElement = *it;
        referencedElement->setCursorImageValue(0);
        if (SVGCursorElement* cursorElement = resourceReferencedByCursorElement(url, referencedElement->document()))
            cursorElement->removeClient(referencedElement);
    }
#endif
}

bool CSSCursorImageValue::updateIfSVGCursorIsUsed(Element* element)
{
#if !ENABLE(SVG)
    UNUSED_PARAM(element);
#else
    if (!element || !element->isSVGElement())
        return false;

    const String& url = getStringValue();
    if (!isSVGCursorIdentifier(url))
        return false;

    if (SVGCursorElement* cursorElement = resourceReferencedByCursorElement(url, element->document())) {
        float x = roundf(cursorElement->x().value(0));
        m_hotspot.setX(static_cast<int>(x));

        float y = roundf(cursorElement->y().value(0));
        m_hotspot.setY(static_cast<int>(y));

        if (cachedImageURL() != element->document()->completeURL(cursorElement->href()))
            clearCachedImage();

        SVGElement* svgElement = static_cast<SVGElement*>(element);
        m_referencedElements.add(svgElement);
        svgElement->setCursorImageValue(this);
        cursorElement->addClient(svgElement);
        return true;
    }
#endif

    return false;
}

StyleCachedImage* CSSCursorImageValue::cachedImage(DocLoader* loader)
{
    String url = getStringValue();

#if ENABLE(SVG) 
    if (isSVGCursorIdentifier(url) && loader && loader->doc()) {
        if (SVGCursorElement* cursorElement = resourceReferencedByCursorElement(url, loader->doc()))
            url = cursorElement->href();
    }
#endif

    return CSSImageValue::cachedImage(loader, url);
}

#if ENABLE(SVG)
void CSSCursorImageValue::removeReferencedElement(SVGElement* element)
{
    m_referencedElements.remove(element);
}
#endif

} // namespace WebCore
