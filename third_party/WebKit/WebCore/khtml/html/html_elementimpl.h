/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#ifndef HTML_ELEMENTIMPL_H
#define HTML_ELEMENTIMPL_H

#include "xml/dom_elementimpl.h"
#include "dom_atomicstringlist.h"

namespace DOM {

class DOMString;
class CSSStyleDeclarationImpl;
class HTMLFormElementImpl;
class DocumentFragmentImpl;

class HTMLNamedAttrMapImpl : public NamedAttrMapImpl
{
public:
    HTMLNamedAttrMapImpl(ElementImpl *e);

    virtual void clearAttributes();
    
    virtual bool isHTMLAttributeMap() const;
    
    virtual void parseClassAttribute(const DOMString& classAttr);
    bool matchesCSSClass(const AtomicString& c, bool caseSensitive) const;
    
private:
    AtomicStringList m_classList;
};
    
class HTMLElementImpl : public ElementImpl
{
public:
    HTMLElementImpl(DocumentPtr *doc);

    virtual ~HTMLElementImpl();

    virtual bool isHTMLElement() const { return true; }

    virtual bool isInline() const;
     
    virtual Id id() const = 0;

    virtual void parseAttribute(AttributeImpl *token);
    virtual void createAttributeMap() const;

    virtual bool matchesCSSClass(const AtomicString& c, bool caseSensitive) const;

    void addCSSLength(int id, const DOMString &value); // FIXME: value will be parsed by the CSS parser
    void addCSSProperty(int id, const DOMString &value); // value will be parsed by the CSS parser
    void addCSSProperty(int id, int value);
    void addCSSStringProperty(int id, const DOMString &value, DOM::CSSPrimitiveValue::UnitTypes = DOM::CSSPrimitiveValue::CSS_STRING);
    void addCSSImageProperty(int id, const DOMString &URL);
    void addHTMLColor( int id, const DOMString &c );
    void removeCSSProperty(int id);

    DOMString innerHTML() const;
    DOMString innerText() const;
    DocumentFragmentImpl *createContextualFragment( const DOMString &html );
    bool setInnerHTML( const DOMString &html );
    bool setInnerText( const DOMString &text );

    virtual DOMString namespaceURI() const;
    
    virtual bool isFocusable() const;
    virtual bool isContentEditable() const;
    virtual DOMString contentEditable() const;
    virtual void setContentEditable(const DOMString &enabled);

    virtual void click();
    
#if APPLE_CHANGES
    virtual bool isGenericFormElement() const { return false; }
#endif

    virtual DOMString toString() const;

protected:
    // for IMG, OBJECT and APPLET
    void addHTMLAlignment( const DOMString& alignment );
};

class HTMLGenericElementImpl : public HTMLElementImpl
{
public:
    HTMLGenericElementImpl(DocumentPtr *doc, ushort i);

    virtual ~HTMLGenericElementImpl();

    virtual Id id() const { return _id; };

protected:
    ushort _id;
};

}; //namespace

#endif
