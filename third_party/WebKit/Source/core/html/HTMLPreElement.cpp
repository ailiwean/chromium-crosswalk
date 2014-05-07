/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2010 Apple Inc. All rights reserved.
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
 *
 */

#include "config.h"
#include "core/html/HTMLPreElement.h"

#include "CSSPropertyNames.h"
#include "CSSValueKeywords.h"
#include "HTMLNames.h"
#include "core/css/StylePropertySet.h"
#include "core/frame/UseCounter.h"

namespace WebCore {

using namespace HTMLNames;

inline HTMLPreElement::HTMLPreElement(const QualifiedName& tagName, Document& document)
    : HTMLElement(tagName, document)
{
    ScriptWrappable::init(this);
}

PassRefPtrWillBeRawPtr<HTMLPreElement> HTMLPreElement::create(const QualifiedName& tagName, Document& document)
{
    return adoptRefWillBeRefCountedGarbageCollected(new HTMLPreElement(tagName, document));
}

bool HTMLPreElement::isPresentationAttribute(const QualifiedName& name) const
{
    if (name == wrapAttr)
        return true;
    return HTMLElement::isPresentationAttribute(name);
}

void HTMLPreElement::collectStyleForPresentationAttribute(const QualifiedName& name, const AtomicString& value, MutableStylePropertySet* style)
{
    if (name == wrapAttr) {
        UseCounter::count(document(), UseCounter::HTMLPreElementWrap);
        style->setProperty(CSSPropertyWhiteSpace, CSSValuePreWrap);
    } else {
        HTMLElement::collectStyleForPresentationAttribute(name, value, style);
    }
}

}
