//
//  WebCookieAdapter.m
//  WebKit
//
//  Created by Maciej Stachowiak on Thu Jun 27 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import "WebCookieAdapter.h"
#import <WebFoundation/WebCookieManager.h>
#import <WebFoundation/WebAssertions.h>
#import <WebFoundation/WebCookieConstants.h>


@implementation WebCookieAdapter

+ (void)createSharedAdapter
{
    if (![self sharedAdapter]) {
        [[[self alloc] init] release];
    }
    ASSERT([[self sharedAdapter] isKindOfClass:self]);
}

- (BOOL)cookiesEnabled
{
    BOOL result;

    WebCookieAcceptPolicy acceptPolicy = [[WebCookieManager sharedCookieManager] acceptPolicy];
    result = (acceptPolicy == WebCookieAcceptPolicyAlways || acceptPolicy == WebCookieAcceptPolicyOnlyFromMainDocumentDomain);
    
    return result;
}

- (NSString *)cookiesForURL:(NSURL *)URL
{
    return [[[WebCookieManager sharedCookieManager] cookieRequestHeadersForURL:URL] objectForKey:@"Cookie"];
}

- (void)setCookies:(NSString *)cookies forURL:(NSURL *)URL policyBaseURL:(NSURL *)policyBaseURL
{
    [[WebCookieManager sharedCookieManager] setCookiesFromResponseHeaders:[NSDictionary dictionaryWithObject:cookies forKey:@"Set-Cookie"] forURL:URL policyBaseURL:policyBaseURL];    
}

@end
