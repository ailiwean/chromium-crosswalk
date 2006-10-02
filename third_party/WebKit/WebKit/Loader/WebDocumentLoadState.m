/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#import "WebDocumentLoadState.h"

#import <JavaScriptCore/Assertions.h>
#import "WebFrameLoader.h"
#import "WebDataProtocol.h"
#import "WebFrameBridge.h"

#import <WebKitSystemInterface.h>

@implementation WebDocumentLoadState

- (id)initWithRequest:(NSURLRequest *)req
{
    self = [super init];
    if (!self)
        return nil;
    
    originalRequest = [req retain];
    originalRequestCopy = [originalRequest copy];
    request = [originalRequest mutableCopy];

    return self;
}

- (void)dealloc
{
    [mainResourceData release];
    [originalRequest release];
    [originalRequestCopy release];
    [request release];
    [response release];
    [mainDocumentError release];
    
    [super dealloc];
}    


- (void)setFrameLoader:(WebFrameLoader *)fl
{
    ASSERT(!fl || !frameLoader);
    
    frameLoader = fl;
}

- (WebFrameLoader *)frameLoader
{
    return frameLoader;
}

- (void)setMainResourceData:(NSData *)data
{
    [data retain];
    [mainResourceData release];
    mainResourceData = data;
}

- (NSData *)mainResourceData
{
    return mainResourceData != nil ? mainResourceData : [frameLoader mainResourceData];
}

- (NSURLRequest *)originalRequest
{
    return originalRequest;
}

- (NSURLRequest *)originalRequestCopy
{
    return originalRequestCopy;
}

- (NSMutableURLRequest *)request
{
    NSMutableURLRequest *clientRequest = [request _webDataRequestExternalRequest];
    if (!clientRequest)
        clientRequest = request;
    return clientRequest;
}

- (void)replaceRequestURLForAnchorScrollWithURL:(NSURL *)URL
{
    // assert that URLs differ only by fragment ID
    
    NSMutableURLRequest *newOriginalRequest = [originalRequestCopy mutableCopy];
    [originalRequestCopy release];
    [newOriginalRequest setURL:URL];
    originalRequestCopy = newOriginalRequest;
    
    NSMutableURLRequest *newRequest = [request mutableCopy];
    [request release];
    [newRequest setURL:URL];
    request = newRequest;
}

- (void)setRequest:(NSURLRequest *)req
{
    ASSERT_ARG(req, req != request);
    
    // Replacing an unreachable URL with alternate content looks like a server-side
    // redirect at this point, but we can replace a committed dataSource.
    BOOL handlingUnreachableURL = [req _webDataRequestUnreachableURL] != nil;
    if (handlingUnreachableURL)
        committed = NO;
    
    // We should never be getting a redirect callback after the data
    // source is committed, except in the unreachable URL case. It 
    // would be a WebFoundation bug if it sent a redirect callback after commit.
    ASSERT(!committed);
    
    NSURLRequest *oldRequest = request;
    
    request = [req mutableCopy];
    
    // Only send webView:didReceiveServerRedirectForProvisionalLoadForFrame: if URL changed.
    // Also, don't send it when replacing unreachable URLs with alternate content.
    if (!handlingUnreachableURL && ![[oldRequest URL] isEqual:[req URL]])
        [frameLoader didReceiveServerRedirectForProvisionalLoadForFrame];
    
    [oldRequest release];
}

- (void)setResponse:(NSURLResponse *)resp
{
    [resp retain];
    [response release];
    response = resp;
}

- (BOOL)isStopping
{
    return stopping;
}

- (WebFrameBridge *)bridge
{
    return [frameLoader bridge];
}

- (void)setMainDocumentError:(NSError *)error
{
    [error retain];
    [mainDocumentError release];
    mainDocumentError = error;
    
    [frameLoader documentLoadState:self setMainDocumentError:error];
 }

- (NSError *)mainDocumentError
{
    return mainDocumentError;
}

- (void)clearErrors
{
    [mainDocumentError release];
    mainDocumentError = nil;
}

- (void)mainReceivedError:(NSError *)error complete:(BOOL)isComplete
{
    if (!frameLoader)
        return;
    
    [self setMainDocumentError:error];
    
    if (isComplete) {
        [frameLoader documentLoadState:self mainReceivedCompleteError:error];
    }
}

