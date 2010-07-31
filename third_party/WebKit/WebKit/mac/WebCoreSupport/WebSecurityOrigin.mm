/*
 * Copyright (C) 2007, 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "WebSecurityOriginInternal.h"

#import <WebCore/KURL.h>
#import <WebCore/SecurityOrigin.h>
#import <WebCore/DatabaseTracker.h>

using namespace WebCore;

@implementation WebSecurityOrigin
- (id)initWithURL:(NSURL *)url
{
    self = [super init];
    if (!self)
        return nil;

    RefPtr<SecurityOrigin> origin = SecurityOrigin::create(KURL([url absoluteURL]));
    origin->ref();
    _private = reinterpret_cast<WebSecurityOriginPrivate*>(origin.get());

    return self;
}

- (NSString*)protocol
{
    return reinterpret_cast<SecurityOrigin*>(_private)->protocol();
}

- (NSString*)host
{
    return reinterpret_cast<SecurityOrigin*>(_private)->host();
}

// Deprecated. Use host instead. This needs to stay here until we ship a new Safari.
- (NSString*)domain
{
    return [self host];
}

- (unsigned short)port
{
    return reinterpret_cast<SecurityOrigin*>(_private)->port();
}

// FIXME: https://bugs.webkit.org/show_bug.cgi?id=40627
// Proper steps should be taken to have subclass implementations of SecurityOrigin's
// origin, quota, and setQuota methods.

- (unsigned long long)usage
{
#if ENABLE(DATABASE)
    return DatabaseTracker::tracker().usageForOrigin(reinterpret_cast<SecurityOrigin*>(_private));
#else
    return 0;
#endif
}

- (unsigned long long)quota
{
#if ENABLE(DATABASE)
    return DatabaseTracker::tracker().quotaForOrigin(reinterpret_cast<SecurityOrigin*>(_private));
#else
    return 0;
#endif
}

// If the quota is set to a value lower than the current usage, that quota will
// "stick" but no data will be purged to meet the new quota. This will simply
// prevent new data from being added to databases in that origin
- (void)setQuota:(unsigned long long)quota
{
#if ENABLE(DATABASE)
    DatabaseTracker::tracker().setQuota(reinterpret_cast<SecurityOrigin*>(_private), quota);
#endif
}

- (BOOL)isEqual:(id)anObject
{
    if (![anObject isMemberOfClass:[WebSecurityOrigin class]]) {
        return NO;
    }
    
    return [self _core]->equal([anObject _core]);
}

- (void)dealloc
{
    if (_private)
        reinterpret_cast<SecurityOrigin*>(_private)->deref();
    [super dealloc];
}

- (void)finalize
{
    if (_private)
        reinterpret_cast<SecurityOrigin*>(_private)->deref();
    [super finalize];
}

@end

@implementation WebSecurityOrigin (WebInternal)

- (id)_initWithWebCoreSecurityOrigin:(SecurityOrigin*)origin
{
    ASSERT(origin);
    self = [super init];
    if (!self)
        return nil;

    origin->ref();
    _private = reinterpret_cast<WebSecurityOriginPrivate*>(origin);

    return self;
}

- (SecurityOrigin *)_core
{
    return reinterpret_cast<SecurityOrigin*>(_private);
}

@end
