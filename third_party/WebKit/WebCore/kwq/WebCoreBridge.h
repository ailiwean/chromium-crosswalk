/*
 * Copyright (C) 2002 Apple Computer, Inc.  All rights reserved.
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

#import <Foundation/Foundation.h>

#ifdef __cplusplus

class KHTMLPart;
class KHTMLView;

namespace khtml {
    class RenderPart;
}

typedef khtml::RenderPart KHTMLRenderPart;

#else

@class KHTMLPart;
@class KHTMLView;
@class KHTMLRenderPart;

#endif

@class WebError;
@class WebResourceHandle;

@class WebCoreFrameBridge;

@protocol WebCoreDOMTreeCopier;
@protocol WebCoreRenderTreeCopier;
@protocol WebCoreResourceLoader;

// WebCoreBridge objects are used by WebCore to abstract away operations that need
// to be implemented by library clients, for example WebKit. The objects are also
// used in the opposite direction, for simple access to WebCore functions without dealing
// directly with the KHTML C++ classes.

// A WebCoreBridge creates and holds a reference to a KHTMLPart.

// The WebCoreBridge interface contains methods for use by the non-WebCore side of the bridge.

@interface WebCoreBridge : NSObject
{
    KHTMLPart *part;
    BOOL bridgeOwnsKHTMLView;
}

- (KHTMLPart *)part;

- (void)openURL:(NSURL *)URL;
- (void)addData:(NSData *)data withEncoding:(NSString *)encoding;
- (void)closeURL;
- (void)end;

- (void)installInFrame:(NSView *)view;
- (void)removeFromFrame;

- (void)setURL:(NSURL *)URL;

- (void)scrollToBaseAnchor;

- (NSString *)documentTextFromDOM;

- (void)createKHTMLViewWithNSView:(NSView *)view
    width:(int)width height:(int)height
    marginWidth:(int)mw marginHeight:(int)mh;

- (NSString *)selectedText;
- (void)selectAll;

- (BOOL)isFrameSet;

- (void)reapplyStyles;
- (void)forceLayout;
- (void)adjustFrames:(NSRect)rect;
- (void)drawRect:(NSRect)rect;

- (void)mouseDown:(NSEvent *)event;
- (void)mouseUp:(NSEvent *)event;
- (void)mouseMoved:(NSEvent *)event;
- (void)mouseDragged:(NSEvent *)event;

- (NSObject *)copyDOMTree:(id <WebCoreDOMTreeCopier>)copier;
- (NSObject *)copyRenderTree:(id <WebCoreRenderTreeCopier>)copier;

@end

// The WebCoreBridge protocol contains methods for use by the WebCore side of the bridge.

@protocol WebCoreBridge

- (WebCoreBridge *)parent;

- (WebCoreFrameBridge *)frame;
- (NSArray *)childFrames; // WebCoreFrameBridge objects
- (WebCoreFrameBridge *)descendantFrameNamed:(NSString *)name;

- (WebCoreFrameBridge *)mainFrame;
- (WebCoreFrameBridge *)frameNamed:(NSString *)name; // searches entire hierarchy starting with mainFrame

- (void)setTitle:(NSString *)title;
- (void)setStatusText:(NSString *)status;

- (BOOL)createChildFrameNamed:(NSString *)frameName
    withURL:(NSURL *)URL renderPart:(KHTMLRenderPart *)renderPart
    allowsScrolling:(BOOL)allowsScrolling marginWidth:(int)width marginHeight:(int)height;

- (WebCoreBridge *)openNewWindowWithURL:(NSURL *)URL;
- (BOOL)areToolbarsVisisble;
- (void)setToolbarsVisible:(BOOL)visible;
- (BOOL)isStatusBarVisisble;
- (void)setStatusBarVisible:(BOOL)visible;
- (BOOL)areScrollbarsVisible;
- (void)setScrollbarsVisible:(BOOL)visible;
- (NSWindow *)window;
- (void)setWindowFrame:(NSRect)frame;

- (WebResourceHandle *)startLoadingResource:(id <WebCoreResourceLoader>)loader withURL:(NSURL *)URL;

- (void)objectLoadedFromCache:(NSURL *)URL size:(unsigned)bytes;

- (BOOL)openedByScript;
- (void)setOpenedByScript:(BOOL)openedByScript;

- (void)unfocusWindow;

@end

// This interface definition allows those who hold a WebCoreBridge * to call all the methods
// in the WebCoreBridge protocol without requiring the base implementation to supply the methods.
// This idiom is appropriate because WebCoreBridge is an abstract class.

@interface WebCoreBridge (SubclassResponsibility) <WebCoreBridge>
@end

@protocol WebCoreDOMTreeCopier <NSObject>
- (NSObject *)nodeWithName:(NSString *)name value:(NSString *)value source:(NSString *)source children:(NSArray *)children;
@end

@protocol WebCoreRenderTreeCopier <NSObject>
- (NSObject *)nodeWithName:(NSString *)name rect:(NSRect)rect view:(NSView *)view children:(NSArray *)children;
@end
