/*
 * Copyright (C) 2006, 2007 Apple Computer, Inc.
 * Copyright (c) 2006, 2007, 2008, 2009, 2012 Google Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "core/platform/graphics/FontPlatformData.h"

#include <windows.h>
#include <mlang.h>
#include <objidl.h>
#include "SkPaint.h"
#include "SkTypeface_win.h"
#include "core/platform/SharedBuffer.h"
#include "core/platform/graphics/FontCache.h"
#include "core/platform/graphics/skia/SkiaFontWin.h"
#include "core/platform/win/HWndDC.h"
#include "public/platform/Platform.h"
#include "public/platform/win/WebSandboxSupport.h"
#include <wtf/StdLibExtras.h>

namespace WebCore {

#if !ENABLE(GDI_FONTS_ON_WINDOWS)
void FontPlatformData::setupPaint(SkPaint* paint) const
{
    const float ts = m_size >= 0 ? m_size : 12;
    paint->setTextSize(SkFloatToScalar(m_size));
    paint->setTypeface(m_typeface);
}
#endif

// Lookup the current system settings for font smoothing.
// We cache these values for performance, but if the browser has a way to be
// notified when these change, we could re-query them at that time.
static uint32_t getDefaultGDITextFlags()
{
    static bool gInited;
    static uint32_t gFlags;
    if (!gInited) {
        BOOL enabled;
        gFlags = 0;
        if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &enabled, 0) && enabled) {
            gFlags |= SkPaint::kAntiAlias_Flag;

            UINT smoothType;
            if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &smoothType, 0)) {
                if (FE_FONTSMOOTHINGCLEARTYPE == smoothType)
                    gFlags |= SkPaint::kLCDRenderText_Flag;
            }
        }
        gInited = true;
    }
    return gFlags;
}

static int computePaintTextFlags(const LOGFONT& lf)
{
    int textFlags = 0;
    switch (lf.lfQuality) {
    case NONANTIALIASED_QUALITY:
        textFlags = 0;
        break;
    case ANTIALIASED_QUALITY:
        textFlags = SkPaint::kAntiAlias_Flag;
        break;
    case CLEARTYPE_QUALITY:
        textFlags = (SkPaint::kAntiAlias_Flag | SkPaint::kLCDRenderText_Flag);
        break;
    default:
        textFlags = getDefaultGDITextFlags();
        break;
    }

    // only allow features that SystemParametersInfo allows
    return textFlags & getDefaultGDITextFlags();
}

SkTypeface* CreateTypefaceFromHFont(HFONT hfont, int* size, int* paintTextFlags)
{
    LOGFONT info;
    GetObject(hfont, sizeof(info), &info);
    if (size) {
        int height = info.lfHeight;
        if (height < 0)
            height = -height;
        *size = height;
    }
    if (paintTextFlags)
        *paintTextFlags = computePaintTextFlags(info);
    return SkCreateTypefaceFromLOGFONT(info);
}

FontPlatformData::FontPlatformData(WTF::HashTableDeletedValueType)
    : m_font(hashTableDeletedFontValue())
    , m_size(-1)
    , m_orientation(Horizontal)
    , m_scriptCache(0)
    , m_scriptFontProperties(0)
    , m_typeface(0)
    , m_paintTextFlags(0)
{
}

FontPlatformData::FontPlatformData()
    : m_font(0)
    , m_size(0)
    , m_orientation(Horizontal)
    , m_scriptCache(0)
    , m_scriptFontProperties(0)
    , m_typeface(0)
    , m_paintTextFlags(0)
{
}

FontPlatformData::FontPlatformData(HFONT font, float size, FontOrientation orientation)
    : m_font(RefCountedHFONT::create(font))
    , m_size(size)
    , m_orientation(orientation)
    , m_scriptCache(0)
    , m_scriptFontProperties(0)
    , m_typeface(CreateTypefaceFromHFont(font, 0, &m_paintTextFlags))
{
}

// FIXME: this constructor is needed for SVG fonts but doesn't seem to do much
FontPlatformData::FontPlatformData(float size, bool bold, bool oblique)
    : m_font(0)
    , m_size(size)
    , m_orientation(Horizontal)
    , m_scriptCache(0)
    , m_scriptFontProperties(0)
    , m_typeface(0)
    , m_paintTextFlags(0)
{
}

FontPlatformData::FontPlatformData(const FontPlatformData& data)
    : m_font(data.m_font)
    , m_size(data.m_size)
    , m_orientation(data.m_orientation)
    , m_scriptCache(0)
    , m_scriptFontProperties(0)
    , m_typeface(data.m_typeface)
    , m_paintTextFlags(data.m_paintTextFlags)
{
    SkSafeRef(m_typeface);
}

FontPlatformData::FontPlatformData(const FontPlatformData& data, float textSize)
    : m_font(data.m_font)
    , m_size(textSize)
    , m_orientation(data.m_orientation)
    , m_scriptCache(0)
    , m_scriptFontProperties(0)
    , m_typeface(data.m_typeface)
    , m_paintTextFlags(data.m_paintTextFlags)
{
    SkSafeRef(m_typeface);
}

FontPlatformData& FontPlatformData::operator=(const FontPlatformData& data)
{
    if (this != &data) {
        m_font = data.m_font;
        m_size = data.m_size;
        m_orientation = data.m_orientation;
        SkRefCnt_SafeAssign(m_typeface, data.m_typeface);
        m_paintTextFlags = data.m_paintTextFlags;

        // The following fields will get re-computed if necessary.
        ScriptFreeCache(&m_scriptCache);
        m_scriptCache = 0;

        delete m_scriptFontProperties;
        m_scriptFontProperties = 0;
    } 
    return *this;
}

FontPlatformData::~FontPlatformData()
{
    SkSafeUnref(m_typeface);

    ScriptFreeCache(&m_scriptCache);
    m_scriptCache = 0;

    delete m_scriptFontProperties;
    m_scriptFontProperties = 0;
}

FontPlatformData::RefCountedHFONT::~RefCountedHFONT()
{
    if (m_hfont != reinterpret_cast<HFONT>(-1)) {
        DeleteObject(m_hfont);
    }
}

FontPlatformData::RefCountedHFONT* FontPlatformData::hashTableDeletedFontValue()
{
    DEFINE_STATIC_LOCAL(RefPtr<RefCountedHFONT>, deletedValue,
                        (RefCountedHFONT::create(reinterpret_cast<HFONT>(-1))));
    return deletedValue.get();
}

SCRIPT_FONTPROPERTIES* FontPlatformData::scriptFontProperties() const
{
    if (!m_scriptFontProperties) {
        m_scriptFontProperties = new SCRIPT_FONTPROPERTIES;
        memset(m_scriptFontProperties, 0, sizeof(SCRIPT_FONTPROPERTIES));
        m_scriptFontProperties->cBytes = sizeof(SCRIPT_FONTPROPERTIES);
        HRESULT result = ScriptGetFontProperties(0, scriptCache(),
                                                 m_scriptFontProperties);
        if (result == E_PENDING) {
            HWndDC dc(0);
            HGDIOBJ oldFont = SelectObject(dc, hfont());
            HRESULT hr = ScriptGetFontProperties(dc, scriptCache(),
                                                 m_scriptFontProperties);
            if (S_OK != hr) {
                if (FontPlatformData::ensureFontLoaded(hfont())) {
                    // FIXME: Handle gracefully the error if this call also fails.
                    hr = ScriptGetFontProperties(dc, scriptCache(),
                                                 m_scriptFontProperties);
                    if (S_OK != hr) {
                        LOG_ERROR("Unable to get the font properties after second attempt");
                    }
                }
            }

            SelectObject(dc, oldFont);
        }
    }
    return m_scriptFontProperties;
}

#if ENABLE(OPENTYPE_VERTICAL)
PassRefPtr<OpenTypeVerticalData> FontPlatformData::verticalData() const
{
    SkFontID id = typeface()->uniqueID();
    return fontCache()->getVerticalData(id, *this);
}

PassRefPtr<SharedBuffer> FontPlatformData::openTypeTable(uint32_t table) const
{
    HWndDC hdc(0);
    HGDIOBJ oldFont = SelectObject(hdc, hfont());

    DWORD size = GetFontData(hdc, table, 0, 0, 0);
    RefPtr<SharedBuffer> buffer;
    if (size != GDI_ERROR) {
        buffer = SharedBuffer::create(size);
        DWORD result = GetFontData(hdc, table, 0, (PVOID)buffer->data(), size);
        ASSERT(result == size);
    }

    SelectObject(hdc, oldFont);
    return buffer.release();
}
#endif

#ifndef NDEBUG
String FontPlatformData::description() const
{
    return String();
}
#endif

bool FontPlatformData::ensureFontLoaded(HFONT font)
{
    WebKit::WebSandboxSupport* sandboxSupport = WebKit::Platform::current()->sandboxSupport();
    // if there is no sandbox, then we can assume the font
    // was able to be loaded successfully already
    return sandboxSupport ? sandboxSupport->ensureFontLoaded(font) : true;
}

}
