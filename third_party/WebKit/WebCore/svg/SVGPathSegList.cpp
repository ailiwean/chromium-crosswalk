/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#include "config.h"

#if ENABLE(SVG)
#include "SVGPathSegList.h"

#include "FloatConversion.h"
#include "FloatPoint.h"
#include "FloatSize.h"
#include "Path.h"
#include "PathTraversalState.h"
#include "SVGPathSegArc.h"
#include "SVGPathSegClosePath.h"
#include "SVGPathSegMoveto.h"
#include "SVGPathSegLineto.h"
#include "SVGPathSegLinetoHorizontal.h"
#include "SVGPathSegLinetoVertical.h"
#include "SVGPathSegCurvetoCubic.h"
#include "SVGPathSegCurvetoCubicSmooth.h"
#include "SVGPathSegCurvetoQuadratic.h"
#include "SVGPathSegCurvetoQuadraticSmooth.h"

namespace WebCore {

SVGPathSegList::SVGPathSegList(const QualifiedName& attributeName)
    : SVGList<RefPtr<SVGPathSeg> >(attributeName)
{
}

SVGPathSegList::~SVGPathSegList()
{
}

unsigned SVGPathSegList::getPathSegAtLength(double length, ExceptionCode& ec)
{
    // FIXME : to be useful this will need to support non-normalized SVGPathSegLists
    int len = numberOfItems();
    // FIXME: Eventually this will likely move to a "path applier"-like model, until then PathTraversalState is less useful as we could just use locals
    PathTraversalState traversalState(PathTraversalState::TraversalSegmentAtLength);
    traversalState.m_desiredLength = narrowPrecisionToFloat(length);
    for (int i = 0; i < len; ++i) {
        SVGPathSeg* segment = getItem(i, ec).get();
        if (ec)
            return 0;
        float segmentLength = 0;
        switch (segment->pathSegType()) {
        case PathSegMoveToAbs:
        {
            SVGPathSegMovetoAbs* moveTo = static_cast<SVGPathSegMovetoAbs*>(segment);
            segmentLength = traversalState.moveTo(FloatPoint(moveTo->x(), moveTo->y()));
            break;
        }
        case PathSegLineToAbs:
        {
            SVGPathSegLinetoAbs* lineTo = static_cast<SVGPathSegLinetoAbs*>(segment);
            segmentLength = traversalState.lineTo(FloatPoint(lineTo->x(), lineTo->y()));
            break;
        }
        case PathSegCurveToCubicAbs:
        {
            SVGPathSegCurvetoCubicAbs* curveTo = static_cast<SVGPathSegCurvetoCubicAbs*>(segment);
            segmentLength = traversalState.cubicBezierTo(FloatPoint(curveTo->x1(), curveTo->y1()),
                                      FloatPoint(curveTo->x2(), curveTo->y2()),
                                      FloatPoint(curveTo->x(), curveTo->y()));
            break;
        }
        case PathSegClosePath:
            segmentLength = traversalState.closeSubpath();
            break;
        default:
            ASSERT(false); // FIXME: This only works with normalized/processed path data.
            break;
        }
        traversalState.m_totalLength += segmentLength;
        if ((traversalState.m_action == PathTraversalState::TraversalSegmentAtLength)
            && (traversalState.m_totalLength >= traversalState.m_desiredLength)) {
            return traversalState.m_segmentIndex;
        }
        traversalState.m_segmentIndex++;
    }

    // The SVG spec is unclear as to what to return when the distance is not on the path.
    // WebKit/Opera/FF all return the last path segment if the distance exceeds the actual path length:
    return traversalState.m_segmentIndex ? traversalState.m_segmentIndex - 1 : 0;
}
    
float adjustAnimatedValue(float from, float to, float progress)
{
    return (to - from) * progress + from;
}
    
#define BLENDPATHSEG1(class, attr1) \
    class::create(adjustAnimatedValue(static_cast<class*>(from)->attr1(), static_cast<class*>(to)->attr1(), progress))
    
#define BLENDPATHSEG2(class, attr1, attr2) \
    class::create(adjustAnimatedValue(static_cast<class*>(from)->attr1(), static_cast<class*>(to)->attr1(), progress), \
                    adjustAnimatedValue(static_cast<class*>(from)->attr2(), static_cast<class*>(to)->attr2(), progress))
    
#define BLENDPATHSEG4(class, attr1, attr2, attr3, attr4) \
    class::create(adjustAnimatedValue(static_cast<class*>(from)->attr1(), static_cast<class*>(to)->attr1(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr2(), static_cast<class*>(to)->attr2(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr3(), static_cast<class*>(to)->attr3(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr4(), static_cast<class*>(to)->attr4(), progress))
    
#define BLENDPATHSEG6(class, attr1, attr2, attr3, attr4, attr5, attr6) \
    class::create(adjustAnimatedValue(static_cast<class*>(from)->attr1(), static_cast<class*>(to)->attr1(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr2(), static_cast<class*>(to)->attr2(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr3(), static_cast<class*>(to)->attr3(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr4(), static_cast<class*>(to)->attr4(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr5(), static_cast<class*>(to)->attr5(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr6(), static_cast<class*>(to)->attr6(), progress))

#define BLENDPATHSEG7(class, attr1, attr2, attr3, attr4, attr5, bool1, bool2) \
    class::create(adjustAnimatedValue(static_cast<class*>(from)->attr1(), static_cast<class*>(to)->attr1(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr2(), static_cast<class*>(to)->attr2(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr3(), static_cast<class*>(to)->attr3(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr4(), static_cast<class*>(to)->attr4(), progress), \
                adjustAnimatedValue(static_cast<class*>(from)->attr5(), static_cast<class*>(to)->attr5(), progress), \
                static_cast<bool>(adjustAnimatedValue(static_cast<class*>(from)->bool1(), static_cast<class*>(to)->bool1(), progress)), \
                static_cast<bool>(adjustAnimatedValue(static_cast<class*>(from)->bool2(), static_cast<class*>(to)->bool2(), progress)))

PassRefPtr<SVGPathSegList> SVGPathSegList::createAnimated(const SVGPathSegList* fromList, const SVGPathSegList* toList, float progress)
{
    unsigned itemCount = fromList->numberOfItems();
    if (!itemCount || itemCount != toList->numberOfItems())
        return 0;
    RefPtr<SVGPathSegList> result = create(fromList->associatedAttributeName());
    ExceptionCode ec = 0;
    for (unsigned n = 0; n < itemCount; ++n) {
        SVGPathSeg* from = fromList->getItem(n, ec).get();
        if (ec)
            return 0;
        SVGPathSeg* to = toList->getItem(n, ec).get();
        if (ec)
            return 0;
        if (from->pathSegType() == PathSegUnknown || from->pathSegType() != to->pathSegType())
            return 0;
        RefPtr<SVGPathSeg> segment = 0;
        switch (static_cast<SVGPathSegType>(from->pathSegType())) {
        case PathSegClosePath:
            segment = SVGPathSegClosePath::create();
            break;
        case PathSegLineToHorizontalAbs:
            segment = BLENDPATHSEG1(SVGPathSegLinetoHorizontalAbs, x);
            break;
        case PathSegLineToHorizontalRel:
            segment = BLENDPATHSEG1(SVGPathSegLinetoHorizontalRel, x);
            break;   
        case PathSegLineToVerticalAbs:
            segment = BLENDPATHSEG1(SVGPathSegLinetoVerticalAbs, y);
            break;
        case PathSegLineToVerticalRel:
            segment = BLENDPATHSEG1(SVGPathSegLinetoVerticalRel, y);
            break;        
        case PathSegMoveToAbs:
            segment = BLENDPATHSEG2(SVGPathSegMovetoAbs, x, y);
            break;
        case PathSegMoveToRel:
            segment = BLENDPATHSEG2(SVGPathSegMovetoRel, x, y);
            break;
        case PathSegLineToAbs:
            segment = BLENDPATHSEG2(SVGPathSegLinetoAbs, x, y);
            break;
        case PathSegLineToRel:
            segment = BLENDPATHSEG2(SVGPathSegLinetoRel, x, y);
            break;
        case PathSegCurveToCubicAbs:
            segment = BLENDPATHSEG6(SVGPathSegCurvetoCubicAbs, x, y, x1, y1, x2, y2);
            break;
        case PathSegCurveToCubicRel:
            segment = BLENDPATHSEG6(SVGPathSegCurvetoCubicRel, x, y, x1, y1, x2, y2);
            break;
        case PathSegCurveToCubicSmoothAbs:
            segment = BLENDPATHSEG4(SVGPathSegCurvetoCubicSmoothAbs, x, y, x2, y2);
            break;
        case PathSegCurveToCubicSmoothRel:
            segment = BLENDPATHSEG4(SVGPathSegCurvetoCubicSmoothRel, x, y, x2, y2);
            break;
        case PathSegCurveToQuadraticAbs:
            segment = BLENDPATHSEG4(SVGPathSegCurvetoQuadraticAbs, x, y, x1, y1);
            break;
        case PathSegCurveToQuadraticRel:
            segment = BLENDPATHSEG4(SVGPathSegCurvetoQuadraticRel, x, y, x1, y1);
            break;
        case PathSegCurveToQuadraticSmoothAbs:
            segment = BLENDPATHSEG2(SVGPathSegCurvetoQuadraticSmoothAbs, x, y);
            break;
        case PathSegCurveToQuadraticSmoothRel:
            segment = BLENDPATHSEG2(SVGPathSegCurvetoQuadraticSmoothRel, x, y);
            break;
        case PathSegArcAbs:
            segment = BLENDPATHSEG7(SVGPathSegArcAbs, x, y, r1, r2, angle, largeArcFlag, sweepFlag);
            break;
        case PathSegArcRel:
            segment = BLENDPATHSEG7(SVGPathSegArcRel, x, y, r1, r2, angle, largeArcFlag, sweepFlag);
            break;
        case PathSegUnknown:
            ASSERT_NOT_REACHED();
        }
        result->appendItem(segment, ec);
        if (ec)
            return 0;
    }
    return result.release();
}

}

#endif // ENABLE(SVG)
