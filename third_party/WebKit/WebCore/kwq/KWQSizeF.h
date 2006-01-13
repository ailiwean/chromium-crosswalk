/*
 * Copyright (C) 2003, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2005 Nokia.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef QSIZEF_H_
#define QSIZEF_H_

#include "KWQDef.h"

// workaround for <rdar://problem/4294625>
#if ! __LP64__ && ! NS_BUILD_32_LIKE_64
#undef NSGEOMETRY_TYPES_SAME_AS_CGGEOMETRY_TYPES
#endif

#ifdef NSGEOMETRY_TYPES_SAME_AS_CGGEOMETRY_TYPES
typedef struct CGSize NSSize;
#else
typedef struct _NSSize NSSize;
#endif
typedef struct CGSize CGSize;

namespace WebCore {
    class IntSize;
}

class QSizeF {
public:
    QSizeF();
    QSizeF(float, float);
    QSizeF(const WebCore::IntSize&);
#ifndef NSGEOMETRY_TYPES_SAME_AS_CGGEOMETRY_TYPES
    explicit QSizeF(const NSSize&);
#endif
    explicit QSizeF(const CGSize&);

    bool isValid() const;
    float width() const { return w; }
    float height() const { return h; }
    void setWidth(float width) { w = width; }
    void setHeight(float height) { h = height; }
    QSizeF expandedTo(const QSizeF&) const;

#ifndef NSGEOMETRY_TYPES_SAME_AS_CGGEOMETRY_TYPES
    operator NSSize() const;
#endif
    operator CGSize() const;

    friend QSizeF operator+(const QSizeF&, const QSizeF&);
    friend bool operator==(const QSizeF&, const QSizeF&);
    friend bool operator!=(const QSizeF&, const QSizeF&);

private:
    float w;
    float h;
};

#endif
