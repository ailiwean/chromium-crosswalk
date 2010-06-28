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

#ifndef TestInvocation_h
#define TestInvocation_h

#include <WebKit2/WKRetainPtr.h>
#include <wtf/Noncopyable.h>

namespace WTR {

class PlatformWebView;

class TestInvocation : Noncopyable {
public:
    TestInvocation(const char*);
    ~TestInvocation();

    void invoke();

private:
    void initializeMainWebView();
    void dump(const char*);

    // Helper
    static void runUntil(bool& done);

    // WKContextInjectedBundleClient
    static void _didRecieveMessageFromInjectedBundle(WKContextRef context, WKStringRef message, const void*);
    void didRecieveMessageFromInjectedBundle(WKStringRef message);

    WKStringRef injectedBundlePath();

    WKRetainPtr<WKURLRef> m_url;
    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageNamespaceRef> m_pageNamespace;
    PlatformWebView* m_mainWebView;

    // Invocation state
    bool m_gotInitialResponse;
    bool m_gotFinalMessage;
};

} // namespace WTR

#endif // TestInvocation_h
