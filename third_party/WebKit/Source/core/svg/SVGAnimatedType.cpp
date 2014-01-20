/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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
#include "core/svg/SVGAnimatedType.h"

#include "bindings/v8/ExceptionState.h"
#include "core/svg/SVGParserUtilities.h"
#include "core/svg/SVGPathByteStream.h"

namespace WebCore {

SVGAnimatedType::SVGAnimatedType(AnimatedPropertyType type)
    : m_type(type)
{
}

SVGAnimatedType::~SVGAnimatedType()
{
    switch (m_type) {
    case AnimatedAngle:
        delete m_data.angleAndEnumeration;
        break;
    case AnimatedColor:
        delete m_data.color;
        break;
    case AnimatedEnumeration:
        delete m_data.enumeration;
        break;
    case AnimatedInteger:
        delete m_data.integer;
        break;
    case AnimatedIntegerOptionalInteger:
        delete m_data.integerOptionalInteger;
        break;
    case AnimatedNumber:
        delete m_data.number;
        break;
    case AnimatedNumberList:
        delete m_data.numberList;
        break;
    case AnimatedNumberOptionalNumber:
        delete m_data.numberOptionalNumber;
        break;
    case AnimatedPath:
        delete m_data.path;
        break;
    case AnimatedPoints:
        delete m_data.pointList;
        break;
    case AnimatedPreserveAspectRatio:
        delete m_data.preserveAspectRatio;
        break;
    case AnimatedString:
        delete m_data.string;
        break;
    case AnimatedTransformList:
        delete m_data.transformList;
        break;
    // Below properties are migrated to new property implementation.
    case AnimatedBoolean:
    case AnimatedLength:
    case AnimatedLengthList:
    case AnimatedRect:
        // handled by RefPtr
        break;
    case AnimatedUnknown:
        ASSERT_NOT_REACHED();
        break;
    }
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createAngleAndEnumeration(std::pair<SVGAngle, unsigned>* angleAndEnumeration)
{
    ASSERT(angleAndEnumeration);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedAngle));
    animatedType->m_data.angleAndEnumeration = angleAndEnumeration;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createColor(StyleColor* color)
{
    ASSERT(color);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedColor));
    animatedType->m_data.color = color;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createEnumeration(unsigned* enumeration)
{
    ASSERT(enumeration);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedEnumeration));
    animatedType->m_data.enumeration = enumeration;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createInteger(int* integer)
{
    ASSERT(integer);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedInteger));
    animatedType->m_data.integer = integer;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createIntegerOptionalInteger(pair<int, int>* integerOptionalInteger)
{
    ASSERT(integerOptionalInteger);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedIntegerOptionalInteger));
    animatedType->m_data.integerOptionalInteger = integerOptionalInteger;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createNumber(float* number)
{
    ASSERT(number);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedNumber));
    animatedType->m_data.number = number;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createNumberList(SVGNumberList* numberList)
{
    ASSERT(numberList);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedNumberList));
    animatedType->m_data.numberList = numberList;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createNumberOptionalNumber(pair<float, float>* numberOptionalNumber)
{
    ASSERT(numberOptionalNumber);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedNumberOptionalNumber));
    animatedType->m_data.numberOptionalNumber = numberOptionalNumber;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createPath(PassOwnPtr<SVGPathByteStream> path)
{
    ASSERT(path);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedPath));
    animatedType->m_data.path = path.leakPtr();
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createPointList(SVGPointList* pointList)
{
    ASSERT(pointList);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedPoints));
    animatedType->m_data.pointList = pointList;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createPreserveAspectRatio(SVGPreserveAspectRatio* preserveAspectRatio)
{
    ASSERT(preserveAspectRatio);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedPreserveAspectRatio));
    animatedType->m_data.preserveAspectRatio = preserveAspectRatio;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createString(String* string)
{
    ASSERT(string);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedString));
    animatedType->m_data.string = string;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createTransformList(SVGTransformList* transformList)
{
    ASSERT(transformList);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(AnimatedTransformList));
    animatedType->m_data.transformList = transformList;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedType::createNewProperty(PassRefPtr<NewSVGPropertyBase> newProperty)
{
    ASSERT(newProperty);
    OwnPtr<SVGAnimatedType> animatedType = adoptPtr(new SVGAnimatedType(newProperty->type()));
    animatedType->m_newProperty = newProperty;
    return animatedType.release();
}

String SVGAnimatedType::valueAsString()
{
    switch (m_type) {
    case AnimatedColor:
        ASSERT(m_data.color);
        return m_data.color->isCurrentColor() ? "currentColor" : m_data.color->color().serializedAsCSSComponentValue();
    case AnimatedNumber:
        ASSERT(m_data.number);
        return String::number(*m_data.number);
    case AnimatedString:
        ASSERT(m_data.string);
        return *m_data.string;

    // Below properties have migrated to new property implementation.
    case AnimatedLength:
    case AnimatedLengthList:
    case AnimatedRect:
        return m_newProperty->valueAsString();

    // These types don't appear in the table in SVGElement::cssPropertyToTypeMap() and thus don't need valueAsString() support.
    case AnimatedAngle:
    case AnimatedBoolean:
    case AnimatedEnumeration:
    case AnimatedInteger:
    case AnimatedIntegerOptionalInteger:
    case AnimatedNumberList:
    case AnimatedNumberOptionalNumber:
    case AnimatedPath:
    case AnimatedPoints:
    case AnimatedPreserveAspectRatio:
    case AnimatedTransformList:
    case AnimatedUnknown:
        // Only SVG DOM animations use these property types - that means valueAsString() is never used for those.
        ASSERT_NOT_REACHED();
        break;
    }
    ASSERT_NOT_REACHED();
    return String();
}

bool SVGAnimatedType::setValueAsString(const QualifiedName& attrName, const String& value)
{
    switch (m_type) {
    case AnimatedColor:
        ASSERT(m_data.color);
        *m_data.color = value.isEmpty() ? StyleColor::currentColor() : SVGColor::colorFromRGBColorString(value);
        break;
    case AnimatedNumber:
        ASSERT(m_data.number);
        parseNumberFromString(value, *m_data.number);
        break;
    case AnimatedString:
        ASSERT(m_data.string);
        *m_data.string = value;
        break;

    // Below properties have migrated to new property implementation.
    case AnimatedLength:
    case AnimatedLengthList:
    case AnimatedRect:
        // Always use createForAnimation call path for these implementations.
        return false;

    // These types don't appear in the table in SVGElement::cssPropertyToTypeMap() and thus don't need setValueAsString() support.
    case AnimatedAngle:
    case AnimatedBoolean:
    case AnimatedEnumeration:
    case AnimatedInteger:
    case AnimatedIntegerOptionalInteger:
    case AnimatedNumberList:
    case AnimatedNumberOptionalNumber:
    case AnimatedPath:
    case AnimatedPoints:
    case AnimatedPreserveAspectRatio:
    case AnimatedTransformList:
    case AnimatedUnknown:
        // Only SVG DOM animations use these property types - that means setValueAsString() is never used for those.
        ASSERT_NOT_REACHED();
        break;
    }
    return true;
}

bool SVGAnimatedType::supportsAnimVal(AnimatedPropertyType type)
{
    // AnimatedColor is only used for CSS property animations.
    return type != AnimatedUnknown && type != AnimatedColor;
}

} // namespace WebCore
