/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef WebTextCheckingResult_h
#define WebTextCheckingResult_h

#include "../../../../public/platform/WebCommon.h"
#include "../../../../public/platform/WebString.h"
#include "WebTextCheckingType.h"

namespace WebCore {
struct TextCheckingResult;
}

namespace WebKit {

// A checked entry of text checking.
struct WebTextCheckingResult {
    WebTextCheckingResult()
        : type(WebTextCheckingTypeSpelling)
        , location(0)
        , length(0)
        , hash(0)
    {
    }

    WebTextCheckingResult(WebTextCheckingType type, int location, int length, const WebString& replacement = WebString(), uint32_t hash = 0)
        : type(type)
        , location(location)
        , length(length)
        , replacement(replacement)
        , hash(hash)
    {
    }

#if WEBKIT_IMPLEMENTATION
    operator WebCore::TextCheckingResult() const;
#endif

    WebTextCheckingType type;
    int location;
    int length;
    WebString replacement;
    uint32_t hash;
};

} // namespace WebKit

#endif
