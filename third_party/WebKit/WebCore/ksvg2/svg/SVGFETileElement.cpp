/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>

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
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "config.h"
#ifdef SVG_SUPPORT
#include "Attr.h"

#include <kcanvas/KCanvasFilters.h>
#include <kcanvas/device/KRenderingDevice.h>

#include "SVGHelper.h"
#include "SVGRenderStyle.h"
#include "SVGFETileElement.h"

namespace WebCore {

SVGFETileElement::SVGFETileElement(const QualifiedName& tagName, Document *doc)
    : SVGFilterPrimitiveStandardAttributes(tagName, doc)
    , m_filterEffect(0)
{
}

SVGFETileElement::~SVGFETileElement()
{
    delete m_filterEffect;
}

ANIMATED_PROPERTY_DEFINITIONS(SVGFETileElement, String, String, string, In, in, SVGNames::inAttr.localName(), m_in)

void SVGFETileElement::parseMappedAttribute(MappedAttribute *attr)
{
    const String& value = attr->value();
    if (attr->name() == SVGNames::inAttr)
        setInBaseValue(value);
    else
        SVGFilterPrimitiveStandardAttributes::parseMappedAttribute(attr);
}

KCanvasFETile *SVGFETileElement::filterEffect() const
{
    if (!m_filterEffect)
        m_filterEffect = static_cast<KCanvasFETile *>(renderingDevice()->createFilterEffect(FE_TILE));
    if (!m_filterEffect)
        return 0;
    m_filterEffect->setIn(in());
    setStandardAttributes(m_filterEffect);
    return m_filterEffect;
}

}

// vim:ts=4:noet
#endif // SVG_SUPPORT

