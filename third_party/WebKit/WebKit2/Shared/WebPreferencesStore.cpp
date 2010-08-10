/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebPreferencesStore.h"

#include "FontSmoothingLevel.h"

namespace WebKit {

WebPreferencesStore::WebPreferencesStore()
    : javaScriptEnabled(true)
    , loadsImagesAutomatically(true)
    , pluginsEnabled(true)
    , offlineWebApplicationCacheEnabled(false)
    , localStorageEnabled(true)
    , fontSmoothingLevel(FontSmoothingLevelMedium)
    , minimumFontSize(1)
    , minimumLogicalFontSize(9)
    , defaultFontSize(16)
    , defaultFixedFontSize(13)
    , standardFontFamily("Times")
    , cursiveFontFamily("Apple Chancery")
    , fantasyFontFamily("Papyrus")
    , fixedFontFamily("Courier")
    , sansSerifFontFamily("Helvetica")
    , serifFontFamily("Times")
{
}

WebPreferencesStore::WebPreferencesStore(const WebPreferencesStore& other)
{
    javaScriptEnabled = other.javaScriptEnabled;
    loadsImagesAutomatically = other.loadsImagesAutomatically;
    pluginsEnabled = other.pluginsEnabled;
    offlineWebApplicationCacheEnabled = other.offlineWebApplicationCacheEnabled;
    localStorageEnabled = other.localStorageEnabled;
    fontSmoothingLevel = other.fontSmoothingLevel;
    minimumFontSize = other.minimumFontSize;
    minimumLogicalFontSize = other.minimumLogicalFontSize;
    defaultFontSize = other.defaultFontSize;
    defaultFixedFontSize = other.defaultFixedFontSize;
    standardFontFamily = other.standardFontFamily;
    cursiveFontFamily = other.cursiveFontFamily;
    fantasyFontFamily = other.fantasyFontFamily;
    fixedFontFamily = other.fixedFontFamily;
    sansSerifFontFamily = other.sansSerifFontFamily;
    serifFontFamily = other.serifFontFamily;

}

WebPreferencesStore& WebPreferencesStore::operator=(const WebPreferencesStore& other)
{
    WebPreferencesStore copy = other;
    swap(copy);
    return *this;
}

void WebPreferencesStore::swap(WebPreferencesStore& other)
{
    std::swap(javaScriptEnabled, other.javaScriptEnabled);
    std::swap(loadsImagesAutomatically, other.loadsImagesAutomatically);
    std::swap(pluginsEnabled, other.pluginsEnabled);
    std::swap(offlineWebApplicationCacheEnabled, other.offlineWebApplicationCacheEnabled);
    std::swap(localStorageEnabled, other.localStorageEnabled);
    std::swap(fontSmoothingLevel, other.fontSmoothingLevel);
    std::swap(minimumFontSize, other.minimumFontSize);
    std::swap(minimumLogicalFontSize, other.minimumLogicalFontSize);
    std::swap(defaultFontSize, other.defaultFontSize);
    std::swap(defaultFixedFontSize, other.defaultFixedFontSize);
    WTF::swap(standardFontFamily, other.standardFontFamily);
    WTF::swap(cursiveFontFamily, other.cursiveFontFamily);
    WTF::swap(fantasyFontFamily, other.fantasyFontFamily);
    WTF::swap(fixedFontFamily, other.fixedFontFamily);
    WTF::swap(sansSerifFontFamily, other.sansSerifFontFamily);
    WTF::swap(serifFontFamily, other.serifFontFamily);
}

} // namespace WebKit
