/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "config.h"
#include "CookieJar.h"

#if USE(CFNETWORK)

#include "Cookie.h"
#include "CookieStorageCFNet.h"
#include "Document.h"
#include "KURL.h"
#include "PlatformString.h"
#include "ResourceHandle.h"
#include "SoftLinking.h"
#include <CFNetwork/CFHTTPCookiesPriv.h>
#include <CoreFoundation/CoreFoundation.h>

#if PLATFORM(WIN)
#include <WebKitSystemInterface/WebKitSystemInterface.h>
#include <windows.h>
#endif

namespace WebCore {

static const CFStringRef s_setCookieKeyCF = CFSTR("Set-Cookie");
static const CFStringRef s_cookieCF = CFSTR("Cookie");

#if PLATFORM(WIN)
#ifdef DEBUG_ALL
SOFT_LINK_DEBUG_LIBRARY(CFNetwork)
#else
SOFT_LINK_LIBRARY(CFNetwork)
#endif
#else
SOFT_LINK_FRAMEWORK_IN_CORESERVICES_UMBRELLA(CFNetwork)
#endif

SOFT_LINK_OPTIONAL(CFNetwork, CFHTTPCookieCopyDomain, CFStringRef, __cdecl, (CFHTTPCookieRef))
SOFT_LINK_OPTIONAL(CFNetwork, CFHTTPCookieGetExpirationTime, CFAbsoluteTime, __cdecl, (CFHTTPCookieRef))
SOFT_LINK_OPTIONAL(CFNetwork, CFHTTPCookieCopyName, CFStringRef, __cdecl, (CFHTTPCookieRef))
SOFT_LINK_OPTIONAL(CFNetwork, CFHTTPCookieCopyPath, CFStringRef, __cdecl, (CFHTTPCookieRef))
SOFT_LINK_OPTIONAL(CFNetwork, CFHTTPCookieCopyValue, CFStringRef, __cdecl, (CFHTTPCookieRef))

static inline RetainPtr<CFStringRef> cookieDomain(CFHTTPCookieRef cookie)
{
    if (CFHTTPCookieCopyDomainPtr())
        return RetainPtr<CFStringRef>(AdoptCF, CFHTTPCookieCopyDomainPtr()(cookie));
    return CFHTTPCookieGetDomain(cookie);
}

static inline CFAbsoluteTime cookieExpirationTime(CFHTTPCookieRef cookie)
{
    if (CFHTTPCookieGetExpirationTimePtr())
        return CFHTTPCookieGetExpirationTimePtr()(cookie);
    return CFDateGetAbsoluteTime(CFHTTPCookieGetExpiratonDate(cookie));
}

static inline RetainPtr<CFStringRef> cookieName(CFHTTPCookieRef cookie)
{
    if (CFHTTPCookieCopyNamePtr())
        return RetainPtr<CFStringRef>(AdoptCF, CFHTTPCookieCopyNamePtr()(cookie));
    return CFHTTPCookieGetName(cookie);
}

static inline RetainPtr<CFStringRef> cookiePath(CFHTTPCookieRef cookie)
{
    if (CFHTTPCookieCopyPathPtr())
        return RetainPtr<CFStringRef>(AdoptCF, CFHTTPCookieCopyPathPtr()(cookie));
    return CFHTTPCookieGetPath(cookie);
}

static inline RetainPtr<CFStringRef> cookieValue(CFHTTPCookieRef cookie)
{
    if (CFHTTPCookieCopyValuePtr())
        return RetainPtr<CFStringRef>(AdoptCF, CFHTTPCookieCopyValuePtr()(cookie));
    return CFHTTPCookieGetValue(cookie);
}

static RetainPtr<CFArrayRef> filterCookies(CFArrayRef unfilteredCookies)
{
    CFIndex count = CFArrayGetCount(unfilteredCookies);
    RetainPtr<CFMutableArrayRef> filteredCookies(AdoptCF, CFArrayCreateMutable(0, count, &kCFTypeArrayCallBacks));
    for (CFIndex i = 0; i < count; ++i) {
        CFHTTPCookieRef cookie = (CFHTTPCookieRef)CFArrayGetValueAtIndex(unfilteredCookies, i);

        // <rdar://problem/5632883> CFHTTPCookieStorage would store an empty cookie,
        // which would be sent as "Cookie: =". We have a workaround in setCookies() to prevent
        // that, but we also need to avoid sending cookies that were previously stored, and
        // there's no harm to doing this check because such a cookie is never valid.
        if (!CFStringGetLength(cookieName(cookie).get()))
            continue;

        if (CFHTTPCookieIsHTTPOnly(cookie))
            continue;

        CFArrayAppendValue(filteredCookies.get(), cookie);
    }
    return filteredCookies;
}

void setCookies(Document* document, const KURL& url, const String& value)
{
    // <rdar://problem/5632883> CFHTTPCookieStorage stores an empty cookie, which would be sent as "Cookie: =".
    if (value.isEmpty())
        return;

    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return;

    RetainPtr<CFURLRef> urlCF(AdoptCF, url.createCFURL());
    RetainPtr<CFURLRef> firstPartyForCookiesCF(AdoptCF, document->firstPartyForCookies().createCFURL());

    // <http://bugs.webkit.org/show_bug.cgi?id=6531>, <rdar://4409034>
    // cookiesWithResponseHeaderFields doesn't parse cookies without a value
    String cookieString = value.contains('=') ? value : value + "=";

    RetainPtr<CFStringRef> cookieStringCF(AdoptCF, cookieString.createCFString());
    RetainPtr<CFDictionaryRef> headerFieldsCF(AdoptCF, CFDictionaryCreate(kCFAllocatorDefault,
        (const void**)&s_setCookieKeyCF, (const void**)&cookieStringCF, 1,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieCreateWithResponseHeaderFields(kCFAllocatorDefault,
        headerFieldsCF.get(), urlCF.get()));

    CFHTTPCookieStorageSetCookies(cookieStorage, filterCookies(cookiesCF.get()).get(), urlCF.get(), firstPartyForCookiesCF.get());
}

String cookies(const Document* /*document*/, const KURL& url)
{
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return String();

    RetainPtr<CFURLRef> urlCF(AdoptCF, url.createCFURL());

    bool secure = url.protocolIs("https");
    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieStorageCopyCookiesForURL(cookieStorage, urlCF.get(), secure));
    RetainPtr<CFDictionaryRef> headerCF(AdoptCF, CFHTTPCookieCopyRequestHeaderFields(kCFAllocatorDefault, filterCookies(cookiesCF.get()).get()));
    return (CFStringRef)CFDictionaryGetValue(headerCF.get(), s_cookieCF);
}

String cookieRequestHeaderFieldValue(const Document* /*document*/, const KURL& url)
{
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return String();

    RetainPtr<CFURLRef> urlCF(AdoptCF, url.createCFURL());

    bool secure = url.protocolIs("https");
    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieStorageCopyCookiesForURL(cookieStorage, urlCF.get(), secure));
    RetainPtr<CFDictionaryRef> headerCF(AdoptCF, CFHTTPCookieCopyRequestHeaderFields(kCFAllocatorDefault, cookiesCF.get()));
    return (CFStringRef)CFDictionaryGetValue(headerCF.get(), s_cookieCF);
}

