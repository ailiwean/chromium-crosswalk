/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
#include "SVGTextChunk.h"

#include "SVGInlineTextBox.h"
#include "SVGTextFragment.h"

namespace WebCore {

SVGTextChunk::SVGTextChunk(bool isVerticalText, ETextAnchor textAnchor, SVGTextContentElement::SVGLengthAdjustType lengthAdjust, float desiredTextLength)
    : m_isVerticalText(isVerticalText)
    , m_textAnchor(textAnchor)
    , m_lengthAdjust(lengthAdjust)
    , m_desiredTextLength(desiredTextLength)
{
}

void SVGTextChunk::calculateLength(float& length, unsigned& characters) const
{
    SVGTextFragment* lastFragment = 0;

    unsigned boxCount = m_boxes.size();
    for (unsigned boxPosition = 0; boxPosition < boxCount; ++boxPosition) {
        SVGInlineTextBox* textBox = m_boxes.at(boxPosition);
        Vector<SVGTextFragment>& fragments = textBox->textFragments();

        unsigned size = fragments.size();
        if (!size)
            continue;

        for (unsigned i = 0; i < size; ++i) {
            SVGTextFragment& fragment = fragments.at(i);
            characters += fragment.length;

            if (m_isVerticalText)
                length += fragment.height;
            else
                length += fragment.width;

            if (!lastFragment) {
                lastFragment = &fragment;
                continue;
            }

            // Resepect gap between chunks.
            if (m_isVerticalText)
                 length += fragment.y - (lastFragment->y + lastFragment->height);
            else
                 length += fragment.x - (lastFragment->x + lastFragment->width);

            lastFragment = &fragment;
        }
    }
}

float SVGTextChunk::calculateTextAnchorShift(float length) const
{
    switch (m_textAnchor) {
    case TA_START:
        return 0;
    case TA_MIDDLE:
        return -length / 2;
    case TA_END:
        return -length;
    };

    ASSERT_NOT_REACHED();
    return 0;
}

} // namespace WebCore

#endif // ENABLE(SVG)
