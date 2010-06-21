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

#ifndef WKContext_h
#define WKContext_h

#include <WebKit2/WKBase.h>

#ifdef __cplusplus
extern "C" {
#endif

// Policy Client.
typedef void (*WKContextDidRecieveMessageFromInjectedBundleCallback)(WKContextRef page, WKStringRef message, const void *clientInfo);

struct WKContextInjectedBundleClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKContextDidRecieveMessageFromInjectedBundleCallback                didRecieveMessageFromInjectedBundle;
};
typedef struct WKContextInjectedBundleClient WKContextInjectedBundleClient;

WK_EXPORT WKContextRef WKContextCreate();
WK_EXPORT WKContextRef WKContextCreateWithInjectedBundlePath(WKStringRef path);
WK_EXPORT WKContextRef WKContextGetSharedProcessContext();

WK_EXPORT void WKContextSetPreferences(WKContextRef context, WKPreferencesRef preferences);
WK_EXPORT WKPreferencesRef WKContextGetPreferences(WKContextRef context);

WK_EXPORT void WKContextSetInjectedBundleClient(WKContextRef context, WKContextInjectedBundleClient * client);

WK_EXPORT void WKContextPostMessageToInjectedBundle(WKContextRef context, WKStringRef message);

WK_EXPORT WKContextRef WKContextRetain(WKContextRef context);
WK_EXPORT void WKContextRelease(WKContextRef context);

#ifdef __cplusplus
}
#endif

WK_DECLARE_RETAIN_RELEASE_OVERLOADS(WKContext)

#endif /* WKContext_h */
