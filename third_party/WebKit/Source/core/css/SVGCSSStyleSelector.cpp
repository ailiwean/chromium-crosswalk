/*
    Copyright (C) 2005 Apple Computer, Inc.
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2008 Rob Buis <buis@kde.org>
    Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>

    Based on khtml css code by:
    Copyright(C) 1999-2003 Lars Knoll(knoll@kde.org)
             (C) 2003 Apple Computer, Inc.
             (C) 2004 Allan Sandfeld Jensen(kde@carewolf.com)
             (C) 2004 Germain Garand(germain@ebooksfrance.org)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "core/css/resolver/StyleResolver.h"

#include "CSSPropertyNames.h"
#include "core/css/CSSPrimitiveValueMappings.h"
#include "core/css/CSSValueList.h"
#include "core/css/ShadowValue.h"
#include "core/rendering/style/SVGRenderStyle.h"
#include "core/rendering/style/SVGRenderStyleDefs.h"
#include "core/svg/SVGColor.h"
#include "core/svg/SVGPaint.h"
#include "core/svg/SVGURIReference.h"
#include "wtf/MathExtras.h"

#define HANDLE_SVG_INHERIT(prop, Prop) \
if (isInherit) { \
    style->accessSVGStyle()->set##Prop(state.parentStyle()->svgStyle()->prop()); \
    return; \
}

#define HANDLE_SVG_INHERIT_AND_INITIAL(prop, Prop) \
HANDLE_SVG_INHERIT(prop, Prop) \
if (isInitial) { \
    style->accessSVGStyle()->set##Prop(SVGRenderStyle::initial##Prop()); \
    return; \
}

namespace WebCore {

static float roundToNearestGlyphOrientationAngle(float angle)
{
    angle = fabsf(fmodf(angle, 360.0f));

    if (angle <= 45.0f || angle > 315.0f)
        return 0.0f;
    else if (angle > 45.0f && angle <= 135.0f)
        return 90.0f;
    else if (angle > 135.0f && angle <= 225.0f)
        return 180.0f;

    return 270.0f;
}

static int angleToGlyphOrientation(float angle)
{
    angle = roundToNearestGlyphOrientationAngle(angle);

    if (angle == 0.0f)
        return GO_0DEG;
    else if (angle == 90.0f)
        return GO_90DEG;
    else if (angle == 180.0f)
        return GO_180DEG;
    else if (angle == 270.0f)
        return GO_270DEG;

    return -1;
}

static Color colorFromSVGColorCSSValue(SVGColor* svgColor, const Color& fgColor)
{
    Color color;
    if (svgColor->colorType() == SVGColor::SVG_COLORTYPE_CURRENTCOLOR)
        color = fgColor;
    else
        color = svgColor->color();
    return color;
}

static bool percentageOrNumberToFloat(const CSSPrimitiveValue* primitiveValue, float& out)
{
    int type = primitiveValue->primitiveType();
    if (type == CSSPrimitiveValue::CSS_PERCENTAGE) {
        out = primitiveValue->getFloatValue() / 100.0f;
        return true;
    }
    if (type == CSSPrimitiveValue::CSS_NUMBER) {
        out = primitiveValue->getFloatValue();
        return true;
    }
    return false;
}

static String fragmentIdentifier(const CSSPrimitiveValue* primitiveValue, Document* document)
{
    if (primitiveValue->primitiveType() != CSSPrimitiveValue::CSS_URI)
        return String();
    return SVGURIReference::fragmentIdentifierFromIRIString(primitiveValue->getStringValue(), document);
}

void StyleResolver::applySVGProperty(CSSPropertyID id, CSSValue* value)
{
    ASSERT(value);
    CSSPrimitiveValue* primitiveValue = 0;
    if (value->isPrimitiveValue())
        primitiveValue = toCSSPrimitiveValue(value);

    // FIXME: This should be a const StyleResolverState, but
    // unfortunately resolveColorFromPrimitiveValue
    // has side-effects and is thus not const.
    StyleResolverState& state = m_state;
    RenderStyle* style = state.style();

    bool isInherit = state.parentNode() && value->isInheritedValue();
    bool isInitial = value->isInitialValue() || (!state.parentNode() && value->isInheritedValue());

    // What follows is a list that maps the CSS properties into their
    // corresponding front-end RenderStyle values. Shorthands(e.g. border,
    // background) occur in this list as well and are only hit when mapping
    // "inherit" or "initial" into front-end values.
    switch (id)
    {
        // ident only properties
        case CSSPropertyAlignmentBaseline:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(alignmentBaseline, AlignmentBaseline)
            if (!primitiveValue)
                break;

            style->accessSVGStyle()->setAlignmentBaseline(*primitiveValue);
            break;
        }
        case CSSPropertyBaselineShift:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(baselineShift, BaselineShift);
            if (!primitiveValue)
                break;

            SVGRenderStyle* svgStyle = style->accessSVGStyle();
            if (primitiveValue->getValueID()) {
                switch (primitiveValue->getValueID()) {
                case CSSValueBaseline:
                    svgStyle->setBaselineShift(BS_BASELINE);
                    break;
                case CSSValueSub:
                    svgStyle->setBaselineShift(BS_SUB);
                    break;
                case CSSValueSuper:
                    svgStyle->setBaselineShift(BS_SUPER);
                    break;
                default:
                    break;
                }
            } else {
                svgStyle->setBaselineShift(BS_LENGTH);
                svgStyle->setBaselineShiftValue(SVGLength::fromCSSPrimitiveValue(primitiveValue));
            }

            break;
        }
        case CSSPropertyKerning:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(kerning, Kerning);
            if (primitiveValue)
                style->accessSVGStyle()->setKerning(SVGLength::fromCSSPrimitiveValue(primitiveValue));
            break;
        }
        case CSSPropertyDominantBaseline:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(dominantBaseline, DominantBaseline)
            if (primitiveValue)
                style->accessSVGStyle()->setDominantBaseline(*primitiveValue);
            break;
        }
        case CSSPropertyColorInterpolation:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(colorInterpolation, ColorInterpolation)
            if (primitiveValue)
                style->accessSVGStyle()->setColorInterpolation(*primitiveValue);
            break;
        }
        case CSSPropertyColorInterpolationFilters:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(colorInterpolationFilters, ColorInterpolationFilters)
            if (primitiveValue)
                style->accessSVGStyle()->setColorInterpolationFilters(*primitiveValue);
            break;
        }
        case CSSPropertyColorProfile:
        {
            // Not implemented.
            break;
        }
        case CSSPropertyColorRendering:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(colorRendering, ColorRendering)
            if (primitiveValue)
                style->accessSVGStyle()->setColorRendering(*primitiveValue);
            break;
        }
        case CSSPropertyClipRule:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(clipRule, ClipRule)
            if (primitiveValue)
                style->accessSVGStyle()->setClipRule(*primitiveValue);
            break;
        }
        case CSSPropertyFillRule:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(fillRule, FillRule)
            if (primitiveValue)
                style->accessSVGStyle()->setFillRule(*primitiveValue);
            break;
        }
        case CSSPropertyStrokeLinejoin:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(joinStyle, JoinStyle)
            if (primitiveValue)
                style->accessSVGStyle()->setJoinStyle(*primitiveValue);
            break;
        }
        case CSSPropertyShapeRendering:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(shapeRendering, ShapeRendering)
            if (primitiveValue)
                style->accessSVGStyle()->setShapeRendering(*primitiveValue);
            break;
        }
        // end of ident only properties
        case CSSPropertyFill:
        {
            SVGRenderStyle* svgStyle = style->accessSVGStyle();
            if (isInherit) {
                const SVGRenderStyle* svgParentStyle = state.parentStyle()->svgStyle();
                svgStyle->setFillPaint(svgParentStyle->fillPaintType(), svgParentStyle->fillPaintColor(), svgParentStyle->fillPaintUri(), state.applyPropertyToRegularStyle(), state.applyPropertyToVisitedLinkStyle());
                return;
            }
            if (isInitial) {
                svgStyle->setFillPaint(SVGRenderStyle::initialFillPaintType(), SVGRenderStyle::initialFillPaintColor(), SVGRenderStyle::initialFillPaintUri(), state.applyPropertyToRegularStyle(), state.applyPropertyToVisitedLinkStyle());
                return;
            }
            if (value->isSVGPaint()) {
                SVGPaint* svgPaint = static_cast<SVGPaint*>(value);
                svgStyle->setFillPaint(svgPaint->paintType(), colorFromSVGColorCSSValue(svgPaint, state.style()->color()), svgPaint->uri(), state.applyPropertyToRegularStyle(), state.applyPropertyToVisitedLinkStyle());
            }
            break;
        }
        case CSSPropertyStroke:
        {
            SVGRenderStyle* svgStyle = style->accessSVGStyle();
            if (isInherit) {
                const SVGRenderStyle* svgParentStyle = state.parentStyle()->svgStyle();
                svgStyle->setStrokePaint(svgParentStyle->strokePaintType(), svgParentStyle->strokePaintColor(), svgParentStyle->strokePaintUri(), state.applyPropertyToRegularStyle(), state.applyPropertyToVisitedLinkStyle());
                return;
            }
            if (isInitial) {
                svgStyle->setStrokePaint(SVGRenderStyle::initialStrokePaintType(), SVGRenderStyle::initialStrokePaintColor(), SVGRenderStyle::initialStrokePaintUri(), state.applyPropertyToRegularStyle(), state.applyPropertyToVisitedLinkStyle());
                return;
            }
            if (value->isSVGPaint()) {
                SVGPaint* svgPaint = static_cast<SVGPaint*>(value);
                svgStyle->setStrokePaint(svgPaint->paintType(), colorFromSVGColorCSSValue(svgPaint, state.style()->color()), svgPaint->uri(), state.applyPropertyToRegularStyle(), state.applyPropertyToVisitedLinkStyle());
            }
            break;
        }
        case CSSPropertyStrokeWidth:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(strokeWidth, StrokeWidth)
            if (primitiveValue)
                style->accessSVGStyle()->setStrokeWidth(SVGLength::fromCSSPrimitiveValue(primitiveValue));
            break;
        }
        case CSSPropertyStrokeDasharray:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(strokeDashArray, StrokeDashArray)
            if (!value->isValueList()) {
                style->accessSVGStyle()->setStrokeDashArray(SVGRenderStyle::initialStrokeDashArray());
                break;
            }

            CSSValueList* dashes = toCSSValueList(value);

            Vector<SVGLength> array;
            size_t length = dashes->length();
            for (size_t i = 0; i < length; ++i) {
                CSSValue* currValue = dashes->itemWithoutBoundsCheck(i);
                if (!currValue->isPrimitiveValue())
                    continue;

                CSSPrimitiveValue* dash = toCSSPrimitiveValue(dashes->itemWithoutBoundsCheck(i));
                array.append(SVGLength::fromCSSPrimitiveValue(dash));
            }

            style->accessSVGStyle()->setStrokeDashArray(array);
            break;
        }
        case CSSPropertyStrokeDashoffset:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(strokeDashOffset, StrokeDashOffset)
            if (primitiveValue)
                style->accessSVGStyle()->setStrokeDashOffset(SVGLength::fromCSSPrimitiveValue(primitiveValue));
            break;
        }
        case CSSPropertyFillOpacity:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(fillOpacity, FillOpacity)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            if (percentageOrNumberToFloat(primitiveValue, f))
                style->accessSVGStyle()->setFillOpacity(f);
            break;
        }
        case CSSPropertyStrokeOpacity:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(strokeOpacity, StrokeOpacity)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            if (percentageOrNumberToFloat(primitiveValue, f))
                style->accessSVGStyle()->setStrokeOpacity(f);
            break;
        }
        case CSSPropertyStopOpacity:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(stopOpacity, StopOpacity)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            if (percentageOrNumberToFloat(primitiveValue, f))
                style->accessSVGStyle()->setStopOpacity(f);
            break;
        }
        case CSSPropertyMarkerStart:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(markerStartResource, MarkerStartResource)
            if (!primitiveValue)
                return;
            style->accessSVGStyle()->setMarkerStartResource(fragmentIdentifier(primitiveValue, state.document()));
            break;
        }
        case CSSPropertyMarkerMid:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(markerMidResource, MarkerMidResource)
            if (!primitiveValue)
                return;
            style->accessSVGStyle()->setMarkerMidResource(fragmentIdentifier(primitiveValue, state.document()));
            break;
        }
        case CSSPropertyMarkerEnd:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(markerEndResource, MarkerEndResource)
            if (!primitiveValue)
                return;
            style->accessSVGStyle()->setMarkerEndResource(fragmentIdentifier(primitiveValue, state.document()));
            break;
        }
        case CSSPropertyStrokeLinecap:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(capStyle, CapStyle)
            if (primitiveValue)
                style->accessSVGStyle()->setCapStyle(*primitiveValue);
            break;
        }
        case CSSPropertyStrokeMiterlimit:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(strokeMiterLimit, StrokeMiterLimit)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_NUMBER)
                f = primitiveValue->getFloatValue();
            else
                return;

            style->accessSVGStyle()->setStrokeMiterLimit(f);
            break;
        }
        case CSSPropertyFilter:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(filterResource, FilterResource)
            if (!primitiveValue)
                return;
            style->accessSVGStyle()->setFilterResource(fragmentIdentifier(primitiveValue, state.document()));
            break;
        }
        case CSSPropertyMask:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(maskerResource, MaskerResource)
            if (!primitiveValue)
                return;
            style->accessSVGStyle()->setMaskerResource(fragmentIdentifier(primitiveValue, state.document()));
            break;
        }
        case CSSPropertyClipPath:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(clipperResource, ClipperResource)
            if (!primitiveValue)
                return;
            style->accessSVGStyle()->setClipperResource(fragmentIdentifier(primitiveValue, state.document()));
            break;
        }
        case CSSPropertyTextAnchor:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(textAnchor, TextAnchor)
            if (primitiveValue)
                style->accessSVGStyle()->setTextAnchor(*primitiveValue);
            break;
        }
        case CSSPropertyWritingMode:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(writingMode, WritingMode)
            if (primitiveValue)
                style->accessSVGStyle()->setWritingMode(*primitiveValue);
            break;
        }
        case CSSPropertyStopColor:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(stopColor, StopColor);
            if (value->isSVGColor())
                style->accessSVGStyle()->setStopColor(colorFromSVGColorCSSValue(static_cast<SVGColor*>(value), state.style()->color()));
            break;
        }
       case CSSPropertyLightingColor:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(lightingColor, LightingColor);
            if (value->isSVGColor())
                style->accessSVGStyle()->setLightingColor(colorFromSVGColorCSSValue(static_cast<SVGColor*>(value), state.style()->color()));
            break;
        }
        case CSSPropertyFloodOpacity:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(floodOpacity, FloodOpacity)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            if (percentageOrNumberToFloat(primitiveValue, f))
                style->accessSVGStyle()->setFloodOpacity(f);
            break;
        }
        case CSSPropertyFloodColor:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(floodColor, FloodColor);
            if (value->isSVGColor())
                style->accessSVGStyle()->setFloodColor(colorFromSVGColorCSSValue(static_cast<SVGColor*>(value), state.style()->color()));
            break;
        }
        case CSSPropertyGlyphOrientationHorizontal:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(glyphOrientationHorizontal, GlyphOrientationHorizontal)
            if (!primitiveValue)
                return;

            if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_DEG) {
                int orientation = angleToGlyphOrientation(primitiveValue->getFloatValue());
                ASSERT(orientation != -1);

                style->accessSVGStyle()->setGlyphOrientationHorizontal((EGlyphOrientation) orientation);
            }

            break;
        }
        case CSSPropertyGlyphOrientationVertical:
        {
            HANDLE_SVG_INHERIT_AND_INITIAL(glyphOrientationVertical, GlyphOrientationVertical)
            if (!primitiveValue)
                return;

            if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_DEG) {
                int orientation = angleToGlyphOrientation(primitiveValue->getFloatValue());
                ASSERT(orientation != -1);

                style->accessSVGStyle()->setGlyphOrientationVertical(static_cast<EGlyphOrientation>(orientation));
            } else if (primitiveValue->getValueID() == CSSValueAuto) {
                style->accessSVGStyle()->setGlyphOrientationVertical(GO_AUTO);
            }
            break;
        }
        case CSSPropertyEnableBackground:
            // Silently ignoring this property for now
            // http://bugs.webkit.org/show_bug.cgi?id=6022
            break;
        case CSSPropertyWebkitSvgShadow: {
            if (isInherit)
                return style->accessSVGStyle()->setShadow(adoptPtr(state.parentStyle()->svgStyle()->shadow() ? new ShadowData(*state.parentStyle()->svgStyle()->shadow()) : 0));
            if (isInitial || primitiveValue) // initial | none
                return style->accessSVGStyle()->setShadow(nullptr);

            if (!value->isValueList())
                return;

            CSSValueList* list = toCSSValueList(value);
            if (!list->length())
                return;

            CSSValue* firstValue = list->itemWithoutBoundsCheck(0);
            if (!firstValue->isShadowValue())
                return;
            ShadowValue* item = static_cast<ShadowValue*>(firstValue);
            IntPoint location(item->x->computeLength<int>(state.style(), state.rootElementStyle()),
                item->y->computeLength<int>(state.style(), state.rootElementStyle()));
            int blur = item->blur ? item->blur->computeLength<int>(state.style(), state.rootElementStyle()) : 0;
            Color color;
            if (item->color)
                color = state.resolveColorFromPrimitiveValue(item->color.get());

            // -webkit-svg-shadow does should not have a spread or style
            ASSERT(!item->spread);
            ASSERT(!item->style);

            OwnPtr<ShadowData> shadowData = adoptPtr(new ShadowData(location, blur, 0, Normal, color.isValid() ? color : Color::transparent));
            style->accessSVGStyle()->setShadow(shadowData.release());
            return;
        }
        case CSSPropertyVectorEffect: {
            HANDLE_SVG_INHERIT_AND_INITIAL(vectorEffect, VectorEffect)
            if (!primitiveValue)
                break;

            style->accessSVGStyle()->setVectorEffect(*primitiveValue);
            break;
        }
        case CSSPropertyBufferedRendering: {
            HANDLE_SVG_INHERIT_AND_INITIAL(bufferedRendering, BufferedRendering)
            if (!primitiveValue)
                break;

            style->accessSVGStyle()->setBufferedRendering(*primitiveValue);
            break;
        }
        case CSSPropertyMaskType: {
            HANDLE_SVG_INHERIT_AND_INITIAL(maskType, MaskType)
            if (!primitiveValue)
                break;

            style->accessSVGStyle()->setMaskType(*primitiveValue);
            break;
        }
        default:
            // If you crash here, it's because you added a css property and are not handling it
            // in either this switch statement or the one in StyleResolver::applyProperty
            ASSERT_WITH_MESSAGE(0, "unimplemented propertyID: %d", id);
            return;
    }
}

}