bool cookiesEnabled(const Document* /*document*/)
{
    CFHTTPCookieStorageAcceptPolicy policy = CFHTTPCookieStorageAcceptPolicyOnlyFromMainDocumentDomain;
    if (CFHTTPCookieStorageRef cookieStorage = currentCookieStorage())
        policy = CFHTTPCookieStorageGetCookieAcceptPolicy(cookieStorage);
    return policy == CFHTTPCookieStorageAcceptPolicyOnlyFromMainDocumentDomain || policy == CFHTTPCookieStorageAcceptPolicyAlways;
}

bool getRawCookies(const Document*, const KURL& url, Vector<Cookie>& rawCookies)
{
    rawCookies.clear();
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return false;

    RetainPtr<CFURLRef> urlCF(AdoptCF, url.createCFURL());

    bool sendSecureCookies = url.protocolIs("https");
    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieStorageCopyCookiesForURL(cookieStorage, urlCF.get(), sendSecureCookies));

    CFIndex count = CFArrayGetCount(cookiesCF.get());
    rawCookies.reserveCapacity(count);

    for (CFIndex i = 0; i < count; i++) {
       CFHTTPCookieRef cookie = (CFHTTPCookieRef)CFArrayGetValueAtIndex(cookiesCF.get(), i);
       String name = cookieName(cookie).get();
       String value = cookieValue(cookie).get();
       String domain = cookieDomain(cookie).get();
       String path = cookiePath(cookie).get();

       double expires = (cookieExpirationTime(cookie) + kCFAbsoluteTimeIntervalSince1970) * 1000;

       bool httpOnly = CFHTTPCookieIsHTTPOnly(cookie);
       bool secure = CFHTTPCookieIsSecure(cookie);
       bool session = false;    // FIXME: Need API for if a cookie is a session cookie.

       rawCookies.uncheckedAppend(Cookie(name, value, domain, path, expires, httpOnly, secure, session));
    }

    return true;
}

