/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2009 Michelangelo De Simone <micdesim@gmail.com>
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
 *
 */

#include "config.h"
#include "ValidityState.h"
#include "HTMLFormControlElement.h"

namespace WebCore {

ValidityState::ValidityState(HTMLFormControlElement* parent)
    : m_control(parent)
{
    ASSERT(parent);
}

bool ValidityState::valid()
{
    bool someError = typeMismatch() || stepMismatch() || rangeUnderflow() || rangeOverflow() ||
                       tooLong() || patternMismatch() || valueMissing() || customError();

    return !someError;
}

} // namespace
