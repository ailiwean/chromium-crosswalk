/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2006 Apple Computer, Inc.
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

#include "HTMLElement.h"

namespace WebCore {

class HTMLParamElement : public HTMLElement {
    friend class HTMLAppletElement;
public:
    HTMLParamElement(const QualifiedName&, Document*);
    ~HTMLParamElement();

    virtual HTMLTagStatus endTagRequirement() const { return TagStatusForbidden; }
    virtual int tagPriority() const { return 0; }

    virtual void parseMappedAttribute(MappedAttribute*);

    virtual bool isURLAttribute(Attribute*) const;

    String name() const { return m_name; }
    void setName(const String&);

    String type() const;
    void setType(const String&);

    String value() const { return m_value; }
    void setValue(const String&);

    String valueType() const;
    void setValueType(const String&);

    virtual void addSubresourceAttributeURLs(ListHashSet<KURL>&) const;

 protected:
    AtomicString m_name;
    AtomicString m_value;
};


}

#endif
