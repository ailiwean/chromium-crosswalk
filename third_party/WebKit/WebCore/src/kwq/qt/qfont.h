/*
 * Copyright (C) 2001 Apple Computer, Inc.  All rights reserved.
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

#ifndef QFONT_H_
#define QFONT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class QString;
class QPainter;

#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
#define Fixed MacFixed
#define Rect MacRect
#define Boolean MacBoolean
#import <Cocoa/Cocoa.h>
#undef Fixed
#undef Rect
#undef Boolean
#endif

// class QFont =================================================================

class QFont {
public:

    // typedefs ----------------------------------------------------------------
    // enums -------------------------------------------------------------------

    enum CharSet { Latin1, Unicode };
    enum Weight { Normal = 50, Bold = 63 };

    // constants ---------------------------------------------------------------
    // static member functions -------------------------------------------------

    // constructors, copy constructors, and destructors ------------------------

    QFont();
    QFont(const QFont &);

    ~QFont();

    // member functions --------------------------------------------------------

    int pixelSize() const;
    QString family() const;
    void setFamily(const QString &);
    void setPixelSize(int);
    void setPixelSizeFloat(float);
    void setWeight(int);
    int weight() const;
    bool setItalic(bool);
    bool italic() const;
    bool bold() const;

    // operators ---------------------------------------------------------------

    QFont &operator=(const QFont &);
    bool operator==(const QFont &x) const;
    bool operator!=(const QFont &x) const;

#ifdef _KWQ_
#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
    static NSFont *QFont::defaultNSFont();
#endif
#endif

#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
        NSFont *getFont();
#else
        void *getFont();
#endif

// protected -------------------------------------------------------------------
// private ---------------------------------------------------------------------
private:
#ifdef _KWQ_
    void _initialize();
    void _initializeWithFont(const QFont *);
    void _free();
#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
    void _setTrait (NSFontTraitMask mask);
#endif
    
#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
        NSFont *font;
        NSString *_family;
        int _trait;
        float _size;
#else
        void *font;
        void *_family;
        int _trait;
        float _size;
#endif

#endif

}; // class QFont ==============================================================

#endif
