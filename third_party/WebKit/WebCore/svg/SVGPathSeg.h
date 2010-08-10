/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2008 Rob Buis <buis@kde.org>
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

#ifndef SVGPathSeg_h
#define SVGPathSeg_h

#if ENABLE(SVG)
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>

namespace WebCore {

enum SVGPathSegType {
    PathSegUnknown = 0,
    PathSegClosePath = 1,
    PathSegMoveToAbs = 2,
    PathSegMoveToRel = 3,
    PathSegLineToAbs = 4,
    PathSegLineToRel = 5,
    PathSegCurveToCubicAbs = 6,
    PathSegCurveToCubicRel = 7,
    PathSegCurveToQuadraticAbs = 8,
    PathSegCurveToQuadraticRel = 9,
    PathSegArcAbs = 10,
    PathSegArcRel = 11,
    PathSegLineToHorizontalAbs = 12,
    PathSegLineToHorizontalRel = 13,
    PathSegLineToVerticalAbs = 14,
    PathSegLineToVerticalRel = 15,
    PathSegCurveToCubicSmoothAbs = 16,
    PathSegCurveToCubicSmoothRel = 17,
    PathSegCurveToQuadraticSmoothAbs = 18,
    PathSegCurveToQuadraticSmoothRel = 19
};

class SVGPathElement;
class SVGStyledElement;
class QualifiedName;

class SVGPathSeg : public RefCounted<SVGPathSeg> {
public:
    virtual ~SVGPathSeg();

    // Forward declare these enums in the w3c naming scheme, for IDL generation
    enum {
        PATHSEG_UNKNOWN = PathSegUnknown,
        PATHSEG_CLOSEPATH = PathSegClosePath,
        PATHSEG_MOVETO_ABS = PathSegMoveToAbs,
        PATHSEG_MOVETO_REL = PathSegMoveToRel,
        PATHSEG_LINETO_ABS = PathSegLineToAbs,
        PATHSEG_LINETO_REL = PathSegLineToRel,
        PATHSEG_CURVETO_CUBIC_ABS = PathSegCurveToCubicAbs,
        PATHSEG_CURVETO_CUBIC_REL = PathSegCurveToCubicRel,
        PATHSEG_CURVETO_QUADRATIC_ABS = PathSegCurveToQuadraticAbs,
        PATHSEG_CURVETO_QUADRATIC_REL = PathSegCurveToQuadraticRel,
        PATHSEG_ARC_ABS = PathSegArcAbs,
        PATHSEG_ARC_REL = PathSegArcRel,
        PATHSEG_LINETO_HORIZONTAL_ABS = PathSegLineToHorizontalAbs,
        PATHSEG_LINETO_HORIZONTAL_REL = PathSegLineToHorizontalRel,
        PATHSEG_LINETO_VERTICAL_ABS = PathSegLineToVerticalAbs,
        PATHSEG_LINETO_VERTICAL_REL = PathSegLineToVerticalRel,
        PATHSEG_CURVETO_CUBIC_SMOOTH_ABS = PathSegCurveToCubicSmoothAbs,
        PATHSEG_CURVETO_CUBIC_SMOOTH_REL = PathSegCurveToCubicSmoothRel,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = PathSegCurveToQuadraticSmoothAbs,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = PathSegCurveToQuadraticSmoothRel
    };


    virtual unsigned short pathSegType() const;
    virtual String pathSegTypeAsLetter() const;
    virtual String toString() const;

    const QualifiedName& associatedAttributeName() const;
    
protected:
    SVGPathSeg() { }
};

class SVGPathSegSingleCoord : public SVGPathSeg { 
public:
    SVGPathSegSingleCoord(float x, float y)
        : m_x(x)
        , m_y(y)
    {
    }

    void setX(float x) { m_x = x; }
    float x() const { return m_x; }

    void setY(float y) { m_y = y; }
    float y() const { return m_y; }

    virtual String toString() const;

private:
    float m_x;
    float m_y;
};

} // namespace WebCore

#endif // ENABLE(SVG)
#endif
