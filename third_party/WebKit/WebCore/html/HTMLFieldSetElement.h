/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
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

#ifndef HTMLFieldSetElement_h
#define HTMLFieldSetElement_h

#include "HTMLFormControlElement.h"

namespace WebCore {
    class RenderStyle;
}

namespace WebCore {

class HTMLFormElement;
class Document;
class Node;

class HTMLFieldSetElement : public HTMLFormControlElement {
public:
    HTMLFieldSetElement(const QualifiedName&, Document*, HTMLFormElement* = 0);
    virtual ~HTMLFieldSetElement();
    
    virtual int tagPriority() const { return 3; }
    virtual bool checkDTD(const Node* newChild);

    virtual bool isFocusable() const;
    virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);
    virtual const AtomicString& type() const;

    virtual bool willValidate() const { return false; }
};

} //namespace

#endif