void deleteCookie(const Document*, const KURL& url, const String& name)
{
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return;

    RetainPtr<CFURLRef> urlCF(AdoptCF, url.createCFURL());

    bool sendSecureCookies = url.protocolIs("https");
    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieStorageCopyCookiesForURL(cookieStorage, urlCF.get(), sendSecureCookies));

    CFIndex count = CFArrayGetCount(cookiesCF.get());
    for (CFIndex i = 0; i < count; i++) {
        CFHTTPCookieRef cookie = (CFHTTPCookieRef)CFArrayGetValueAtIndex(cookiesCF.get(), i);
        if (String(cookieName(cookie).get()) == name) {
            CFHTTPCookieStorageDeleteCookie(cookieStorage, cookie);
            break;
        }
    }
}

void getHostnamesWithCookies(HashSet<String>& hostnames)
{
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return;

    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieStorageCopyCookies(cookieStorage));
    if (!cookiesCF)
        return;

    CFIndex count = CFArrayGetCount(cookiesCF.get());
    for (CFIndex i = 0; i < count; ++i) {
        CFHTTPCookieRef cookie = static_cast<CFHTTPCookieRef>(const_cast<void *>(CFArrayGetValueAtIndex(cookiesCF.get(), i)));
        RetainPtr<CFStringRef> domain = cookieDomain(cookie);
        hostnames.add(domain.get());
    }
}

void deleteCookiesForHostname(const String& hostname)
{
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return;

    RetainPtr<CFArrayRef> cookiesCF(AdoptCF, CFHTTPCookieStorageCopyCookies(cookieStorage));
    if (!cookiesCF)
        return;

    CFIndex count = CFArrayGetCount(cookiesCF.get());
    for (CFIndex i = count - 1; i >=0; i--) {
        CFHTTPCookieRef cookie = static_cast<CFHTTPCookieRef>(const_cast<void *>(CFArrayGetValueAtIndex(cookiesCF.get(), i)));
        RetainPtr<CFStringRef> domain = cookieDomain(cookie);
        if (String(domain.get()) == hostname)
            CFHTTPCookieStorageDeleteCookie(cookieStorage, cookie);
    }
}

void deleteAllCookies()
{
    CFHTTPCookieStorageRef cookieStorage = currentCookieStorage();
    if (!cookieStorage)
        return;

    CFHTTPCookieStorageDeleteAllCookies(cookieStorage);
}

} // namespace WebCore

#endif // USE(CFNETWORK)