// Cancels the data source's pending loads.  Conceptually, a data source only loads
// one document at a time, but one document may have many related resources. 
// stopLoading will stop all loads initiated by the data source, 
// but not loads initiated by child frames' data sources -- that's the WebFrame's job.
- (void)stopLoading
{
    // Always attempt to stop the bridge/part because it may still be loading/parsing after the data source
    // is done loading and not stopping it can cause a world leak.
    if (committed)
        [[self bridge] stopLoading];
    
    if (!loading)
        return;
    
    [self retain];
    
    stopping = YES;
    
    if ([frameLoader isLoadingMainResource]) {
        // Stop the main resource loader and let it send the cancelled message.
        [frameLoader cancelMainResourceLoad];
    } else if ([frameLoader isLoadingSubresources]) {
        // The main resource loader already finished loading. Set the cancelled error on the 
        // document and let the subresourceLoaders send individual cancelled messages below.
        [self setMainDocumentError:[frameLoader cancelledErrorWithRequest:request]];
    } else {
        // If there are no resource loaders, we need to manufacture a cancelled message.
        // (A back/forward navigation has no resource loaders because its resources are cached.)
        [self mainReceivedError:[frameLoader cancelledErrorWithRequest:request] complete:YES];
    }
    
    [frameLoader stopLoadingSubresources];
    [frameLoader stopLoadingPlugIns];
    
    stopping = NO;
    
    [self release];
}

- (void)setupForReplace
{
    [frameLoader setupForReplace];
    committed = NO;
}

- (void)commitIfReady
{
    if (gotFirstByte && !committed) {
        committed = YES;
        [frameLoader commitProvisitionalLoad];
    }
}

- (void)finishedLoading
{
    gotFirstByte = YES;   
    [self commitIfReady];
    [frameLoader finishedLoadingDocumentLoadState:self];
    [[self bridge] end];
}

- (void)setCommitted:(BOOL)f
{
    committed = f;
}

- (BOOL)isCommitted
{
    return committed;
}

- (void)setLoading:(BOOL)f
{
    loading = f;
}

- (BOOL)isLoading
{
    return loading;
}

- (void)commitLoadWithData:(NSData *)data
{
    // Both unloading the old page and parsing the new page may execute JavaScript which destroys the datasource
    // by starting a new load, so retain temporarily.
    [self retain];
    [self commitIfReady];
    
    [frameLoader committedLoadWithDocumentLoadState:self data:data];

    [self release];
}

- (BOOL)doesProgressiveLoadWithMIMEType:(NSString *)MIMEType
{
    return ![frameLoader isReplacing] || [MIMEType isEqualToString:@"text/html"];
}

- (void)receivedData:(NSData *)data
{    
    gotFirstByte = YES;
    
    if ([self doesProgressiveLoadWithMIMEType:[response MIMEType]])
        [self commitLoadWithData:data];
}

- (void)setupForReplaceByMIMEType:(NSString *)newMIMEType
{
    if (!gotFirstByte)
        return;
    
    NSString *oldMIMEType = [response MIMEType];
    
    if (![self doesProgressiveLoadWithMIMEType:oldMIMEType]) {
        [frameLoader revertToProvisionalWithDocumentLoadState:self];
        [self setupForReplace];
        [self commitLoadWithData:[self mainResourceData]];
    }
    
    [frameLoader finishedLoadingDocumentLoadState:self];
    [[self bridge] end];
    
    [frameLoader setReplacing];
    gotFirstByte = NO;
    
    if ([self doesProgressiveLoadWithMIMEType:newMIMEType]) {
        [frameLoader revertToProvisionalWithDocumentLoadState:self];
        [self setupForReplace];
    }
    
    [frameLoader stopLoadingSubresources];
    [frameLoader stopLoadingPlugIns];

    [frameLoader finalSetupForReplaceWithDocumentLoadState:self];
}

- (void)updateLoading
{
    ASSERT(self == [frameLoader activeDocumentLoadState]);
    
    [self setLoading:[frameLoader isLoading]];
}

- (NSURLResponse *)response
{
    return response;
}

@end
