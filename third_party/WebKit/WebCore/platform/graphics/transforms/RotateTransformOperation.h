/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#ifndef RotateTransformOperation_h
#define RotateTransformOperation_h

#include "TransformOperation.h"

namespace WebCore {

class RotateTransformOperation : public TransformOperation {
public:
    static PassRefPtr<RotateTransformOperation> create(double angle, OperationType type)
    {
        return adoptRef(new RotateTransformOperation(angle, type));
    }

    virtual bool isIdentity() const { return m_angle == 0; }
    virtual OperationType getOperationType() const { return m_type; }
    virtual bool isSameType(const TransformOperation& o) const { return o.getOperationType() == m_type; }

    virtual bool operator==(const TransformOperation& o) const
    {
        if (!isSameType(o))
            return false;
        const RotateTransformOperation* r = static_cast<const RotateTransformOperation*>(&o);
        return m_angle == r->m_angle;
    }

    virtual bool apply(TransformationMatrix& transform, const IntSize& /*borderBoxSize*/) const
    {
        transform.rotate(m_angle);
        return false;
    }

    virtual PassRefPtr<TransformOperation> blend(const TransformOperation* from, double progress, bool blendToIdentity = false);

    double angle() const { return m_angle; }

private:
    RotateTransformOperation(double angle, OperationType type)
        : m_angle(angle)
        , m_type(type)
    {
    }

    double m_angle;
    OperationType m_type;
};

} // namespace WebCore

#endif // RotateTransformOperation_h
