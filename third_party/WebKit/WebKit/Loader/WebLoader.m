/*
 * Copyright (C) 2005 Apple Computer, Inc.  All rights reserved.
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

#import <WebKit/WebLoader.h>

#import <Foundation/NSURLAuthenticationChallenge.h>
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSURLResponse.h>

#import <JavaScriptCore/Assertions.h>
#import <WebKit/WebDataProtocol.h>
#import <WebKit/WebFrameLoader.h>
#import <WebCore/WebCoreSystemInterface.h>

static unsigned inNSURLConnectionCallback;
static BOOL NSURLConnectionSupportsBufferedData;

@interface NSURLConnection (NSURLConnectionTigerPrivate)
- (NSData *)_bufferedData;
@end

@interface WebLoader (WebNSURLAuthenticationChallengeSender) <NSURLAuthenticationChallengeSender>
@end

@implementation WebLoader (WebNSURLAuthenticationChallengeSender) 

- (void)useCredential:(NSURLCredential *)credential forAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    if (challenge == nil || challenge != currentWebChallenge) {
        return;
    }

    [[currentConnectionChallenge sender] useCredential:credential forAuthenticationChallenge:currentConnectionChallenge];

    [currentConnectionChallenge release];
    currentConnectionChallenge = nil;
    
    [currentWebChallenge release];
    currentWebChallenge = nil;
}

- (void)continueWithoutCredentialForAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    if (challenge == nil || challenge != currentWebChallenge) {
        return;
    }

    [[currentConnectionChallenge sender] continueWithoutCredentialForAuthenticationChallenge:currentConnectionChallenge];

    [currentConnectionChallenge release];
    currentConnectionChallenge = nil;
    
    [currentWebChallenge release];
    currentWebChallenge = nil;
}

- (void)cancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    if (challenge == nil || challenge != currentWebChallenge) {
        return;
    }

    [self cancel];
}

@end

// This declaration is only needed to ease the transition to a new SPI.  It can be removed
// moving forward beyond Tiger 8A416.
@interface NSURLProtocol (WebFoundationSecret) 
+ (void)_removePropertyForKey:(NSString *)key inRequest:(NSMutableURLRequest *)request;
@end

@implementation WebLoader

+ (void)initialize
{
    NSURLConnectionSupportsBufferedData = [NSURLConnection instancesRespondToSelector:@selector(_bufferedData)];
}

- (void)releaseResources
{
    ASSERT(!reachedTerminalState);
    
    // It's possible that when we release the handle, it will be
    // deallocated and release the last reference to this object.
    // We need to retain to avoid accessing the object after it
    // has been deallocated and also to avoid reentering this method.
    
    [self retain];

    // We need to set reachedTerminalState to YES before we release
    // the resources to prevent a double dealloc of WebView <rdar://problem/4372628>

    reachedTerminalState = YES;

    [identifier release];
    identifier = nil;

    [connection release];
    connection = nil;

    [frameLoader release];
    frameLoader = nil;
    
    [resourceData release];
    resourceData = nil;

    [self release];
}

- (void)dealloc
{
    ASSERT(reachedTerminalState);
    [request release];
    [response release];
    [originalURL release];
    [super dealloc];
}

- (BOOL)loadWithRequest:(NSURLRequest *)r
{
    ASSERT(connection == nil);
    ASSERT(![frameLoader archiveLoadPendingForLoader:self]);
    
    NSURL *URL = [[r URL] retain];
    [originalURL release];
    originalURL = URL;
    
    NSURLRequest *clientRequest = [self willSendRequest:r redirectResponse:nil];
    if (clientRequest == nil) {
        NSError *cancelledError = [frameLoader cancelledErrorWithRequest:r];
        [self didFailWithError:cancelledError];
        return NO;
    }
    r = clientRequest;
    
    if ([frameLoader willUseArchiveForRequest:r originalURL:originalURL loader:self])
        return YES;
    
#ifndef NDEBUG
    isInitializingConnection = YES;
#endif
    connection = [[NSURLConnection alloc] initWithRequest:r delegate:self];
#ifndef NDEBUG
    isInitializingConnection = NO;
#endif
    if (defersCallbacks)
        wkSetNSURLConnectionDefersCallbacks(connection, YES);

    return YES;
}

- (void)setDefersCallbacks:(BOOL)defers
{
    defersCallbacks = defers;
    wkSetNSURLConnectionDefersCallbacks(connection, defers);
    // Deliver the resource after a delay because callers don't expect to receive callbacks while calling this method.
}

- (BOOL)defersCallbacks
{
    return defersCallbacks;
}

- (void)setFrameLoader:(WebFrameLoader *)fl
{
    ASSERT(fl);
    
    [fl retain];
    [frameLoader release];
    frameLoader = fl;

    [self setDefersCallbacks:[frameLoader _defersCallbacks]];
}

- (WebFrameLoader *)frameLoader
{
    return frameLoader;
}

- (void)addData:(NSData *)data allAtOnce:(BOOL)allAtOnce
{
    if (allAtOnce) {
        resourceData = [data copy];
        return;
    }
        
    if (NSURLConnectionSupportsBufferedData) {
        // Buffer data only if the connection has handed us the data because is has stopped buffering it.
        if (resourceData != nil)
            [resourceData appendData:data];
    } else {
        if (resourceData == nil)
            resourceData = [[NSMutableData alloc] init];
        [resourceData appendData:data];
    }
}

- (NSData *)resourceData
{
    if (resourceData)
        // Retain and autorelease resourceData since releaseResources (which releases resourceData) may be called 
        // before the caller of this method has an opporuntity to retain the returned data (4070729).
        return [[resourceData retain] autorelease];

    if (NSURLConnectionSupportsBufferedData)
        return [connection _bufferedData];

    return nil;
}

- (void)clearResourceData
{
    [resourceData setLength:0];
}

- (NSURLRequest *)willSendRequest:(NSURLRequest *)newRequest redirectResponse:(NSURLResponse *)redirectResponse
{
    ASSERT(!reachedTerminalState);
    NSMutableURLRequest *mutableRequest = [[newRequest mutableCopy] autorelease];
    NSMutableURLRequest *clientRequest;
    NSURLRequest *updatedRequest;
    BOOL haveDataSchemeRequest = NO;
    
    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];

    newRequest = mutableRequest;

    // If we have a special "applewebdata" scheme URL we send a fake request to the delegate.
    clientRequest = [mutableRequest _webDataRequestExternalRequest];
    if (!clientRequest)
        clientRequest = mutableRequest;
    else
        haveDataSchemeRequest = YES;
    
    if (identifier == nil)
        identifier = [frameLoader _identifierForInitialRequest:clientRequest];

    updatedRequest = [frameLoader _willSendRequest:clientRequest forResource:identifier redirectResponse:redirectResponse];

    if (!haveDataSchemeRequest)
        newRequest = updatedRequest;
    else {
        // If the delegate modified the request use that instead of
        // our applewebdata request, otherwise use the original
        // applewebdata request.
        if (![updatedRequest isEqual:clientRequest]) {
            newRequest = updatedRequest;
        
            // The respondsToSelector: check is only necessary for people building/running prior to Tier 8A416.
            if ([NSURLProtocol respondsToSelector:@selector(_removePropertyForKey:inRequest:)] &&
                [newRequest isKindOfClass:[NSMutableURLRequest class]]) {
                NSMutableURLRequest *mr = (NSMutableURLRequest *)newRequest;
                [NSURLProtocol _removePropertyForKey:[NSURLRequest _webDataRequestPropertyKey] inRequest:mr];
            }

        }
    }

    // Store a copy of the request.
    [request autorelease];

    request = [newRequest copy];

    [self release];
    return request;
}

- (void)didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    ASSERT(!reachedTerminalState);
    ASSERT(!currentConnectionChallenge);
    ASSERT(!currentWebChallenge);

    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];
    currentConnectionChallenge = [challenge retain];;
    currentWebChallenge = [[NSURLAuthenticationChallenge alloc] initWithAuthenticationChallenge:challenge sender:self];

    [frameLoader _didReceiveAuthenticationChallenge:currentWebChallenge forResource:identifier];

    [self release];
}

- (void)didCancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    ASSERT(!reachedTerminalState);
    ASSERT(currentConnectionChallenge);
    ASSERT(currentWebChallenge);
    ASSERT(currentConnectionChallenge = challenge);

    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];
    [frameLoader _didCancelAuthenticationChallenge:currentWebChallenge forResource:identifier];
    [self release];
}

- (void)didReceiveResponse:(NSURLResponse *)r
{
    ASSERT(!reachedTerminalState);

    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain]; 

    // If the URL is one of our whacky applewebdata URLs then
    // fake up a substitute URL to present to the delegate.
    if([WebDataProtocol _webIsDataProtocolURL:[r URL]]) {
        r = [[[NSURLResponse alloc] initWithURL:[request _webDataRequestExternalURL] MIMEType:[r MIMEType] expectedContentLength:[r expectedContentLength] textEncodingName:[r textEncodingName]] autorelease];
    }

    [r retain];
    [response release];
    response = r;

    [frameLoader _didReceiveResponse:r forResource:identifier];

    [self release];
}

- (void)didReceiveData:(NSData *)data lengthReceived:(long long)lengthReceived allAtOnce:(BOOL)allAtOnce
{
    // The following assertions are not quite valid here, since a subclass
    // might override didReceiveData: in a way that invalidates them. This
    // happens with the steps listed in 3266216
    // ASSERT(con == connection);
    // ASSERT(!reachedTerminalState);

    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];
    
    [self addData:data allAtOnce:allAtOnce];
    
    [frameLoader _didReceiveData:data contentLength:lengthReceived forResource:identifier];

    [self release];
}

- (void)willStopBufferingData:(NSData *)data
{
    ASSERT(resourceData == nil);
    resourceData = [data mutableCopy];
}

- (void)signalFinish
{
    signalledFinish = YES;
    [frameLoader _didFinishLoadingForResource:identifier];
}

- (void)didFinishLoading
{
    // If load has been cancelled after finishing (which could happen with a 
    // javascript that changes the window location), do nothing.
    if (cancelledFlag)
        return;
    
    ASSERT(!reachedTerminalState);

    if (!signalledFinish)
        [self signalFinish];

    [self releaseResources];
}

- (void)didFailWithError:(NSError *)error
{
    if (cancelledFlag) {
        return;
    }
    
    ASSERT(!reachedTerminalState);

    // retain/release self in this delegate method since the additional processing can do
    // anything including possibly releasing self; one example of this is 3266216
    [self retain];

    [frameLoader _didFailLoadingWithError:error forResource:identifier];

    [self releaseResources];
    [self release];
}

- (NSCachedURLResponse *)willCacheResponse:(NSCachedURLResponse *)cachedResponse
{
    // When in private browsing mode, prevent caching to disk
    if ([cachedResponse storagePolicy] == NSURLCacheStorageAllowed && [frameLoader _privateBrowsingEnabled]) {
        cachedResponse = [[[NSCachedURLResponse alloc] initWithResponse:[cachedResponse response]
                                                                   data:[cachedResponse data]
                                                               userInfo:[cachedResponse userInfo]
                                                          storagePolicy:NSURLCacheStorageAllowedInMemoryOnly] autorelease];
    }
    return cachedResponse;
}

- (NSURLRequest *)connection:(NSURLConnection *)con willSendRequest:(NSURLRequest *)newRequest redirectResponse:(NSURLResponse *)redirectResponse
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    NSURLRequest *result = [self willSendRequest:newRequest redirectResponse:redirectResponse];
    --inNSURLConnectionCallback;
    return result;
}

- (void)connection:(NSURLConnection *)con didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    [self didReceiveAuthenticationChallenge:challenge];
    --inNSURLConnectionCallback;
}

- (void)connection:(NSURLConnection *)con didCancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    [self didCancelAuthenticationChallenge:challenge];
    --inNSURLConnectionCallback;
}

- (void)connection:(NSURLConnection *)con didReceiveResponse:(NSURLResponse *)r
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    [self didReceiveResponse:r];
    --inNSURLConnectionCallback;
}

- (void)connection:(NSURLConnection *)con didReceiveData:(NSData *)data lengthReceived:(long long)lengthReceived
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    [self didReceiveData:data lengthReceived:lengthReceived allAtOnce:NO];
    --inNSURLConnectionCallback;
}

- (void)connection:(NSURLConnection *)con willStopBufferingData:(NSData *)data
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    [self willStopBufferingData:data];
    --inNSURLConnectionCallback;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)con
{
    // don't worry about checking connection consistency if this load
    // got cancelled while finishing.
    ASSERT(cancelledFlag || con == connection);
    ++inNSURLConnectionCallback;
    [self didFinishLoading];
    --inNSURLConnectionCallback;
}

- (void)connection:(NSURLConnection *)con didFailWithError:(NSError *)error
{
    ASSERT(con == connection);
    ++inNSURLConnectionCallback;
    [self didFailWithError:error];
    --inNSURLConnectionCallback;
}

- (NSCachedURLResponse *)connection:(NSURLConnection *)con willCacheResponse:(NSCachedURLResponse *)cachedResponse
{
#ifndef NDEBUG
    if (connection == nil && isInitializingConnection) {
        LOG_ERROR("connection:willCacheResponse: was called inside of [NSURLConnection initWithRequest:delegate:] (40676250)");
    }
#endif
    ++inNSURLConnectionCallback;
    NSCachedURLResponse *result = [self willCacheResponse:cachedResponse];
    --inNSURLConnectionCallback;
    return result;
}

- (void)cancelWithError:(NSError *)error
{
    ASSERT(!reachedTerminalState);

    // This flag prevents bad behvior when loads that finish cause the
    // load itself to be cancelled (which could happen with a javascript that 
    // changes the window location). This is used to prevent both the body
    // of this method and the body of connectionDidFinishLoading: running
    // for a single delegate. Cancelling wins.
    cancelledFlag = YES;
    
    [currentConnectionChallenge release];
    currentConnectionChallenge = nil;
    
    [currentWebChallenge release];
    currentWebChallenge = nil;

    [frameLoader cancelPendingArchiveLoadForLoader:self];
    [connection cancel];

    [frameLoader _didFailLoadingWithError:error forResource:identifier];

    [self releaseResources];
}

- (void)cancel
{
    if (!reachedTerminalState) {
        [self cancelWithError:[self cancelledError]];
    }
}

- (void)setIdentifier: ident
{
    if (identifier != ident){
        [identifier release];
        identifier = [ident retain];
    }
}

- (NSURLResponse *)response
{
    return response;
}

+ (BOOL)inConnectionCallback
{
    return inNSURLConnectionCallback != 0;
}

- (NSError *)cancelledError
{
    return [frameLoader cancelledErrorWithRequest:request];
}

@end
