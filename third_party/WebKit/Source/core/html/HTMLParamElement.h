/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2006, 2010 Apple Inc. All rights reserved.
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

#ifndef HTMLParamElement_h
#define HTMLParamElement_h

#include "core/html/HTMLElement.h"

namespace WebCore {

class HTMLParamElement FINAL : public HTMLElement {
public:
    static PassRefPtr<HTMLParamElement> create(Document&);

    const AtomicString& name() const;
    const AtomicString& value() const;

    static bool isURLParameter(const String&);

private:
    explicit HTMLParamElement(Document&);

    virtual bool isURLAttribute(const Attribute&) const OVERRIDE;

    virtual void addSubresourceAttributeURLs(ListHashSet<KURL>&) const;
};

DEFINE_NODE_TYPE_CASTS(HTMLParamElement, hasTagName(HTMLNames::paramTag));

} // namespace WebCore

#endif
