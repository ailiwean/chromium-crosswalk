/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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

#ifndef WebKitCSSKeyframesRule_h
#define WebKitCSSKeyframesRule_h

#include "CSSRule.h"
#include <wtf/RefPtr.h>
#include "AtomicString.h"

namespace WebCore {

class CSSRuleList;
class WebKitCSSKeyframeRule;
class String;

typedef int ExceptionCode;

class WebKitCSSKeyframesRule : public CSSRule {
public:
    static PassRefPtr<WebKitCSSKeyframesRule> create()
    {
        return adoptRef(new WebKitCSSKeyframesRule(0));
    }
    static PassRefPtr<WebKitCSSKeyframesRule> create(CSSStyleSheet* parent)
    {
        return adoptRef(new WebKitCSSKeyframesRule(parent));
    }

    ~WebKitCSSKeyframesRule();

    virtual bool isKeyframesRule() { return true; }

    // Inherited from CSSRule
    virtual unsigned short type() const { return WEBKIT_KEYFRAMES_RULE; }

    String name() const;
    void setName(const String&, ExceptionCode&);    
    void setName(String);    

    CSSRuleList* cssRules() { return m_lstCSSRules.get(); }

    void insertRule(const String& rule);
    void deleteRule(const String& key);
    WebKitCSSKeyframeRule* findRule(const String& key);

    virtual String cssText() const;

    /* not part of the DOM */
    unsigned length() const;
    WebKitCSSKeyframeRule*        item(unsigned index);
    const WebKitCSSKeyframeRule*  item(unsigned index) const;
    void insert(WebKitCSSKeyframeRule* rule);

private:
    WebKitCSSKeyframesRule(CSSStyleSheet* parent);

    int findRuleIndex(float key) const;
    
    RefPtr<CSSRuleList> m_lstCSSRules;
    String m_name;
};

} // namespace WebCore

#endif // WebKitCSSKeyframesRule_h
