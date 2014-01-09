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

#include "core/svg/SVGAnimatedColor.h"

#include "core/rendering/RenderObject.h"
#include "core/svg/ColorDistance.h"
#include "core/svg/SVGAnimateElement.h"
#include "core/svg/SVGColor.h"

namespace WebCore {

SVGAnimatedColorAnimator::SVGAnimatedColorAnimator(SVGAnimationElement* animationElement, SVGElement* contextElement)
    : SVGAnimatedTypeAnimator(AnimatedColor, animationElement, contextElement)
{
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedColorAnimator::constructFromString(const String& string)
{
    OwnPtr<SVGAnimatedType> animtedType = SVGAnimatedType::createColor(new Color);
    animtedType->color() = string.isEmpty() ? Color() : SVGColor::colorFromRGBColorString(string);
    return animtedType.release();
}

static inline void adjustForCurrentColor(SVGElement* targetElement, Color& color)
{
    ASSERT(targetElement);
    if (color.isValid())
        return;

    if (RenderObject* targetRenderer = targetElement->renderer())
        color = targetRenderer->style()->visitedDependentColor(CSSPropertyColor);
    else
        color = Color();
}

void SVGAnimatedColorAnimator::addAnimatedTypes(SVGAnimatedType* from, SVGAnimatedType* to)
{
    ASSERT(from->type() == AnimatedColor);
    ASSERT(from->type() == to->type());
    Color fromColor = from->color();
    Color toColor = to->color();
    adjustForCurrentColor(m_contextElement, fromColor);
    adjustForCurrentColor(m_contextElement, toColor);
    to->color() = ColorDistance::addColors(fromColor, toColor);
}

static Color parseColorFromString(SVGAnimationElement*, const String& string)
{
    return SVGColor::colorFromRGBColorString(string);
}

void SVGAnimatedColorAnimator::calculateAnimatedValue(float percentage, unsigned repeatCount, SVGAnimatedType* from, SVGAnimatedType* to, SVGAnimatedType* toAtEndOfDuration, SVGAnimatedType* animated)
{
    ASSERT(m_animationElement);
    ASSERT(m_contextElement);

    Color fromColor = m_animationElement->animationMode() == ToAnimation ? animated->color() : from->color();
    Color toColor = to->color();
    Color toAtEndOfDurationColor = toAtEndOfDuration->color();
    Color& animatedColor = animated->color();

    // Apply CSS inheritance rules.
    m_animationElement->adjustForInheritance<Color>(parseColorFromString, m_animationElement->fromPropertyValueType(), fromColor, m_contextElement);
    m_animationElement->adjustForInheritance<Color>(parseColorFromString, m_animationElement->toPropertyValueType(), toColor, m_contextElement);

    // Apply <animateColor> rules.
    adjustForCurrentColor(m_contextElement, fromColor);
    adjustForCurrentColor(m_contextElement, toColor);
    adjustForCurrentColor(m_contextElement, toAtEndOfDurationColor);

    float animatedRed = animatedColor.red();
    m_animationElement->animateAdditiveNumber(percentage, repeatCount, fromColor.red(), toColor.red(), toAtEndOfDurationColor.red(), animatedRed);

    float animatedGreen = animatedColor.green();
    m_animationElement->animateAdditiveNumber(percentage, repeatCount, fromColor.green(), toColor.green(), toAtEndOfDurationColor.green(), animatedGreen);

    float animatedBlue = animatedColor.blue();
    m_animationElement->animateAdditiveNumber(percentage, repeatCount, fromColor.blue(), toColor.blue(), toAtEndOfDurationColor.blue(), animatedBlue);

    float animatedAlpha = animatedColor.alpha();
    m_animationElement->animateAdditiveNumber(percentage, repeatCount, fromColor.alpha(), toColor.alpha(), toAtEndOfDurationColor.alpha(), animatedAlpha);

    animatedColor = makeRGBA(static_cast<int>(roundf(animatedRed)), static_cast<int>(roundf(animatedGreen)), static_cast<int>(roundf(animatedBlue)), static_cast<int>(roundf(animatedAlpha)));
}

float SVGAnimatedColorAnimator::calculateDistance(const String& fromString, const String& toString)
{
    ASSERT(m_contextElement);
    Color from = SVGColor::colorFromRGBColorString(fromString);
    if (!from.isValid())
        return -1;
    Color to = SVGColor::colorFromRGBColorString(toString);
    if (!to.isValid())
        return -1;
    return ColorDistance::distance(from, to);
}

}
