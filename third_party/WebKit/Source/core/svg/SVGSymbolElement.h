/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#ifndef SVGSymbolElement_h
#define SVGSymbolElement_h

#include "core/svg/SVGAnimatedBoolean.h"
#include "core/svg/SVGAnimatedPreserveAspectRatio.h"
#include "core/svg/SVGAnimatedRect.h"
#include "core/svg/SVGElement.h"
#include "core/svg/SVGFitToViewBox.h"

namespace WebCore {

class SVGSymbolElement FINAL : public SVGElement,
                               public SVGFitToViewBox {
public:
    static PassRefPtr<SVGSymbolElement> create(Document&);

    SVGAnimatedRect* viewBox() const { return m_viewBox.get(); }

private:
    explicit SVGSymbolElement(Document&);

    virtual bool supportsFocus() const OVERRIDE { return hasFocusEventListeners(); }

    bool isSupportedAttribute(const QualifiedName&);
    virtual void parseAttribute(const QualifiedName&, const AtomicString&) OVERRIDE;
    virtual void svgAttributeChanged(const QualifiedName&) OVERRIDE;

    virtual RenderObject* createRenderer(RenderStyle*) OVERRIDE;

    virtual bool selfHasRelativeLengths() const OVERRIDE;

    RefPtr<SVGAnimatedRect> m_viewBox;
    BEGIN_DECLARE_ANIMATED_PROPERTIES(SVGSymbolElement)
        DECLARE_ANIMATED_PRESERVEASPECTRATIO(PreserveAspectRatio, preserveAspectRatio)
    END_DECLARE_ANIMATED_PROPERTIES
};

} // namespace WebCore

#endif
