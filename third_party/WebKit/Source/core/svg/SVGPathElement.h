/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
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

#ifndef SVGPathElement_h
#define SVGPathElement_h

#include "SVGNames.h"
#include "core/svg/SVGAnimatedBoolean.h"
#include "core/svg/SVGAnimatedNumber.h"
#include "core/svg/SVGGeometryElement.h"
#include "core/svg/SVGPathByteStream.h"
#include "core/svg/SVGPathSegList.h"

namespace WebCore {

class SVGPathSegArcAbs;
class SVGPathSegArcRel;
class SVGPathSegClosePath;
class SVGPathSegLinetoAbs;
class SVGPathSegLinetoRel;
class SVGPathSegMovetoAbs;
class SVGPathSegMovetoRel;
class SVGPathSegCurvetoCubicAbs;
class SVGPathSegCurvetoCubicRel;
class SVGPathSegLinetoVerticalAbs;
class SVGPathSegLinetoVerticalRel;
class SVGPathSegLinetoHorizontalAbs;
class SVGPathSegLinetoHorizontalRel;
class SVGPathSegCurvetoQuadraticAbs;
class SVGPathSegCurvetoQuadraticRel;
class SVGPathSegCurvetoCubicSmoothAbs;
class SVGPathSegCurvetoCubicSmoothRel;
class SVGPathSegCurvetoQuadraticSmoothAbs;
class SVGPathSegCurvetoQuadraticSmoothRel;
class SVGPathSegListPropertyTearOff;

class SVGPathElement FINAL : public SVGGeometryElement {
public:
    static PassRefPtr<SVGPathElement> create(Document&);

    float getTotalLength();
    SVGPoint getPointAtLength(float distance);
    unsigned getPathSegAtLength(float distance);

    PassRefPtr<SVGPathSegClosePath> createSVGPathSegClosePath(SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegMovetoAbs> createSVGPathSegMovetoAbs(float x, float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegMovetoRel> createSVGPathSegMovetoRel(float x, float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegLinetoAbs> createSVGPathSegLinetoAbs(float x, float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegLinetoRel> createSVGPathSegLinetoRel(float x, float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoCubicAbs> createSVGPathSegCurvetoCubicAbs(float x, float y, float x1, float y1, float x2, float y2, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoCubicRel> createSVGPathSegCurvetoCubicRel(float x, float y, float x1, float y1, float x2, float y2, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoQuadraticAbs> createSVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoQuadraticRel> createSVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegArcAbs> createSVGPathSegArcAbs(float x, float y, float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegArcRel> createSVGPathSegArcRel(float x, float y, float r1, float r2, float angle, bool largeArcFlag, bool sweepFlag, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegLinetoHorizontalAbs> createSVGPathSegLinetoHorizontalAbs(float x, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegLinetoHorizontalRel> createSVGPathSegLinetoHorizontalRel(float x, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegLinetoVerticalAbs> createSVGPathSegLinetoVerticalAbs(float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegLinetoVerticalRel> createSVGPathSegLinetoVerticalRel(float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoCubicSmoothAbs> createSVGPathSegCurvetoCubicSmoothAbs(float x, float y, float x2, float y2, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoCubicSmoothRel> createSVGPathSegCurvetoCubicSmoothRel(float x, float y, float x2, float y2, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoQuadraticSmoothAbs> createSVGPathSegCurvetoQuadraticSmoothAbs(float x, float y, SVGPathSegRole role = PathSegUndefinedRole);
    PassRefPtr<SVGPathSegCurvetoQuadraticSmoothRel> createSVGPathSegCurvetoQuadraticSmoothRel(float x, float y, SVGPathSegRole role = PathSegUndefinedRole);

    // Used in the bindings only.
    SVGPathSegListPropertyTearOff* pathSegList();
    SVGPathSegListPropertyTearOff* animatedPathSegList();
    SVGPathSegListPropertyTearOff* normalizedPathSegList();
    SVGPathSegListPropertyTearOff* animatedNormalizedPathSegList();

    SVGPathByteStream* pathByteStream() const;

    void pathSegListChanged(SVGPathSegRole, ListModification = ListModificationUnknown);

    virtual FloatRect getBBox() OVERRIDE;

    static const SVGPropertyInfo* dPropertyInfo();

    bool isAnimValObserved() const { return m_isAnimValObserved; }

private:
    explicit SVGPathElement(Document&);

    virtual bool supportsFocus() const OVERRIDE { return hasFocusEventListeners(); }

    bool isSupportedAttribute(const QualifiedName&);
    virtual void parseAttribute(const QualifiedName&, const AtomicString&) OVERRIDE;
    virtual void svgAttributeChanged(const QualifiedName&) OVERRIDE;
    virtual bool supportsMarkers() const OVERRIDE { return true; }

    // Custom 'd' property
    static void synchronizeD(SVGElement* contextElement);
    static PassRefPtr<SVGAnimatedProperty> lookupOrCreateDWrapper(SVGElement* contextElement);

    virtual Node::InsertionNotificationRequest insertedInto(ContainerNode*) OVERRIDE;
    virtual void removedFrom(ContainerNode*) OVERRIDE;

    void invalidateMPathDependencies();

    OwnPtr<SVGPathByteStream> m_pathByteStream;
    mutable SVGSynchronizableAnimatedProperty<SVGPathSegList> m_pathSegList;
    bool m_isAnimValObserved;

    BEGIN_DECLARE_ANIMATED_PROPERTIES(SVGPathElement)
        DECLARE_ANIMATED_NUMBER(PathLength, pathLength)
    END_DECLARE_ANIMATED_PROPERTIES
};

DEFINE_NODE_TYPE_CASTS(SVGPathElement, hasTagName(SVGNames::pathTag));

} // namespace WebCore

#endif
