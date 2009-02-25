/*
 * Copyright (C) 2005, 2006, 2007 Apple, Inc.  All rights reserved.
 *           (C) 2007 Graham Dennis (graham.dennis@gmail.com)
 *           (C) 2007 Eric Seidel <eric@webkit.org>
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
 
#import "DumpRenderTreeWindow.h"

#import "DumpRenderTree.h"

// FIXME: This file is ObjC++ only because of this include. :(
#import "LayoutTestController.h"
#import <WebKit/WebViewPrivate.h>
#import <WebKit/WebTypesInternal.h>

CFMutableArrayRef openWindowsRef = 0;

static CFArrayCallBacks NonRetainingArrayCallbacks = {
    0,
    NULL,
    NULL,
    CFCopyDescription,
    CFEqual
};

@implementation DumpRenderTreeWindow

+ (NSArray *)openWindows
{
    return [[(NSArray *)openWindowsRef copy] autorelease];
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)styleMask backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
    if (!openWindowsRef)
        openWindowsRef = CFArrayCreateMutable(NULL, 0, &NonRetainingArrayCallbacks);

    CFArrayAppendValue(openWindowsRef, self);
            
    return [super initWithContentRect:contentRect styleMask:styleMask backing:bufferingType defer:deferCreation];
}

- (void)close
{
    CFRange arrayRange = CFRangeMake(0, CFArrayGetCount(openWindowsRef));
    CFIndex i = CFArrayGetFirstIndexOfValue(openWindowsRef, arrayRange, self);
    assert(i != -1);
    CFArrayRemoveValueAtIndex(openWindowsRef, i);
    
    [super close];
}

- (BOOL)isKeyWindow
{
    return gLayoutTestController ? gLayoutTestController->windowIsKey() : YES;
}

- (void)keyDown:(id)sender
{
    // Do nothing, avoiding the beep we'd otherwise get from NSResponder,
    // once we get to the end of the responder chain.
}

- (WebView *)webView
{
    NSView *firstView = nil;
    if ([[[self contentView] subviews] count] > 0) {
        firstView = [[[self contentView] subviews] objectAtIndex:0];
        if ([firstView isKindOfClass:[WebView class]])
            return static_cast<WebView *>(firstView);
    }
    return nil;
}

- (void)startObservingWebView
{
    [[self webView] addObserver:self forKeyPath:@"_isUsingAcceleratedCompositing" options:0 context:0];
}

- (void)stopObservingWebView
{
    [[self webView] removeObserver:self forKeyPath:@"_isUsingAcceleratedCompositing"];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"_isUsingAcceleratedCompositing"]) {
        // When using accelerated compositing, the window needs to be autodisplay for AppKit/CA to
        // start accelerated animations correctly.
        BOOL isAccelerated = [[self webView] _isUsingAcceleratedCompositing];
        [self setAutodisplay:isAccelerated];
        return;
    }
    
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

@end
