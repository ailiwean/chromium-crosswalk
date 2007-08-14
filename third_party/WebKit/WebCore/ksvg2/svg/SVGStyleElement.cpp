/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
    Copyright (C) 2006 Apple Computer, Inc.

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
#if ENABLE(SVG)
#include "SVGStyleElement.h"

#include "CSSStyleSheet.h"
#include "Document.h"
#include "ExceptionCode.h"
#include "HTMLNames.h"
#include "XMLNames.h"

namespace WebCore {

using namespace HTMLNames;

SVGStyleElement::SVGStyleElement(const QualifiedName& tagName, Document* doc)
     : SVGElement(tagName, doc)
     , m_createdByParser(false)
{
}

const AtomicString& SVGStyleElement::xmlspace() const
{
    return getAttribute(XMLNames::spaceAttr);
}

void SVGStyleElement::setXmlspace(const AtomicString&, ExceptionCode& ec)
{
    ec = NO_MODIFICATION_ALLOWED_ERR;
}

const AtomicString& SVGStyleElement::type() const
{
    static const AtomicString defaultValue("text/css");
    const AtomicString& n = getAttribute(typeAttr);
    return n.isNull() ? defaultValue : n;
}

void SVGStyleElement::setType(const AtomicString&, ExceptionCode& ec)
{
    ec = NO_MODIFICATION_ALLOWED_ERR;
}

const AtomicString& SVGStyleElement::media() const
{
    static const AtomicString defaultValue("all");
    const AtomicString& n = getAttribute(mediaAttr);
    return n.isNull() ? defaultValue : n;
}

void SVGStyleElement::setMedia(const AtomicString&, ExceptionCode& ec)
{
    ec = NO_MODIFICATION_ALLOWED_ERR;
}

String SVGStyleElement::title() const
{
    return getAttribute(titleAttr);
}

void SVGStyleElement::setTitle(const AtomicString&, ExceptionCode& ec)
{
    ec = NO_MODIFICATION_ALLOWED_ERR;
}

void SVGStyleElement::parseMappedAttribute(MappedAttribute* attr)
{
    if (attr->name() == titleAttr && m_sheet)
        m_sheet->setTitle(attr->value());
    else
        SVGElement::parseMappedAttribute(attr);
}

void SVGStyleElement::finishedParsing()
{
    StyleElement::process(this);
    SVGElement::finishedParsing();
}

void SVGStyleElement::insertedIntoDocument()
{
    SVGElement::insertedIntoDocument();

    if (!m_createdByParser)
        StyleElement::insertedIntoDocument(document(), this);
}

void SVGStyleElement::removedFromDocument()
{
    SVGElement::removedFromDocument();
    StyleElement::removedFromDocument(document());
}

void SVGStyleElement::childrenChanged()
{
    StyleElement::process(this);
}

StyleSheet* SVGStyleElement::sheet()
{
    return StyleElement::sheet(this);
}

bool SVGStyleElement::sheetLoaded()
{
    document()->removePendingSheet();
    return true;
}

}

// vim:ts=4:noet
#endif // ENABLE(SVG)
