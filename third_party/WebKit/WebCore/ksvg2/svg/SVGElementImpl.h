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

#ifndef KSVG_SVGElementImpl_H
#define KSVG_SVGElementImpl_H
#if SVG_SUPPORT

#include "SVGNames.h"
#include <kdom/core/XMLElementImpl.h>

namespace WebCore {
    class SVGMatrixImpl;
    class SVGSVGElementImpl;
    class SVGStyledElementImpl;
    class Ecma;
    class DocumentPtr;
    typedef StyledElementImpl XMLElementImpl;

    class SVGElementImpl : public XMLElementImpl
    {
    public:
        SVGElementImpl(const QualifiedName& tagName, DocumentImpl *doc);
        virtual ~SVGElementImpl();
        virtual bool isSVGElement() const { return true; }
        virtual bool isSupported(DOMStringImpl *feature, DOMStringImpl *version) const;

        SVGSVGElementImpl *ownerSVGElement() const;
        SVGElementImpl *viewportElement() const;

        // Helper methods that returns the attr value if attr is set, otherwise the default value.
        // It throws NO_MODIFICATION_ALLOWED_ERR if the element is read-only.
        AtomicString tryGetAttribute(const DOMString& name, AtomicString defaultValue = AtomicString()) const;
        AtomicString tryGetAttributeNS(const DOMString& namespaceURI, const DOMString& localName, AtomicString defaultValue = AtomicString()) const;

        // Internal
        virtual void parseMappedAttribute(MappedAttributeImpl *attr);

        // To be implemented by any element which can establish new viewports...
        virtual QString adjustViewportClipping() const { return QString::null; }
        
        virtual bool isStyled() const { return false; }
        virtual bool isStyledTransformable() const { return false; }
        virtual bool isStyledLocatable() const { return false; }
        virtual bool isSVG() const { return false; }
        virtual bool isFilterEffect() const { return false; }
        virtual bool isGradientStop() const { return false; }
        
        // For SVGTestsImpl
        virtual bool isValid() const { return true; }
        
        virtual void closeRenderer() { m_closed = true; }
        virtual bool rendererIsNeeded(RenderStyle *) { return false; }
        virtual bool childShouldCreateRenderer(DOM::NodeImpl *) const;
        
        // helper:
        bool isClosed() const { return m_closed; }

    private:
        bool m_closed;
        void addSVGEventListener(const AtomicString& eventType, const AttributeImpl* attr);
    };
};

namespace WebCore {
    static inline SVGElementImpl *svg_dynamic_cast(NodeImpl *node) {
        SVGElementImpl *svgElement = NULL;
        if (node && node->isSVGElement())
            svgElement = static_cast<SVGElementImpl *>(node);
        return svgElement;
    }
};

#endif // SVG_SUPPORT
#endif

// vim:ts=4:noet
