/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#ifndef WebImageCache_h
#define WebImageCache_h

#include "../../../Platform/chromium/public/WebCommon.h"

namespace WebKit {

// An interface to query and configure WebKit's image cache.
//
// Methods of this interface can be called on any thread.
//
// Methods of this interface can be only be used after WebKit::initialize()
// and before WebKit::shutdown() is called.

class WebImageCache {
public:
    // Sets the capacities of the image cache, evicting objects as necessary.
    WEBKIT_EXPORT static void setCacheLimitInBytes(size_t);

    // Clears the cache (as much as possible; some resources may not be
    // cleared if they are actively referenced).
    WEBKIT_EXPORT static void clear();

    // Returns the number of bytes used by the cache.
    WEBKIT_EXPORT static size_t memoryUsageInBytes();

    // Returns the number of cached entries.
    WEBKIT_EXPORT static unsigned cacheEntries();

private:
    WebImageCache();  // Not intended to be instanced.
};

}  // namespace WebKit

#endif
