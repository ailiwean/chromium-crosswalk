/*
 * Copyright (C) 2005, 2006, 2007 Apple Inc. All rights reserved.
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

#if ENABLE(NETSCAPE_PLUGIN_API)

#import "WebBaseNetscapePluginView.h"

#import "WebDataSourceInternal.h"
#import "WebDefaultUIDelegate.h"
#import "WebFrameInternal.h" 
#import "WebFrameView.h"
#import "WebGraphicsExtras.h"
#import "WebKitLogging.h"
#import "WebKitNSStringExtras.h"
#import "WebKitSystemInterface.h"
#import "WebNSDataExtras.h"
#import "WebNSDictionaryExtras.h"
#import "WebNSObjectExtras.h"
#import "WebNSURLExtras.h"
#import "WebNSURLRequestExtras.h"
#import "WebNSViewExtras.h"
#import "WebNetscapePluginPackage.h"
#import "WebBaseNetscapePluginStream.h"
#import "WebNetscapePluginEventHandler.h"
#import "WebNullPluginView.h"
#import "WebPreferences.h"
#import "WebViewInternal.h"
#import "WebUIDelegatePrivate.h"
#import <Carbon/Carbon.h>
#import <kjs/JSLock.h>
#import <WebCore/npruntime_impl.h>
#import <WebCore/Document.h>
#import <WebCore/DocumentLoader.h>
#import <WebCore/Element.h>
#import <WebCore/Frame.h> 
#import <WebCore/FrameLoader.h> 
#import <WebCore/FrameTree.h> 
#import <WebCore/Page.h> 
#import <WebCore/PluginMainThreadScheduler.h>
#import <WebCore/ScriptController.h>
#import <WebCore/SoftLinking.h> 
#import <WebCore/WebCoreObjCExtras.h>
#import <WebKit/nptextinput.h>
#import <WebKit/DOMPrivate.h>
#import <WebKit/WebUIDelegate.h>
#import <wtf/Assertions.h>
#import <objc/objc-runtime.h>

using namespace WebCore;

#define LoginWindowDidSwitchFromUserNotification    @"WebLoginWindowDidSwitchFromUserNotification"
#define LoginWindowDidSwitchToUserNotification      @"WebLoginWindowDidSwitchToUserNotification"


@interface WebBaseNetscapePluginView (Internal)
- (void)_viewHasMoved;
- (NPError)_createPlugin;
- (void)_destroyPlugin;
- (NSBitmapImageRep *)_printedPluginBitmap;
- (void)_redeliverStream;
@end

static WebBaseNetscapePluginView *currentPluginView = nil;

typedef struct OpaquePortState* PortState;

static const double ThrottledTimerInterval = 0.25;

class PluginTimer : public TimerBase {
public:
    typedef void (*TimerFunc)(NPP npp, uint32 timerID);
    
    PluginTimer(NPP npp, uint32 timerID, uint32 interval, NPBool repeat, TimerFunc timerFunc)
        : m_npp(npp)
        , m_timerID(timerID)
        , m_interval(interval)
        , m_repeat(repeat)
        , m_timerFunc(timerFunc)
    {
    }
    
    void start(bool throttle)
    {
        ASSERT(!isActive());
        
        double timeInterval = throttle ? ThrottledTimerInterval : m_interval / 1000.0;
        if (m_repeat)
            startRepeating(timeInterval);
        else
            startOneShot(timeInterval);
    }

private:
    virtual void fired() 
    {
        m_timerFunc(m_npp, m_timerID);
        if (!m_repeat)
            delete this;
    }
    
    NPP m_npp;
    uint32 m_timerID;
    uint32 m_interval;
    NPBool m_repeat;
    TimerFunc m_timerFunc;
};

#ifndef NP_NO_QUICKDRAW

// QuickDraw is not available in 64-bit

typedef struct {
    GrafPtr oldPort;
    GDHandle oldDevice;
    Point oldOrigin;
    RgnHandle oldClipRegion;
    RgnHandle oldVisibleRegion;
    RgnHandle clipRegion;
    BOOL forUpdate;
} PortState_QD;

#endif /* NP_NO_QUICKDRAW */

typedef struct {
    CGContextRef context;
} PortState_CG;

@class NSTextInputContext;
@interface NSResponder (AppKitDetails)
- (NSTextInputContext *)inputContext;
@end

@interface WebPluginRequest : NSObject
{
    NSURLRequest *_request;
    NSString *_frameName;
    void *_notifyData;
    BOOL _didStartFromUserGesture;
    BOOL _sendNotification;
}

- (id)initWithRequest:(NSURLRequest *)request frameName:(NSString *)frameName notifyData:(void *)notifyData sendNotification:(BOOL)sendNotification didStartFromUserGesture:(BOOL)currentEventIsUserGesture;

- (NSURLRequest *)request;
- (NSString *)frameName;
- (void *)notifyData;
- (BOOL)isCurrentEventUserGesture;
- (BOOL)sendNotification;

@end

@interface NSData (WebPluginDataExtras)
- (BOOL)_web_startsWithBlankLine;
- (NSInteger)_web_locationAfterFirstBlankLine;
@end

@interface WebBaseNetscapePluginView (ForwardDeclarations)
- (void)setWindowIfNecessary;
- (NPError)loadRequest:(NSMutableURLRequest *)request inTarget:(const char *)cTarget withNotifyData:(void *)notifyData sendNotification:(BOOL)sendNotification;
@end

@implementation WebBaseNetscapePluginView

+ (void)initialize
{
#ifndef BUILDING_ON_TIGER
    WebCoreObjCFinalizeOnMainThread(self);
#endif
    WKSendUserChangeNotifications();
}

#pragma mark EVENTS

- (BOOL)superviewsHaveSuperviews
{
    NSView *contentView = [[self window] contentView];
    NSView *view;
    for (view = self; view != nil; view = [view superview]) { 
        if (view == contentView) {
            return YES;
        }
    }
    return NO;
}

#ifndef NP_NO_QUICKDRAW

// The WindowRef created by -[NSWindow windowRef] has a QuickDraw GrafPort that covers 
// the entire window frame (or structure region to use the Carbon term) rather then just the window content.
// We can remove this when <rdar://problem/4201099> is fixed.
- (void)fixWindowPort
{
    ASSERT(drawingModel == NPDrawingModelQuickDraw);
    
    NSWindow *currentWindow = [self currentWindow];
    if ([currentWindow isKindOfClass:objc_getClass("NSCarbonWindow")])
        return;
    
    float windowHeight = [currentWindow frame].size.height;
    NSView *contentView = [currentWindow contentView];
    NSRect contentRect = [contentView convertRect:[contentView frame] toView:nil]; // convert to window-relative coordinates
    
    CGrafPtr oldPort;
    GetPort(&oldPort);    
    SetPort(GetWindowPort((WindowRef)[currentWindow windowRef]));
    
    MovePortTo(static_cast<short>(contentRect.origin.x), /* Flip Y */ static_cast<short>(windowHeight - NSMaxY(contentRect)));
    PortSize(static_cast<short>(contentRect.size.width), static_cast<short>(contentRect.size.height));
    
    SetPort(oldPort);
}

static UInt32 getQDPixelFormatForBitmapContext(CGContextRef context)
{
    UInt32 byteOrder = CGBitmapContextGetBitmapInfo(context) & kCGBitmapByteOrderMask;
    if (byteOrder == kCGBitmapByteOrderDefault)
        switch (CGBitmapContextGetBitsPerPixel(context)) {
            case 16:
                byteOrder = kCGBitmapByteOrder16Host;
                break;
            case 32:
                byteOrder = kCGBitmapByteOrder32Host;
                break;
        }
    switch (byteOrder) {
        case kCGBitmapByteOrder16Little:
            return k16LE555PixelFormat;
        case kCGBitmapByteOrder32Little:
            return k32BGRAPixelFormat;
        case kCGBitmapByteOrder16Big:
            return k16BE555PixelFormat;
        case kCGBitmapByteOrder32Big:
            return k32ARGBPixelFormat;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

static inline void getNPRect(const CGRect& cgr, NPRect& npr)
{
    npr.top = static_cast<uint16>(cgr.origin.y);
    npr.left = static_cast<uint16>(cgr.origin.x);
    npr.bottom = static_cast<uint16>(CGRectGetMaxY(cgr));
    npr.right = static_cast<uint16>(CGRectGetMaxX(cgr));
}

#endif

static inline void getNPRect(const NSRect& nr, NPRect& npr)
{
    npr.top = static_cast<uint16>(nr.origin.y);
    npr.left = static_cast<uint16>(nr.origin.x);
    npr.bottom = static_cast<uint16>(NSMaxY(nr));
    npr.right = static_cast<uint16>(NSMaxX(nr));
}

- (NSRect)visibleRect
{
    // WebCore may impose an additional clip (via CSS overflow or clip properties).  Fetch
    // that clip now.    
    return NSIntersectionRect([self convertRect:[element _windowClipRect] fromView:nil], [super visibleRect]);
}

- (PortState)saveAndSetNewPortStateForUpdate:(BOOL)forUpdate
{
    ASSERT([self currentWindow] != nil);

#ifndef NP_NO_QUICKDRAW
    // If drawing with QuickDraw, fix the window port so that it has the same bounds as the NSWindow's
    // content view.  This makes it easier to convert between AppKit view and QuickDraw port coordinates.
    if (drawingModel == NPDrawingModelQuickDraw)
        [self fixWindowPort];
#endif

    // Use AppKit to convert view coordinates to NSWindow coordinates.
    NSRect boundsInWindow = [self convertRect:[self bounds] toView:nil];
    NSRect visibleRectInWindow = [self convertRect:[self visibleRect] toView:nil];
    
    // Flip Y to convert NSWindow coordinates to top-left-based window coordinates.
    float borderViewHeight = [[self currentWindow] frame].size.height;
    boundsInWindow.origin.y = borderViewHeight - NSMaxY(boundsInWindow);
    visibleRectInWindow.origin.y = borderViewHeight - NSMaxY(visibleRectInWindow);
    
#ifndef NP_NO_QUICKDRAW
    WindowRef windowRef = (WindowRef)[[self currentWindow] windowRef];
    ASSERT(windowRef);
        
    // Look at the Carbon port to convert top-left-based window coordinates into top-left-based content coordinates.
    if (drawingModel == NPDrawingModelQuickDraw) {
        ::Rect portBounds;
        CGrafPtr port = GetWindowPort(windowRef);
        GetPortBounds(port, &portBounds);

        PixMap *pix = *GetPortPixMap(port);
        boundsInWindow.origin.x += pix->bounds.left - portBounds.left;
        boundsInWindow.origin.y += pix->bounds.top - portBounds.top;
        visibleRectInWindow.origin.x += pix->bounds.left - portBounds.left;
        visibleRectInWindow.origin.y += pix->bounds.top - portBounds.top;
    }
#endif
    
    window.x = (int32)boundsInWindow.origin.x; 
    window.y = (int32)boundsInWindow.origin.y;
    window.width = static_cast<uint32>(NSWidth(boundsInWindow));
    window.height = static_cast<uint32>(NSHeight(boundsInWindow));
    
    // "Clip-out" the plug-in when:
    // 1) it's not really in a window or off-screen or has no height or width.
    // 2) window.x is a "big negative number" which is how WebCore expresses off-screen widgets.
    // 3) the window is miniaturized or the app is hidden
    // 4) we're inside of viewWillMoveToWindow: with a nil window. In this case, superviews may already have nil 
    // superviews and nil windows and results from convertRect:toView: are incorrect.
    NSWindow *realWindow = [self window];
    if (window.width <= 0 || window.height <= 0 || window.x < -100000
            || realWindow == nil || [realWindow isMiniaturized]
            || [NSApp isHidden]
            || ![self superviewsHaveSuperviews]
            || [self isHiddenOrHasHiddenAncestor]) {

        // The following code tries to give plug-ins the same size they will eventually have.
        // The specifiedWidth and specifiedHeight variables are used to predict the size that
        // WebCore will eventually resize us to.

        // The QuickTime plug-in has problems if you give it a width or height of 0.
        // Since other plug-ins also might have the same sort of trouble, we make sure
        // to always give plug-ins a size other than 0,0.

        if (window.width <= 0)
            window.width = specifiedWidth > 0 ? specifiedWidth : 100;
        if (window.height <= 0)
            window.height = specifiedHeight > 0 ? specifiedHeight : 100;

        window.clipRect.bottom = window.clipRect.top;
        window.clipRect.left = window.clipRect.right;
    } else {
        getNPRect(visibleRectInWindow, window.clipRect);
    }
    
    // Save the port state, set up the port for entry into the plugin
    PortState portState;
    switch (drawingModel) {
#ifndef NP_NO_QUICKDRAW
        case NPDrawingModelQuickDraw: {
            // Set up NS_Port.
            ::Rect portBounds;
            CGrafPtr port = GetWindowPort(windowRef);
            GetPortBounds(port, &portBounds);
            nPort.qdPort.port = port;
            nPort.qdPort.portx = (int32)-boundsInWindow.origin.x;
            nPort.qdPort.porty = (int32)-boundsInWindow.origin.y;
            window.window = &nPort;

            PortState_QD *qdPortState = (PortState_QD*)malloc(sizeof(PortState_QD));
            portState = (PortState)qdPortState;
            
            GetGWorld(&qdPortState->oldPort, &qdPortState->oldDevice);    

            qdPortState->oldOrigin.h = portBounds.left;
            qdPortState->oldOrigin.v = portBounds.top;

            qdPortState->oldClipRegion = NewRgn();
            GetPortClipRegion(port, qdPortState->oldClipRegion);
            
            qdPortState->oldVisibleRegion = NewRgn();
            GetPortVisibleRegion(port, qdPortState->oldVisibleRegion);
            
            RgnHandle clipRegion = NewRgn();
            qdPortState->clipRegion = clipRegion;

            CGContextRef currentContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
            if (currentContext && WKCGContextIsBitmapContext(currentContext)) {
                // We use WKCGContextIsBitmapContext here, because if we just called CGBitmapContextGetData
                // on any context, we'd log to the console every time. But even if WKCGContextIsBitmapContext
                // returns true, it still might not be a context we need to create a GWorld for; for example
                // transparency layers will return true, but return 0 for CGBitmapContextGetData.
                void* offscreenData = CGBitmapContextGetData(currentContext);
                if (offscreenData) {
                    // If the current context is an offscreen bitmap, then create a GWorld for it.
                    ::Rect offscreenBounds;
                    offscreenBounds.top = 0;
                    offscreenBounds.left = 0;
                    offscreenBounds.right = CGBitmapContextGetWidth(currentContext);
                    offscreenBounds.bottom = CGBitmapContextGetHeight(currentContext);
                    GWorldPtr newOffscreenGWorld;
                    QDErr err = NewGWorldFromPtr(&newOffscreenGWorld,
                        getQDPixelFormatForBitmapContext(currentContext), &offscreenBounds, 0, 0, 0,
                        static_cast<char*>(offscreenData), CGBitmapContextGetBytesPerRow(currentContext));
                    ASSERT(newOffscreenGWorld && !err);
                    if (!err) {
                        if (offscreenGWorld)
                            DisposeGWorld(offscreenGWorld);
                        offscreenGWorld = newOffscreenGWorld;

                        SetGWorld(offscreenGWorld, NULL);

                        port = offscreenGWorld;

                        nPort.qdPort.port = port;
                        boundsInWindow = [self bounds];
                        
                        // Generate a QD origin based on the current affine transform for currentContext.
                        CGAffineTransform offscreenMatrix = CGContextGetCTM(currentContext);
                        CGPoint origin = {0,0};
                        CGPoint axisFlip = {1,1};
                        origin = CGPointApplyAffineTransform(origin, offscreenMatrix);
                        axisFlip = CGPointApplyAffineTransform(axisFlip, offscreenMatrix);
                        
                        // Quartz bitmaps have origins at the bottom left, but the axes may be inverted, so handle that.
                        origin.x = offscreenBounds.left - origin.x * (axisFlip.x - origin.x);
                        origin.y = offscreenBounds.bottom + origin.y * (axisFlip.y - origin.y);
                        
                        nPort.qdPort.portx = static_cast<int32>(-boundsInWindow.origin.x + origin.x);
                        nPort.qdPort.porty = static_cast<int32>(-boundsInWindow.origin.y - origin.y);
                        window.x = 0;
                        window.y = 0;
                        window.window = &nPort;

                        // Use the clip bounds from the context instead of the bounds we created
                        // from the window above.
                        getNPRect(CGRectOffset(CGContextGetClipBoundingBox(currentContext), -origin.x, origin.y), window.clipRect);
                    }
                }
            }

            MacSetRectRgn(clipRegion,
                window.clipRect.left + nPort.qdPort.portx, window.clipRect.top + nPort.qdPort.porty,
                window.clipRect.right + nPort.qdPort.portx, window.clipRect.bottom + nPort.qdPort.porty);
            
            // Clip to the dirty region if drawing to a window. When drawing to another bitmap context, do not clip.
            if ([NSGraphicsContext currentContext] == [[self currentWindow] graphicsContext]) {
                // Clip to dirty region so plug-in does not draw over already-drawn regions of the window that are
                // not going to be redrawn this update.  This forces plug-ins to play nice with z-index ordering.
                if (forUpdate) {
                    RgnHandle viewClipRegion = NewRgn();
                    
                    // Get list of dirty rects from the opaque ancestor -- WebKit does some tricks with invalidation and
                    // display to enable z-ordering for NSViews; a side-effect of this is that only the WebHTMLView
                    // knows about the true set of dirty rects.
                    NSView *opaqueAncestor = [self opaqueAncestor];
                    const NSRect *dirtyRects;
                    NSInteger dirtyRectCount, dirtyRectIndex;
                    [opaqueAncestor getRectsBeingDrawn:&dirtyRects count:&dirtyRectCount];

                    for (dirtyRectIndex = 0; dirtyRectIndex < dirtyRectCount; dirtyRectIndex++) {
                        NSRect dirtyRect = [self convertRect:dirtyRects[dirtyRectIndex] fromView:opaqueAncestor];
                        if (!NSEqualSizes(dirtyRect.size, NSZeroSize)) {
                            // Create a region for this dirty rect
                            RgnHandle dirtyRectRegion = NewRgn();
                            SetRectRgn(dirtyRectRegion, static_cast<short>(NSMinX(dirtyRect)), static_cast<short>(NSMinY(dirtyRect)), static_cast<short>(NSMaxX(dirtyRect)), static_cast<short>(NSMaxY(dirtyRect)));
                            
                            // Union this dirty rect with the rest of the dirty rects
                            UnionRgn(viewClipRegion, dirtyRectRegion, viewClipRegion);
                            DisposeRgn(dirtyRectRegion);
                        }
                    }
                
                    // Intersect the dirty region with the clip region, so that we only draw over dirty parts
                    SectRgn(clipRegion, viewClipRegion, clipRegion);
                    DisposeRgn(viewClipRegion);
                }
            }

            // Switch to the port and set it up.
            SetPort(port);
            PenNormal();
            ForeColor(blackColor);
            BackColor(whiteColor);
            SetOrigin(nPort.qdPort.portx, nPort.qdPort.porty);
            SetPortClipRegion(nPort.qdPort.port, clipRegion);

            if (forUpdate) {
                // AppKit may have tried to help us by doing a BeginUpdate.
                // But the invalid region at that level didn't include AppKit's notion of what was not valid.
                // We reset the port's visible region to counteract what BeginUpdate did.
                SetPortVisibleRegion(nPort.qdPort.port, clipRegion);
                InvalWindowRgn(windowRef, clipRegion);
            }
            
            qdPortState->forUpdate = forUpdate;
            break;
        }
#endif /* NP_NO_QUICKDRAW */

        case NPDrawingModelCoreGraphics: {            
            ASSERT([NSView focusView] == self);

            CGContextRef context = static_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);

            PortState_CG *cgPortState = (PortState_CG *)malloc(sizeof(PortState_CG));
            portState = (PortState)cgPortState;
            cgPortState->context = context;
            
            // Update the plugin's window/context
#ifdef NP_NO_CARBON
            nPort.cgPort.window = (NPNSWindow *)[self currentWindow];
#else
            nPort.cgPort.window = eventHandler->platformWindow([self currentWindow]);
#endif /* NP_NO_CARBON */
            nPort.cgPort.context = context;
            window.window = &nPort.cgPort;

            // Save current graphics context's state; will be restored by -restorePortState:
            CGContextSaveGState(context);

            // Clip to the dirty region if drawing to a window. When drawing to another bitmap context, do not clip.
            if ([NSGraphicsContext currentContext] == [[self currentWindow] graphicsContext]) {
                // Get list of dirty rects from the opaque ancestor -- WebKit does some tricks with invalidation and
                // display to enable z-ordering for NSViews; a side-effect of this is that only the WebHTMLView
                // knows about the true set of dirty rects.
                NSView *opaqueAncestor = [self opaqueAncestor];
                const NSRect *dirtyRects;
                NSInteger count;
                [opaqueAncestor getRectsBeingDrawn:&dirtyRects count:&count];
                Vector<CGRect, 16> convertedDirtyRects;
                convertedDirtyRects.resize(count);
                for (int i = 0; i < count; ++i)
                    reinterpret_cast<NSRect&>(convertedDirtyRects[i]) = [self convertRect:dirtyRects[i] fromView:opaqueAncestor];
                CGContextClipToRects(context, convertedDirtyRects.data(), count);
            }

            break;
        }

        default:
            ASSERT_NOT_REACHED();
            portState = NULL;
            break;
    }
    
    return portState;
}

- (PortState)saveAndSetNewPortState
{
    return [self saveAndSetNewPortStateForUpdate:NO];
}

- (void)restorePortState:(PortState)portState
{
    ASSERT([self currentWindow]);
    ASSERT(portState);
    
    switch (drawingModel) {
#ifndef NP_NO_QUICKDRAW
        case NPDrawingModelQuickDraw: {
            PortState_QD *qdPortState = (PortState_QD *)portState;
            WindowRef windowRef = (WindowRef)[[self currentWindow] windowRef];
            CGrafPtr port = GetWindowPort(windowRef);

            SetPort(port);

            if (qdPortState->forUpdate)
                ValidWindowRgn(windowRef, qdPortState->clipRegion);

            SetOrigin(qdPortState->oldOrigin.h, qdPortState->oldOrigin.v);

            SetPortClipRegion(port, qdPortState->oldClipRegion);
            if (qdPortState->forUpdate)
                SetPortVisibleRegion(port, qdPortState->oldVisibleRegion);

            DisposeRgn(qdPortState->oldClipRegion);
            DisposeRgn(qdPortState->oldVisibleRegion);
            DisposeRgn(qdPortState->clipRegion);

            SetGWorld(qdPortState->oldPort, qdPortState->oldDevice);
            break;
        }
#endif /* NP_NO_QUICKDRAW */
        
        case NPDrawingModelCoreGraphics:
            ASSERT([NSView focusView] == self);
            ASSERT(((PortState_CG *)portState)->context == nPort.cgPort.context);
            CGContextRestoreGState(nPort.cgPort.context);
            break;
                
        default:
            ASSERT_NOT_REACHED();
            break;
    }
}

- (BOOL)sendEvent:(void*)event isDrawRect:(BOOL)eventIsDrawRect
{
    if (![self window])
        return NO;
    ASSERT(event);
       
    if (!isStarted)
        return NO;

    ASSERT(NPP_HandleEvent);
    
    // Make sure we don't call NPP_HandleEvent while we're inside NPP_SetWindow.
    // We probably don't want more general reentrancy protection; we are really
    // protecting only against this one case, which actually comes up when
    // you first install the SVG viewer plug-in.
    if (inSetWindow)
        return NO;

    Frame* frame = core([self webFrame]);
    if (!frame)
        return NO;
    Page* page = frame->page();
    if (!page)
        return NO;

    bool wasDeferring = page->defersLoading();
    if (!wasDeferring)
        page->setDefersLoading(true);

    // Can only send drawRect (updateEvt) to CoreGraphics plugins when actually drawing
    ASSERT((drawingModel != NPDrawingModelCoreGraphics) || !eventIsDrawRect || [NSView focusView] == self);
    
    PortState portState;
    if ((drawingModel != NPDrawingModelCoreGraphics) || eventIsDrawRect) {
        // In CoreGraphics mode, the port state only needs to be saved/set when redrawing the plug-in view.  The plug-in is not
        // allowed to draw at any other time.
        portState = [self saveAndSetNewPortStateForUpdate:eventIsDrawRect];
        
        // We may have changed the window, so inform the plug-in.
        [self setWindowIfNecessary];
    } else
        portState = NULL;
    
#if !defined(NDEBUG) && !defined(NP_NO_QUICKDRAW)
    // Draw green to help debug.
    // If we see any green we know something's wrong.
    // Note that PaintRect() only works for QuickDraw plugins; otherwise the current QD port is undefined.
    if (drawingModel == NPDrawingModelQuickDraw && !isTransparent && eventIsDrawRect) {
        ForeColor(greenColor);
        const ::Rect bigRect = { -10000, -10000, 10000, 10000 };
        PaintRect(&bigRect);
        ForeColor(blackColor);
    }
#endif
    
    // Temporarily retain self in case the plug-in view is released while sending an event. 
    [[self retain] autorelease];
    
    BOOL acceptedEvent;
    [self willCallPlugInFunction];
    {
        JSC::JSLock::DropAllLocks dropAllLocks(false);
        acceptedEvent = NPP_HandleEvent(plugin, event);
    }
    [self didCallPlugInFunction];
        
    if (portState) {
        if ([self currentWindow])
            [self restorePortState:portState];
        free(portState);
    }

    if (!wasDeferring)
        page->setDefersLoading(false);
            
    return acceptedEvent;
}

- (void)sendActivateEvent:(BOOL)activate
{
    if (!isStarted)
        return;

    eventHandler->windowFocusChanged(activate);
}

- (void)sendDrawRectEvent:(NSRect)rect
{
    ASSERT(eventHandler);
    
    eventHandler->drawRect(rect);
}

- (void)stopTimers
{
    if (eventHandler)
        eventHandler->stopTimers();
    
    shouldFireTimers = NO;
    
    if (!timers)
        return;

    HashMap<uint32, PluginTimer*>::const_iterator end = timers->end();
    for (HashMap<uint32, PluginTimer*>::const_iterator it = timers->begin(); it != end; ++it) {
        PluginTimer* timer = it->second;
        timer->stop();
    }    
}

- (void)restartTimers
{
    ASSERT([self window]);
    
    if (shouldFireTimers)
        [self stopTimers];
    
    if (!isStarted || [[self window] isMiniaturized])
        return;

    shouldFireTimers = YES;
    
    // If the plugin is completely obscured (scrolled out of view, for example), then we will
    // send null events at a reduced rate.
    eventHandler->startTimers(isCompletelyObscured);
    
    if (!timers)
        return;
    
    HashMap<uint32, PluginTimer*>::const_iterator end = timers->end();
    for (HashMap<uint32, PluginTimer*>::const_iterator it = timers->begin(); it != end; ++it) {
        PluginTimer* timer = it->second;
        ASSERT(!timer->isActive());
        timer->start(isCompletelyObscured);
    }    
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)setHasFocus:(BOOL)flag
{
    if (!isStarted)
        return;

    if (hasFocus == flag)
        return;
    
    hasFocus = flag;
    
    // We need to null check the event handler here because
    // the plug-in view can resign focus after it's been stopped
    // and the event handler has been deleted.
    if (eventHandler)
        eventHandler->focusChanged(hasFocus);    
}

- (BOOL)becomeFirstResponder
{
    [self setHasFocus:YES];
    return YES;
}

- (BOOL)resignFirstResponder
{
    [self setHasFocus:NO];    
    return YES;
}

// AppKit doesn't call mouseDown or mouseUp on right-click. Simulate control-click
// mouseDown and mouseUp so plug-ins get the right-click event as they do in Carbon (3125743).
- (void)rightMouseDown:(NSEvent *)theEvent
{
    [self mouseDown:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self mouseUp:theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->mouseDown(theEvent);
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->mouseUp(theEvent);
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->mouseEntered(theEvent);
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->mouseExited(theEvent);
    
    // Set cursor back to arrow cursor.  Because NSCursor doesn't know about changes that the plugin made, we could get confused about what we think the
    // current cursor is otherwise.  Therefore we have no choice but to unconditionally reset the cursor when the mouse exits the plugin.
    [[NSCursor arrowCursor] set];
}

// We can't name this method mouseMoved because we don't want to override 
// the NSView mouseMoved implementation.
- (void)handleMouseMoved:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->mouseMoved(theEvent);
}
    
- (void)mouseDragged:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->mouseDragged(theEvent);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    if (!isStarted) {
        [super scrollWheel:theEvent];
        return;
    }

    if (!eventHandler->scrollWheel(theEvent))
        [super scrollWheel:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->keyUp(theEvent);
}

- (void)keyDown:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->keyDown(theEvent);
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    if (!isStarted)
        return;

    eventHandler->flagsChanged(theEvent);
}

- (void)cut:(id)sender
{
    if (!isStarted)
        return;

    eventHandler->keyDown([NSApp currentEvent]);
}

- (void)copy:(id)sender
{
    if (!isStarted)
        return;

    eventHandler->keyDown([NSApp currentEvent]);
}

- (void)paste:(id)sender
{
    if (!isStarted)
        return;

    eventHandler->keyDown([NSApp currentEvent]);
}

- (void)selectAll:(id)sender
{
    if (!isStarted)
        return;

    eventHandler->keyDown([NSApp currentEvent]);
}

#pragma mark WEB_NETSCAPE_PLUGIN

- (BOOL)isNewWindowEqualToOldWindow
{
    if (window.x != lastSetWindow.x)
        return NO;
    if (window.y != lastSetWindow.y)
        return NO;
    if (window.width != lastSetWindow.width)
        return NO;
    if (window.height != lastSetWindow.height)
        return NO;
    if (window.clipRect.top != lastSetWindow.clipRect.top)
        return NO;
    if (window.clipRect.left != lastSetWindow.clipRect.left)
        return NO;
    if (window.clipRect.bottom  != lastSetWindow.clipRect.bottom)
        return NO;
    if (window.clipRect.right != lastSetWindow.clipRect.right)
        return NO;
    if (window.type != lastSetWindow.type)
        return NO;
    
    switch (drawingModel) {
#ifndef NP_NO_QUICKDRAW
        case NPDrawingModelQuickDraw:
            if (nPort.qdPort.portx != lastSetPort.qdPort.portx)
                return NO;
            if (nPort.qdPort.porty != lastSetPort.qdPort.porty)
                return NO;
            if (nPort.qdPort.port != lastSetPort.qdPort.port)
                return NO;
        break;
#endif /* NP_NO_QUICKDRAW */
            
        case NPDrawingModelCoreGraphics:
            if (nPort.cgPort.window != lastSetPort.cgPort.window)
                return NO;
            if (nPort.cgPort.context != lastSetPort.cgPort.context)
                return NO;
        break;
                    
        default:
            ASSERT_NOT_REACHED();
        break;
    }
    
    return YES;
}

- (void)updateAndSetWindow
{
    // A plug-in can only update if it's (1) already been started (2) isn't stopped
    // and (3) is able to draw on-screen. To meet condition (3) the plug-in must not
    // be hidden and be attached to a window. QuickDraw plug-ins are an important
    // excpetion to rule (3) because they manually must be told when to stop writing
    // bits to the window backing store, thus to do so requires a new call to
    // NPP_SetWindow() with an empty NPWindow struct.
    if (!isStarted)
        return;
#ifdef NP_NO_QUICKDRAW
    if (![self canDraw])
        return;
#else
    if (drawingModel != NPDrawingModelQuickDraw && ![self canDraw])
        return;
#endif // NP_NO_QUICKDRAW
    
    BOOL didLockFocus = [NSView focusView] != self && [self lockFocusIfCanDraw];
    PortState portState = [self saveAndSetNewPortState];
    if (portState) {
        [self setWindowIfNecessary];
        [self restorePortState:portState];
        free(portState);
    }   
    if (didLockFocus)
        [self unlockFocus];
}

- (void)setWindowIfNecessary
{
    if (!isStarted) {
        return;
    }
    
    if (![self isNewWindowEqualToOldWindow]) {        
        // Make sure we don't call NPP_HandleEvent while we're inside NPP_SetWindow.
        // We probably don't want more general reentrancy protection; we are really
        // protecting only against this one case, which actually comes up when
        // you first install the SVG viewer plug-in.
        NPError npErr;
        ASSERT(!inSetWindow);
        
        inSetWindow = YES;
        
        // A CoreGraphics plugin's window may only be set while the plugin is being updated
        ASSERT((drawingModel != NPDrawingModelCoreGraphics) || [NSView focusView] == self);
        
        [self willCallPlugInFunction];
        {
            JSC::JSLock::DropAllLocks dropAllLocks(false);
            npErr = NPP_SetWindow(plugin, &window);
        }
        [self didCallPlugInFunction];
        inSetWindow = NO;

#ifndef NDEBUG
        switch (drawingModel) {
#ifndef NP_NO_QUICKDRAW
            case NPDrawingModelQuickDraw:
                LOG(Plugins, "NPP_SetWindow (QuickDraw): %d, port=0x%08x, window.x:%d window.y:%d window.width:%d window.height:%d",
                npErr, (int)nPort.qdPort.port, (int)window.x, (int)window.y, (int)window.width, (int)window.height);
            break;
#endif /* NP_NO_QUICKDRAW */
            
            case NPDrawingModelCoreGraphics:
                LOG(Plugins, "NPP_SetWindow (CoreGraphics): %d, window=%p, context=%p, window.x:%d window.y:%d window.width:%d window.height:%d",
                npErr, nPort.cgPort.window, nPort.cgPort.context, (int)window.x, (int)window.y, (int)window.width, (int)window.height);
            break;
            
            default:
                ASSERT_NOT_REACHED();
            break;
        }
#endif /* !defined(NDEBUG) */
        
        lastSetWindow = window;
        lastSetPort = nPort;
    }
}

- (void)removeTrackingRect
{
    if (trackingTag) {
        [self removeTrackingRect:trackingTag];
        trackingTag = 0;

        // Do the following after setting trackingTag to 0 so we don't re-enter.

        // Balance the retain in resetTrackingRect. Use autorelease in case we hold 
        // the last reference to the window during tear-down, to avoid crashing AppKit. 
        [[self window] autorelease];
    }
}

- (void)resetTrackingRect
{
    [self removeTrackingRect];
    if (isStarted) {
        // Retain the window so that removeTrackingRect can work after the window is closed.
        [[self window] retain];
        trackingTag = [self addTrackingRect:[self bounds] owner:self userData:nil assumeInside:NO];
    }
}

+ (void)setCurrentPluginView:(WebBaseNetscapePluginView *)view
{
    currentPluginView = view;
}

+ (WebBaseNetscapePluginView *)currentPluginView
{
    return currentPluginView;
}

- (BOOL)canStart
{
    return YES;
}

- (void)didStart
{
    if (_loadManually) {
        [self _redeliverStream];
        return;
    }
    
    // If the OBJECT/EMBED tag has no SRC, the URL is passed to us as "".
    // Check for this and don't start a load in this case.
    if (sourceURL != nil && ![sourceURL _web_isEmpty]) {
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:sourceURL];
        [request _web_setHTTPReferrer:core([self webFrame])->loader()->outgoingReferrer()];
        [self loadRequest:request inTarget:nil withNotifyData:nil sendNotification:NO];
    } 
}

- (void)addWindowObservers
{
    ASSERT([self window]);

    NSWindow *theWindow = [self window];
    
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    [notificationCenter addObserver:self selector:@selector(windowWillClose:) 
                               name:NSWindowWillCloseNotification object:theWindow]; 
    [notificationCenter addObserver:self selector:@selector(windowBecameKey:)
                               name:NSWindowDidBecomeKeyNotification object:theWindow];
    [notificationCenter addObserver:self selector:@selector(windowResignedKey:)
                               name:NSWindowDidResignKeyNotification object:theWindow];
    [notificationCenter addObserver:self selector:@selector(windowDidMiniaturize:)
                               name:NSWindowDidMiniaturizeNotification object:theWindow];
    [notificationCenter addObserver:self selector:@selector(windowDidDeminiaturize:)
                               name:NSWindowDidDeminiaturizeNotification object:theWindow];
    
    [notificationCenter addObserver:self selector:@selector(loginWindowDidSwitchFromUser:)
                               name:LoginWindowDidSwitchFromUserNotification object:nil];
    [notificationCenter addObserver:self selector:@selector(loginWindowDidSwitchToUser:)
                               name:LoginWindowDidSwitchToUserNotification object:nil];
}

- (void)removeWindowObservers
{
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    [notificationCenter removeObserver:self name:NSWindowWillCloseNotification        object:nil]; 
    [notificationCenter removeObserver:self name:NSWindowDidBecomeKeyNotification     object:nil];
    [notificationCenter removeObserver:self name:NSWindowDidResignKeyNotification     object:nil];
    [notificationCenter removeObserver:self name:NSWindowDidMiniaturizeNotification   object:nil];
    [notificationCenter removeObserver:self name:NSWindowDidDeminiaturizeNotification object:nil];
    [notificationCenter removeObserver:self name:LoginWindowDidSwitchFromUserNotification   object:nil];
    [notificationCenter removeObserver:self name:LoginWindowDidSwitchToUserNotification     object:nil];
}

- (BOOL)start
{
    ASSERT([self currentWindow]);
    
    if (isStarted)
        return YES;

    if (![self canStart])
        return NO;
    
    ASSERT([self webView]);
    
    if (![[[self webView] preferences] arePlugInsEnabled])
        return NO;

    // Open the plug-in package so it remains loaded while our plugin uses it
    [pluginPackage open];
    
    // Initialize drawingModel to an invalid value so that we can detect when the plugin does not specify a drawingModel
    drawingModel = (NPDrawingModel)-1;
    
    // Initialize eventModel to an invalid value so that we can detect when the plugin does not specify an event model.
    eventModel = (NPEventModel)-1;
    
    // Plug-ins are "windowed" by default.  On MacOS, windowed plug-ins share the same window and graphics port as the main
    // browser window.  Windowless plug-ins are rendered off-screen, then copied into the main browser window.
    window.type = NPWindowTypeWindow;
    
    NPError npErr = [self _createPlugin];
    if (npErr != NPERR_NO_ERROR) {
        LOG_ERROR("NPP_New failed with error: %d", npErr);
        [self _destroyPlugin];
        [pluginPackage close];
        return NO;
    }
    
    if (drawingModel == (NPDrawingModel)-1) {
#ifndef NP_NO_QUICKDRAW
        // Default to QuickDraw if the plugin did not specify a drawing model.
        drawingModel = NPDrawingModelQuickDraw;
#else
        // QuickDraw is not available, so we can't default to it. Instead, default to CoreGraphics.
        drawingModel = NPDrawingModelCoreGraphics;
#endif
    }

    if (eventModel == (NPEventModel)-1) {
        // If the plug-in did not specify a drawing model we default to Carbon when it is available.
#ifndef NP_NO_CARBON
        eventModel = NPEventModelCarbon;
#else
        eventModel = NPEventModelCocoa;
#endif // NP_NO_CARBON
    }

#ifndef NP_NO_CARBON
    if (eventModel == NPEventModelCocoa &&
        drawingModel == NPDrawingModelQuickDraw) {
        LOG(Plugins, "Plugin can't use use Cocoa event model with QuickDraw drawing model: %@", pluginPackage);
        [self _destroyPlugin];
        [pluginPackage close];
        
        return NO;
    }        
#endif // NP_NO_CARBON
    
    // Create the event handler
    eventHandler = WebNetscapePluginEventHandler::create(self);
    
    // Get the text input vtable
    if (eventModel == NPEventModelCocoa) {
        [self willCallPlugInFunction];
        {
            JSC::JSLock::DropAllLocks dropAllLocks(false);
            NPPluginTextInputFuncs *value;
            if (NPP_GetValue(plugin, NPPVpluginTextInputFuncs, &value) == NPERR_NO_ERROR && value)
                textInputFuncs = value;
        }
        [self didCallPlugInFunction];
    }
    
    isStarted = YES;
    [[self webView] addPluginInstanceView:self];
        
    [self updateAndSetWindow];

    if ([self window]) {
        [self addWindowObservers];
        if ([[self window] isKeyWindow]) {
            [self sendActivateEvent:YES];
        }
        [self restartTimers];
    }

    [self resetTrackingRect];
    
    [self didStart];
    
    return YES;
}

- (void)stop
{
    // If we're already calling a plug-in function, do not call NPP_Destroy().  The plug-in function we are calling
    // may assume that its instance->pdata, or other memory freed by NPP_Destroy(), is valid and unchanged until said
    // plugin-function returns.
    // See <rdar://problem/4480737>.
    if (pluginFunctionCallDepth > 0) {
        shouldStopSoon = YES;
        return;
    }
    
    [self removeTrackingRect];

    if (!isStarted)
        return;
    
    isStarted = NO;
    
    [[self webView] removePluginInstanceView:self];

    // To stop active streams it's necessary to invoke stop() on a copy 
    // of streams. This is because calling WebNetscapePluginStream::stop() also has the side effect
    // of removing a stream from this hash set.
    Vector<RefPtr<WebNetscapePluginStream> > streamsCopy;
    copyToVector(streams, streamsCopy);
    for (size_t i = 0; i < streamsCopy.size(); i++)
        streamsCopy[i]->stop();
    
   
    // Stop the timers
    [self stopTimers];
    
    // Stop notifications and callbacks.
    [self removeWindowObservers];
    [[pendingFrameLoads allKeys] makeObjectsPerformSelector:@selector(_setInternalLoadDelegate:) withObject:nil];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    // Setting the window type to 0 ensures that NPP_SetWindow will be called if the plug-in is restarted.
    lastSetWindow.type = (NPWindowType)0;
    
    [self _destroyPlugin];
    [pluginPackage close];
    
    delete eventHandler;
    eventHandler = 0;
    
    textInputFuncs = 0;
}

- (BOOL)isStarted
{
    return isStarted;
}

- (NPEventModel)eventModel
{
    return eventModel;
}

- (WebDataSource *)dataSource
{
    WebFrame *webFrame = kit(core(element)->document()->frame());
    return [webFrame _dataSource];
}

- (WebFrame *)webFrame
{
    return [[self dataSource] webFrame];
}

- (WebView *)webView
{
    return [[self webFrame] webView];
}

- (NSWindow *)currentWindow
{
    return [self window] ? [self window] : [[self webView] hostWindow];
}

- (NPP)plugin
{
    return plugin;
}

- (WebNetscapePluginPackage *)pluginPackage
{
    return pluginPackage;
}

- (void)setPluginPackage:(WebNetscapePluginPackage *)thePluginPackage;
{
    [thePluginPackage retain];
    [pluginPackage release];
    pluginPackage = thePluginPackage;

    NPP_New =           [pluginPackage NPP_New];
    NPP_Destroy =       [pluginPackage NPP_Destroy];
    NPP_SetWindow =     [pluginPackage NPP_SetWindow];
    NPP_NewStream =     [pluginPackage NPP_NewStream];
    NPP_WriteReady =    [pluginPackage NPP_WriteReady];
    NPP_Write =         [pluginPackage NPP_Write];
    NPP_StreamAsFile =  [pluginPackage NPP_StreamAsFile];
    NPP_DestroyStream = [pluginPackage NPP_DestroyStream];
    NPP_HandleEvent =   [pluginPackage NPP_HandleEvent];
    NPP_URLNotify =     [pluginPackage NPP_URLNotify];
    NPP_GetValue =      [pluginPackage NPP_GetValue];
    NPP_SetValue =      [pluginPackage NPP_SetValue];
    NPP_Print =         [pluginPackage NPP_Print];
}

- (void)setMIMEType:(NSString *)theMIMEType
{
    NSString *type = [theMIMEType copy];
    [MIMEType release];
    MIMEType = type;
}

- (void)setBaseURL:(NSURL *)theBaseURL
{
    [theBaseURL retain];
    [baseURL release];
    baseURL = theBaseURL;
}

- (void)setAttributeKeys:(NSArray *)keys andValues:(NSArray *)values;
{
    ASSERT([keys count] == [values count]);
    
    // Convert the attributes to 2 C string arrays.
    // These arrays are passed to NPP_New, but the strings need to be
    // modifiable and live the entire life of the plugin.

    // The Java plug-in requires the first argument to be the base URL
    if ([MIMEType isEqualToString:@"application/x-java-applet"]) {
        cAttributes = (char **)malloc(([keys count] + 1) * sizeof(char *));
        cValues = (char **)malloc(([values count] + 1) * sizeof(char *));
        cAttributes[0] = strdup("DOCBASE");
        cValues[0] = strdup([baseURL _web_URLCString]);
        argsCount++;
    } else {
        cAttributes = (char **)malloc([keys count] * sizeof(char *));
        cValues = (char **)malloc([values count] * sizeof(char *));
    }

    BOOL isWMP = [[[pluginPackage bundle] bundleIdentifier] isEqualToString:@"com.microsoft.WMP.defaultplugin"];
    
    unsigned i;
    unsigned count = [keys count];
    for (i = 0; i < count; i++) {
        NSString *key = [keys objectAtIndex:i];
        NSString *value = [values objectAtIndex:i];
        if ([key _webkit_isCaseInsensitiveEqualToString:@"height"]) {
            specifiedHeight = [value intValue];
        } else if ([key _webkit_isCaseInsensitiveEqualToString:@"width"]) {
            specifiedWidth = [value intValue];
        }
        // Avoid Window Media Player crash when these attributes are present.
        if (isWMP && ([key _webkit_isCaseInsensitiveEqualToString:@"SAMIStyle"] || [key _webkit_isCaseInsensitiveEqualToString:@"SAMILang"])) {
            continue;
        }
        cAttributes[argsCount] = strdup([key UTF8String]);
        cValues[argsCount] = strdup([value UTF8String]);
        LOG(Plugins, "%@ = %@", key, value);
        argsCount++;
    }
}

- (void)setMode:(int)theMode
{
    mode = theMode;
}

#pragma mark NSVIEW

- (id)initWithFrame:(NSRect)frame
      pluginPackage:(WebNetscapePluginPackage *)thePluginPackage
                URL:(NSURL *)theURL
            baseURL:(NSURL *)theBaseURL
           MIMEType:(NSString *)MIME
      attributeKeys:(NSArray *)keys
    attributeValues:(NSArray *)values
       loadManually:(BOOL)loadManually
         DOMElement:(DOMElement *)anElement
{
    [super initWithFrame:frame];
 
    pendingFrameLoads = [[NSMutableDictionary alloc] init];    
    
    // load the plug-in if it is not already loaded
    if (![thePluginPackage load]) {
        [self release];
        return nil;
    }
    [self setPluginPackage:thePluginPackage];
    
    element = [anElement retain];
    sourceURL = [theURL retain];
    
    [self setMIMEType:MIME];
    [self setBaseURL:theBaseURL];
    [self setAttributeKeys:keys andValues:values];
    if (loadManually)
        [self setMode:NP_FULL];
    else
        [self setMode:NP_EMBED];
    
    _loadManually = loadManually;
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    ASSERT_NOT_REACHED();
    return nil;
}

- (void)fini
{
#ifndef NP_NO_QUICKDRAW
    if (offscreenGWorld)
        DisposeGWorld(offscreenGWorld);
#endif

    unsigned i;
    for (i = 0; i < argsCount; i++) {
        free(cAttributes[i]);
        free(cValues[i]);
    }
    free(cAttributes);
    free(cValues);
    
    ASSERT(!eventHandler);
    
    if (timers) {
        deleteAllValues(*timers);
        delete timers;
    }    
}

- (void)disconnectStream:(WebNetscapePluginStream*)stream
{
    streams.remove(stream);
}

- (void)dealloc
{
    ASSERT(!isStarted);

    [sourceURL release];
    [_error release];
    
    [pluginPackage release];
    [MIMEType release];
    [baseURL release];
    [pendingFrameLoads release];
    [element release];
    
    ASSERT(!plugin);

    [self fini];

    [super dealloc];
}

- (void)finalize
{
    ASSERT_MAIN_THREAD();
    ASSERT(!isStarted);

    [self fini];

    [super finalize];
}

- (void)drawRect:(NSRect)rect
{
    if (!isStarted)
        return;
    
    if ([NSGraphicsContext currentContextDrawingToScreen])
        [self sendDrawRectEvent:rect];
    else {
        NSBitmapImageRep *printedPluginBitmap = [self _printedPluginBitmap];
        if (printedPluginBitmap) {
            // Flip the bitmap before drawing because the QuickDraw port is flipped relative
            // to this view.
            CGContextRef cgContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
            CGContextSaveGState(cgContext);
            NSRect bounds = [self bounds];
            CGContextTranslateCTM(cgContext, 0.0f, NSHeight(bounds));
            CGContextScaleCTM(cgContext, 1.0f, -1.0f);
            [printedPluginBitmap drawInRect:bounds];
            CGContextRestoreGState(cgContext);
        }
    }
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)renewGState
{
    [super renewGState];
    
    // -renewGState is called whenever the view's geometry changes.  It's a little hacky to override this method, but
    // much safer than walking up the view hierarchy and observing frame/bounds changed notifications, since you don't
    // have to track subsequent changes to the view hierarchy and add/remove notification observers.
    // NSOpenGLView uses the exact same technique to reshape its OpenGL surface.
    [self _viewHasMoved];
}

#ifndef NP_NO_QUICKDRAW
-(void)tellQuickTimeToChill
{
    ASSERT(drawingModel == NPDrawingModelQuickDraw);
    
    // Make a call to the secret QuickDraw API that makes QuickTime calm down.
    WindowRef windowRef = (WindowRef)[[self window] windowRef];
    if (!windowRef) {
        return;
    }
    CGrafPtr port = GetWindowPort(windowRef);
    ::Rect bounds;
    GetPortBounds(port, &bounds);
    WKCallDrawingNotification(port, &bounds);
}
#endif /* NP_NO_QUICKDRAW */

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
#ifndef NP_NO_QUICKDRAW
    if (drawingModel == NPDrawingModelQuickDraw)
        [self tellQuickTimeToChill];
#endif

    // We must remove the tracking rect before we move to the new window.
    // Once we move to the new window, it will be too late.
    [self removeTrackingRect];
    [self removeWindowObservers];
    
    // Workaround for: <rdar://problem/3822871> resignFirstResponder is not sent to first responder view when it is removed from the window
    [self setHasFocus:NO];

    if (!newWindow) {
        if ([[self webView] hostWindow]) {
            // View will be moved out of the actual window but it still has a host window.
            [self stopTimers];
        } else {
            // View will have no associated windows.
            [self stop];

            // Stop observing WebPreferencesChangedNotification -- we only need to observe this when installed in the view hierarchy.
            // When not in the view hierarchy, -viewWillMoveToWindow: and -viewDidMoveToWindow will start/stop the plugin as needed.
            [[NSNotificationCenter defaultCenter] removeObserver:self name:WebPreferencesChangedNotification object:nil];
        }
    }
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview
{
    if (!newSuperview) {
        // Stop the plug-in when it is removed from its superview.  It is not sufficient to do this in -viewWillMoveToWindow:nil, because
        // the WebView might still has a hostWindow at that point, which prevents the plug-in from being destroyed.
        // There is no need to start the plug-in when moving into a superview.  -viewDidMoveToWindow takes care of that.
        [self stop];
        
        // Stop observing WebPreferencesChangedNotification -- we only need to observe this when installed in the view hierarchy.
        // When not in the view hierarchy, -viewWillMoveToWindow: and -viewDidMoveToWindow will start/stop the plugin as needed.
        [[NSNotificationCenter defaultCenter] removeObserver:self name:WebPreferencesChangedNotification object:nil];
    }
}

- (void)viewDidMoveToWindow
{
    [self resetTrackingRect];
    
    if ([self window]) {
        // While in the view hierarchy, observe WebPreferencesChangedNotification so that we can start/stop depending
        // on whether plugins are enabled.
        [[NSNotificationCenter defaultCenter] addObserver:self
                                              selector:@selector(preferencesHaveChanged:)
                                              name:WebPreferencesChangedNotification
                                              object:nil];

        // View moved to an actual window. Start it if not already started.
        [self start];
        [self restartTimers];
        [self addWindowObservers];
    } else if ([[self webView] hostWindow]) {
        // View moved out of an actual window, but still has a host window.
        // Call setWindow to explicitly "clip out" the plug-in from sight.
        // FIXME: It would be nice to do this where we call stopNullEvents in viewWillMoveToWindow.
        [self updateAndSetWindow];
    }
}

- (void)viewWillMoveToHostWindow:(NSWindow *)hostWindow
{
    if (!hostWindow && ![self window]) {
        // View will have no associated windows.
        [self stop];

        // Remove WebPreferencesChangedNotification observer -- we will observe once again when we move back into the window
        [[NSNotificationCenter defaultCenter] removeObserver:self name:WebPreferencesChangedNotification object:nil];
    }
}

- (void)viewDidMoveToHostWindow
{
    if ([[self webView] hostWindow]) {
        // View now has an associated window. Start it if not already started.
        [self start];
    }
}

#pragma mark NOTIFICATIONS

- (void)windowWillClose:(NSNotification *)notification 
{
    [self stop]; 
} 

- (void)windowBecameKey:(NSNotification *)notification
{
    [self sendActivateEvent:YES];
    [self setNeedsDisplay:YES];
    [self restartTimers];
#ifndef NP_NO_CARBON
    SetUserFocusWindow((WindowRef)[[self window] windowRef]);
#endif // NP_NO_CARBON
}

- (void)windowResignedKey:(NSNotification *)notification
{
    [self sendActivateEvent:NO];
    [self setNeedsDisplay:YES];
    [self restartTimers];
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    [self stopTimers];
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    [self stopTimers];
}

- (void)loginWindowDidSwitchFromUser:(NSNotification *)notification
{
    [self stopTimers];
}

-(void)loginWindowDidSwitchToUser:(NSNotification *)notification
{
    [self restartTimers];
}

- (void)preferencesHaveChanged:(NSNotification *)notification
{
    WebPreferences *preferences = [[self webView] preferences];
    BOOL arePlugInsEnabled = [preferences arePlugInsEnabled];
    
    if ([notification object] == preferences && isStarted != arePlugInsEnabled) {
        if (arePlugInsEnabled) {
            if ([self currentWindow]) {
                [self start];
            }
        } else {
            [self stop];
            [self setNeedsDisplay:YES];
        }
    }
}

- (NPObject *)createPluginScriptableObject
{
    if (!NPP_GetValue || ![self isStarted])
        return NULL;
        
    NPObject *value = NULL;
    NPError error;
    [self willCallPlugInFunction];
    {
        JSC::JSLock::DropAllLocks dropAllLocks(false);
        error = NPP_GetValue(plugin, NPPVpluginScriptableNPObject, &value);
    }
    [self didCallPlugInFunction];
    if (error != NPERR_NO_ERROR)
        return NULL;
    
    return value;
}

- (void)willCallPlugInFunction
{
    ASSERT(plugin);

    // Could try to prevent infinite recursion here, but it's probably not worth the effort.
    pluginFunctionCallDepth++;
}

- (void)didCallPlugInFunction
{
    ASSERT(pluginFunctionCallDepth > 0);
    pluginFunctionCallDepth--;
    
    // If -stop was called while we were calling into a plug-in function, and we're no longer
    // inside a plug-in function, stop now.
    if (pluginFunctionCallDepth == 0 && shouldStopSoon) {
        shouldStopSoon = NO;
        [self stop];
    }
}

-(void)pluginView:(NSView *)pluginView receivedResponse:(NSURLResponse *)response
{
    ASSERT(_loadManually);
    ASSERT(!_manualStream);

    _manualStream = WebNetscapePluginStream::create(core([self webFrame])->loader());
}

- (void)pluginView:(NSView *)pluginView receivedData:(NSData *)data
{
    ASSERT(_loadManually);
    ASSERT(_manualStream);
    
    _dataLengthReceived += [data length];
    
    if (![self isStarted])
        return;

    if (!_manualStream->plugin()) {

        _manualStream->setRequestURL([[[self dataSource] request] URL]);
        _manualStream->setPlugin([self plugin]);
        ASSERT(_manualStream->plugin());
        
        _manualStream->startStreamWithResponse([[self dataSource] response]);
    }

    if (_manualStream->plugin())
        _manualStream->didReceiveData(0, static_cast<const char *>([data bytes]), [data length]);
}

- (void)pluginView:(NSView *)pluginView receivedError:(NSError *)error
{
    ASSERT(_loadManually);
    
    [error retain];
    [_error release];
    _error = error;
    
    if (![self isStarted]) {
        return;
    }

    _manualStream->destroyStreamWithError(error);
}

- (void)pluginViewFinishedLoading:(NSView *)pluginView 
{
    ASSERT(_loadManually);
    ASSERT(_manualStream);
    
    if ([self isStarted])
        _manualStream->didFinishLoading(0);
}

#pragma mark NSTextInput implementation

- (NSTextInputContext *)inputContext
{
#ifndef NP_NO_CARBON
    if (![self isStarted] || eventModel == NPEventModelCarbon)
        return nil;
#endif

    return [super inputContext];
}

- (BOOL)hasMarkedText
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);
    
    if (textInputFuncs && textInputFuncs->hasMarkedText)
        return textInputFuncs->hasMarkedText(plugin);
    
    return NO;
}

- (void)insertText:(id)aString
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);
    
    if (textInputFuncs && textInputFuncs->insertText)
        textInputFuncs->insertText(plugin, aString);
}

- (NSRange)markedRange
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);

    if (textInputFuncs && textInputFuncs->markedRange)
        return textInputFuncs->markedRange(plugin);
    
    return NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);

    if (textInputFuncs && textInputFuncs->selectedRange)
        return textInputFuncs->selectedRange(plugin);

    return NSMakeRange(NSNotFound, 0);
}    

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);

    if (textInputFuncs && textInputFuncs->setMarkedText)
        textInputFuncs->setMarkedText(plugin, aString, selRange);
}

- (void)unmarkText
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);
    
    if (textInputFuncs && textInputFuncs->unmarkText)
        textInputFuncs->unmarkText(plugin);
}

- (NSArray *)validAttributesForMarkedText
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);
        
    if (textInputFuncs && textInputFuncs->validAttributesForMarkedText)
        return textInputFuncs->validAttributesForMarkedText(plugin);
    
    return [NSArray array];
}

- (NSAttributedString *)attributedSubstringFromRange:(NSRange)theRange
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);
    
    if (textInputFuncs && textInputFuncs->attributedSubstringFromRange)
        return textInputFuncs->attributedSubstringFromRange(plugin, theRange);

    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)thePoint
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);

    if (textInputFuncs && textInputFuncs->characterIndexForPoint) {
        // Convert the point to window coordinates
        NSPoint point = [[self window] convertScreenToBase:thePoint];
        
        // And view coordinates
        point = [self convertPoint:point fromView:nil];
        
        return textInputFuncs->characterIndexForPoint(plugin, point);
    }        

    return NSNotFound;
}

- (void)doCommandBySelector:(SEL)aSelector
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);

    if (textInputFuncs && textInputFuncs->doCommandBySelector)
        textInputFuncs->doCommandBySelector(plugin, aSelector);
}

- (NSRect)firstRectForCharacterRange:(NSRange)theRange
{
    ASSERT(eventModel == NPEventModelCocoa);
    ASSERT([self isStarted]);

    if (textInputFuncs && textInputFuncs->firstRectForCharacterRange) {
        NSRect rect = textInputFuncs->firstRectForCharacterRange(plugin, theRange);
        
        // Convert the rect to window coordinates
        rect = [self convertRect:rect toView:nil];
        
        // Convert the rect location to screen coordinates
        rect.origin = [[self window] convertBaseToScreen:rect.origin];
        
        return rect;
    }

    return NSZeroRect;
}

// test for 10.4 because of <rdar://problem/4243463>
#ifdef BUILDING_ON_TIGER
- (long)conversationIdentifier
{
    return (long)self;
}
#else
- (NSInteger)conversationIdentifier
{
    return (NSInteger)self;
}
#endif

@end

@implementation WebBaseNetscapePluginView (WebNPPCallbacks)

- (NSMutableURLRequest *)requestWithURLCString:(const char *)URLCString
{
    if (!URLCString)
        return nil;
    
    CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault, URLCString, kCFStringEncodingISOLatin1);
    ASSERT(string); // All strings should be representable in ISO Latin 1
    
    NSString *URLString = [(NSString *)string _web_stringByStrippingReturnCharacters];
    NSURL *URL = [NSURL _web_URLWithDataAsString:URLString relativeToURL:baseURL];
    CFRelease(string);
    if (!URL)
        return nil;

    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:URL];
    Frame* frame = core([self webFrame]);
    if (!frame)
        return nil;
    [request _web_setHTTPReferrer:frame->loader()->outgoingReferrer()];
    return request;
}

- (void)evaluateJavaScriptPluginRequest:(WebPluginRequest *)JSPluginRequest
{
    // FIXME: Is this isStarted check needed here? evaluateJavaScriptPluginRequest should not be called
    // if we are stopped since this method is called after a delay and we call 
    // cancelPreviousPerformRequestsWithTarget inside of stop.
    if (!isStarted) {
        return;
    }
    
    NSURL *URL = [[JSPluginRequest request] URL];
    NSString *JSString = [URL _webkit_scriptIfJavaScriptURL];
    ASSERT(JSString);
    
    NSString *result = [[self webFrame] _stringByEvaluatingJavaScriptFromString:JSString forceUserGesture:[JSPluginRequest isCurrentEventUserGesture]];
    
    // Don't continue if stringByEvaluatingJavaScriptFromString caused the plug-in to stop.
    if (!isStarted) {
        return;
    }
        
    if ([JSPluginRequest frameName] != nil) {
        // FIXME: If the result is a string, we probably want to put that string into the frame.
        if ([JSPluginRequest sendNotification]) {
            [self willCallPlugInFunction];
            {
                JSC::JSLock::DropAllLocks dropAllLocks(false);
                NPP_URLNotify(plugin, [URL _web_URLCString], NPRES_DONE, [JSPluginRequest notifyData]);
            }
            [self didCallPlugInFunction];
        }
    } else if ([result length] > 0) {
        // Don't call NPP_NewStream and other stream methods if there is no JS result to deliver. This is what Mozilla does.
        NSData *JSData = [result dataUsingEncoding:NSUTF8StringEncoding];
        
        RefPtr<WebNetscapePluginStream> stream = WebNetscapePluginStream::create([NSURLRequest requestWithURL:URL], plugin, [JSPluginRequest sendNotification], [JSPluginRequest notifyData]);
        
        RetainPtr<NSURLResponse> response(AdoptNS, [[NSURLResponse alloc] initWithURL:URL 
                                                                             MIMEType:@"text/plain" 
                                                                expectedContentLength:[JSData length]
                                                                     textEncodingName:nil]);
        
        stream->startStreamWithResponse(response.get());
        stream->didReceiveData(0, static_cast<const char*>([JSData bytes]), [JSData length]);
        stream->didFinishLoading(0);
    }
}

- (void)webFrame:(WebFrame *)webFrame didFinishLoadWithReason:(NPReason)reason
{
    ASSERT(isStarted);
    
    WebPluginRequest *pluginRequest = [pendingFrameLoads objectForKey:webFrame];
    ASSERT(pluginRequest != nil);
    ASSERT([pluginRequest sendNotification]);
        
    [self willCallPlugInFunction];
    {
        JSC::JSLock::DropAllLocks dropAllLocks(false);
        NPP_URLNotify(plugin, [[[pluginRequest request] URL] _web_URLCString], reason, [pluginRequest notifyData]);
    }
    [self didCallPlugInFunction];
    
    [pendingFrameLoads removeObjectForKey:webFrame];
    [webFrame _setInternalLoadDelegate:nil];
}

- (void)webFrame:(WebFrame *)webFrame didFinishLoadWithError:(NSError *)error
{
    NPReason reason = NPRES_DONE;
    if (error != nil)
        reason = WebNetscapePluginStream::reasonForError(error);
    [self webFrame:webFrame didFinishLoadWithReason:reason];
}

- (void)loadPluginRequest:(WebPluginRequest *)pluginRequest
{
    NSURLRequest *request = [pluginRequest request];
    NSString *frameName = [pluginRequest frameName];
    WebFrame *frame = nil;
    
    NSURL *URL = [request URL];
    NSString *JSString = [URL _webkit_scriptIfJavaScriptURL];
    
    ASSERT(frameName || JSString);
    
    if (frameName) {
        // FIXME - need to get rid of this window creation which
        // bypasses normal targeted link handling
        frame = kit(core([self webFrame])->loader()->findFrameForNavigation(frameName));
        if (frame == nil) {
            WebView *currentWebView = [self webView];
            NSDictionary *features = [[NSDictionary alloc] init];
            WebView *newWebView = [[currentWebView _UIDelegateForwarder] webView:currentWebView
                                                        createWebViewWithRequest:nil
                                                                  windowFeatures:features];
            [features release];

            if (!newWebView) {
                if ([pluginRequest sendNotification]) {
                    [self willCallPlugInFunction];
                    {
                        JSC::JSLock::DropAllLocks dropAllLocks(false);
                        NPP_URLNotify(plugin, [[[pluginRequest request] URL] _web_URLCString], NPERR_GENERIC_ERROR, [pluginRequest notifyData]);
                    }
                    [self didCallPlugInFunction];
                }
                return;
            }
            
            frame = [newWebView mainFrame];
            core(frame)->tree()->setName(frameName);
            [[newWebView _UIDelegateForwarder] webViewShow:newWebView];
        }
    }

    if (JSString) {
        ASSERT(frame == nil || [self webFrame] == frame);
        [self evaluateJavaScriptPluginRequest:pluginRequest];
    } else {
        [frame loadRequest:request];
        if ([pluginRequest sendNotification]) {
            // Check if another plug-in view or even this view is waiting for the frame to load.
            // If it is, tell it that the load was cancelled because it will be anyway.
            WebBaseNetscapePluginView *view = [frame _internalLoadDelegate];
            if (view != nil) {
                ASSERT([view isKindOfClass:[WebBaseNetscapePluginView class]]);
                [view webFrame:frame didFinishLoadWithReason:NPRES_USER_BREAK];
            }
            [pendingFrameLoads _webkit_setObject:pluginRequest forUncopiedKey:frame];
            [frame _setInternalLoadDelegate:self];
        }
    }
}

- (NPError)loadRequest:(NSMutableURLRequest *)request inTarget:(const char *)cTarget withNotifyData:(void *)notifyData sendNotification:(BOOL)sendNotification
{
    NSURL *URL = [request URL];

    if (!URL) 
        return NPERR_INVALID_URL;

    // Don't allow requests to be loaded when the document loader is stopping all loaders.
    if ([[self dataSource] _documentLoader]->isStopping())
        return NPERR_GENERIC_ERROR;
    
    NSString *target = nil;
    if (cTarget) {
        // Find the frame given the target string.
        target = [NSString stringWithCString:cTarget encoding:NSISOLatin1StringEncoding];
    }
    WebFrame *frame = [self webFrame];

    // don't let a plugin start any loads if it is no longer part of a document that is being 
    // displayed unless the loads are in the same frame as the plugin.
    if ([[self dataSource] _documentLoader] != core([self webFrame])->loader()->activeDocumentLoader() &&
        (!cTarget || [frame findFrameNamed:target] != frame)) {
        return NPERR_GENERIC_ERROR; 
    }
    
    NSString *JSString = [URL _webkit_scriptIfJavaScriptURL];
    if (JSString != nil) {
        if (![[[self webView] preferences] isJavaScriptEnabled]) {
            // Return NPERR_GENERIC_ERROR if JS is disabled. This is what Mozilla does.
            return NPERR_GENERIC_ERROR;
        } else if (cTarget == NULL && mode == NP_FULL) {
            // Don't allow a JavaScript request from a standalone plug-in that is self-targetted
            // because this can cause the user to be redirected to a blank page (3424039).
            return NPERR_INVALID_PARAM;
        }
    } else {
        if (!FrameLoader::canLoad(URL, String(), core([self webFrame])->document()))
            return NPERR_GENERIC_ERROR;
    }
        
    if (cTarget || JSString) {
        // Make when targetting a frame or evaluating a JS string, perform the request after a delay because we don't
        // want to potentially kill the plug-in inside of its URL request.
        
        if (JSString && target && [frame findFrameNamed:target] != frame) {
            // For security reasons, only allow JS requests to be made on the frame that contains the plug-in.
            return NPERR_INVALID_PARAM;
        }
        
        bool currentEventIsUserGesture = false;
        if (eventHandler)
            currentEventIsUserGesture = eventHandler->currentEventIsUserGesture();
        
        WebPluginRequest *pluginRequest = [[WebPluginRequest alloc] initWithRequest:request 
                                                                          frameName:target
                                                                         notifyData:notifyData 
                                                                   sendNotification:sendNotification
                                                            didStartFromUserGesture:currentEventIsUserGesture];
        [self performSelector:@selector(loadPluginRequest:) withObject:pluginRequest afterDelay:0];
        [pluginRequest release];
    } else {
        RefPtr<WebNetscapePluginStream> stream = WebNetscapePluginStream::create(request, plugin, sendNotification, notifyData);

        streams.add(stream.get());
        stream->start();
    }
    
    return NPERR_NO_ERROR;
}

-(NPError)getURLNotify:(const char *)URLCString target:(const char *)cTarget notifyData:(void *)notifyData
{
    LOG(Plugins, "NPN_GetURLNotify: %s target: %s", URLCString, cTarget);

    NSMutableURLRequest *request = [self requestWithURLCString:URLCString];
    return [self loadRequest:request inTarget:cTarget withNotifyData:notifyData sendNotification:YES];
}

-(NPError)getURL:(const char *)URLCString target:(const char *)cTarget
{
    LOG(Plugins, "NPN_GetURL: %s target: %s", URLCString, cTarget);

    NSMutableURLRequest *request = [self requestWithURLCString:URLCString];
    return [self loadRequest:request inTarget:cTarget withNotifyData:NULL sendNotification:NO];
}

- (NPError)_postURL:(const char *)URLCString
             target:(const char *)target
                len:(UInt32)len
                buf:(const char *)buf
               file:(NPBool)file
         notifyData:(void *)notifyData
   sendNotification:(BOOL)sendNotification
       allowHeaders:(BOOL)allowHeaders
{
    if (!URLCString || !len || !buf) {
        return NPERR_INVALID_PARAM;
    }
    
    NSData *postData = nil;

    if (file) {
        // If we're posting a file, buf is either a file URL or a path to the file.
        NSString *bufString = (NSString *)CFStringCreateWithCString(kCFAllocatorDefault, buf, kCFStringEncodingWindowsLatin1);
        if (!bufString) {
            return NPERR_INVALID_PARAM;
        }
        NSURL *fileURL = [NSURL _web_URLWithDataAsString:bufString];
        NSString *path;
        if ([fileURL isFileURL]) {
            path = [fileURL path];
        } else {
            path = bufString;
        }
        postData = [NSData dataWithContentsOfFile:[path _webkit_fixedCarbonPOSIXPath]];
        CFRelease(bufString);
        if (!postData) {
            return NPERR_FILE_NOT_FOUND;
        }
    } else {
        postData = [NSData dataWithBytes:buf length:len];
    }

    if ([postData length] == 0) {
        return NPERR_INVALID_PARAM;
    }

    NSMutableURLRequest *request = [self requestWithURLCString:URLCString];
    [request setHTTPMethod:@"POST"];
    
    if (allowHeaders) {
        if ([postData _web_startsWithBlankLine]) {
            postData = [postData subdataWithRange:NSMakeRange(1, [postData length] - 1)];
        } else {
            NSInteger location = [postData _web_locationAfterFirstBlankLine];
            if (location != NSNotFound) {
                // If the blank line is somewhere in the middle of postData, everything before is the header.
                NSData *headerData = [postData subdataWithRange:NSMakeRange(0, location)];
                NSMutableDictionary *header = [headerData _webkit_parseRFC822HeaderFields];
                unsigned dataLength = [postData length] - location;

                // Sometimes plugins like to set Content-Length themselves when they post,
                // but WebFoundation does not like that. So we will remove the header
                // and instead truncate the data to the requested length.
                NSString *contentLength = [header objectForKey:@"Content-Length"];

                if (contentLength != nil)
                    dataLength = MIN((unsigned)[contentLength intValue], dataLength);
                [header removeObjectForKey:@"Content-Length"];

                if ([header count] > 0) {
                    [request setAllHTTPHeaderFields:header];
                }
                // Everything after the blank line is the actual content of the POST.
                postData = [postData subdataWithRange:NSMakeRange(location, dataLength)];

            }
        }
        if ([postData length] == 0) {
            return NPERR_INVALID_PARAM;
        }
    }

    // Plug-ins expect to receive uncached data when doing a POST (3347134).
    [request setCachePolicy:NSURLRequestReloadIgnoringCacheData];
    [request setHTTPBody:postData];
    
    return [self loadRequest:request inTarget:target withNotifyData:notifyData sendNotification:sendNotification];
}

- (NPError)postURLNotify:(const char *)URLCString
                  target:(const char *)target
                     len:(UInt32)len
                     buf:(const char *)buf
                    file:(NPBool)file
              notifyData:(void *)notifyData
{
    LOG(Plugins, "NPN_PostURLNotify: %s", URLCString);
    return [self _postURL:URLCString target:target len:len buf:buf file:file notifyData:notifyData sendNotification:YES allowHeaders:YES];
}

-(NPError)postURL:(const char *)URLCString
           target:(const char *)target
              len:(UInt32)len
              buf:(const char *)buf
             file:(NPBool)file
{
    LOG(Plugins, "NPN_PostURL: %s", URLCString);        
    // As documented, only allow headers to be specified via NPP_PostURL when using a file.
    return [self _postURL:URLCString target:target len:len buf:buf file:file notifyData:NULL sendNotification:NO allowHeaders:file];
}

-(NPError)newStream:(NPMIMEType)type target:(const char *)target stream:(NPStream**)stream
{
    LOG(Plugins, "NPN_NewStream");
    return NPERR_GENERIC_ERROR;
}

-(NPError)write:(NPStream*)stream len:(SInt32)len buffer:(void *)buffer
{
    LOG(Plugins, "NPN_Write");
    return NPERR_GENERIC_ERROR;
}

-(NPError)destroyStream:(NPStream*)stream reason:(NPReason)reason
{
    LOG(Plugins, "NPN_DestroyStream");
    // This function does a sanity check to ensure that the NPStream provided actually
    // belongs to the plug-in that provided it, which fixes a crash in the DivX 
    // plug-in: <rdar://problem/5093862> | http://bugs.webkit.org/show_bug.cgi?id=13203
    if (!stream || WebNetscapePluginStream::ownerForStream(stream) != plugin) {
        LOG(Plugins, "Invalid NPStream passed to NPN_DestroyStream: %p", stream);
        return NPERR_INVALID_INSTANCE_ERROR;
    }
    
    WebNetscapePluginStream* browserStream = static_cast<WebNetscapePluginStream*>(stream->ndata);
    browserStream->cancelLoadAndDestroyStreamWithError(browserStream->errorForReason(reason));
    
    return NPERR_NO_ERROR;
}

- (const char *)userAgent
{
    return [[[self webView] userAgentForURL:baseURL] UTF8String];
}

-(void)status:(const char *)message
{    
    if (!message) {
        LOG_ERROR("NPN_Status passed a NULL status message");
        return;
    }

    CFStringRef status = CFStringCreateWithCString(NULL, message, kCFStringEncodingUTF8);
    if (!status) {
        LOG_ERROR("NPN_Status: the message was not valid UTF-8");
        return;
    }
    
    LOG(Plugins, "NPN_Status: %@", status);
    WebView *wv = [self webView];
    [[wv _UIDelegateForwarder] webView:wv setStatusText:(NSString *)status];
    CFRelease(status);
}

-(void)invalidateRect:(NPRect *)invalidRect
{
    LOG(Plugins, "NPN_InvalidateRect");
    [self setNeedsDisplayInRect:NSMakeRect(invalidRect->left, invalidRect->top,
        (float)invalidRect->right - invalidRect->left, (float)invalidRect->bottom - invalidRect->top)];
}

-(BOOL)isOpaque
{
    return YES;
}

- (void)invalidateRegion:(NPRegion)invalidRegion
{
    LOG(Plugins, "NPN_InvalidateRegion");
    NSRect invalidRect = NSZeroRect;
    switch (drawingModel) {
#ifndef NP_NO_QUICKDRAW
        case NPDrawingModelQuickDraw:
        {
            ::Rect qdRect;
            GetRegionBounds((NPQDRegion)invalidRegion, &qdRect);
            invalidRect = NSMakeRect(qdRect.left, qdRect.top, qdRect.right - qdRect.left, qdRect.bottom - qdRect.top);
        }
        break;
#endif /* NP_NO_QUICKDRAW */
        
        case NPDrawingModelCoreGraphics:
        {
            CGRect cgRect = CGPathGetBoundingBox((NPCGRegion)invalidRegion);
            invalidRect = *(NSRect*)&cgRect;
            break;
        }
        default:
            ASSERT_NOT_REACHED();
        break;
    }
    
    [self setNeedsDisplayInRect:invalidRect];
}

-(void)forceRedraw
{
    LOG(Plugins, "forceRedraw");
    [self setNeedsDisplay:YES];
    [[self window] displayIfNeeded];
}

static NPBrowserTextInputFuncs *browserTextInputFuncs()
{
    static NPBrowserTextInputFuncs inputFuncs = {
        0,
        sizeof(NPBrowserTextInputFuncs),
        NPN_MarkedTextAbandoned,
        NPN_MarkedTextSelectionChanged
    };
    
    return &inputFuncs;
}

- (NPError)getVariable:(NPNVariable)variable value:(void *)value
{
    switch (variable) {
        case NPNVWindowNPObject:
        {
            Frame* frame = core([self webFrame]);
            NPObject* windowScriptObject = frame ? frame->script()->windowScriptNPObject() : 0;

            // Return value is expected to be retained, as described here: <http://www.mozilla.org/projects/plugins/npruntime.html#browseraccess>
            if (windowScriptObject)
                _NPN_RetainObject(windowScriptObject);
            
            void **v = (void **)value;
            *v = windowScriptObject;

            return NPERR_NO_ERROR;
        }

        case NPNVPluginElementNPObject:
        {
            if (!element)
                return NPERR_GENERIC_ERROR;
            
            NPObject *plugInScriptObject = (NPObject *)[element _NPObject];

            // Return value is expected to be retained, as described here: <http://www.mozilla.org/projects/plugins/npruntime.html#browseraccess>
            if (plugInScriptObject)
                _NPN_RetainObject(plugInScriptObject);

            void **v = (void **)value;
            *v = plugInScriptObject;

            return NPERR_NO_ERROR;
        }
        
        case NPNVpluginDrawingModel:
        {
            *(NPDrawingModel *)value = drawingModel;
            return NPERR_NO_ERROR;
        }

#ifndef NP_NO_QUICKDRAW
        case NPNVsupportsQuickDrawBool:
        {
            *(NPBool *)value = TRUE;
            return NPERR_NO_ERROR;
        }
#endif /* NP_NO_QUICKDRAW */
        
        case NPNVsupportsCoreGraphicsBool:
        {
            *(NPBool *)value = TRUE;
            return NPERR_NO_ERROR;
        }

        case NPNVsupportsOpenGLBool:
        {
            *(NPBool *)value = FALSE;
            return NPERR_NO_ERROR;
        }
        
#ifndef NP_NO_CARBON
        case NPNVsupportsCarbonBool:
        {
            *(NPBool *)value = TRUE;
            return NPERR_NO_ERROR;
        }
#endif /* NP_NO_CARBON */
            
        case NPNVsupportsCocoaBool:
        {
            *(NPBool *)value = TRUE;
            return NPERR_NO_ERROR;
        }
            
        case NPNVbrowserTextInputFuncs:
        {
            if (eventModel == NPEventModelCocoa) {
                *(NPBrowserTextInputFuncs **)value = browserTextInputFuncs();
                return NPERR_NO_ERROR;
            }
        }
        default:
            break;
    }

    return NPERR_GENERIC_ERROR;
}

- (NPError)setVariable:(NPPVariable)variable value:(void *)value
{
    switch (variable) {
        case NPPVpluginWindowBool:
        {
            NPWindowType newWindowType = (value ? NPWindowTypeWindow : NPWindowTypeDrawable);

            // Redisplay if window type is changing (some drawing models can only have their windows set while updating).
            if (newWindowType != window.type)
                [self setNeedsDisplay:YES];
            
            window.type = newWindowType;
        }
        
        case NPPVpluginTransparentBool:
        {
            BOOL newTransparent = (value != 0);
            
            // Redisplay if transparency is changing
            if (isTransparent != newTransparent)
                [self setNeedsDisplay:YES];
            
            isTransparent = newTransparent;
            
            return NPERR_NO_ERROR;
        }
        
        case NPPVpluginDrawingModel:
        {
            // Can only set drawing model inside NPP_New()
            if (self != [[self class] currentPluginView])
                return NPERR_GENERIC_ERROR;
            
            // Check for valid, supported drawing model
            NPDrawingModel newDrawingModel = (NPDrawingModel)(uintptr_t)value;
            switch (newDrawingModel) {
                // Supported drawing models:
#ifndef NP_NO_QUICKDRAW
                case NPDrawingModelQuickDraw:
#endif
                case NPDrawingModelCoreGraphics:
                    drawingModel = newDrawingModel;
                    return NPERR_NO_ERROR;

                // Unsupported (or unknown) drawing models:
                default:
                    LOG(Plugins, "Plugin %@ uses unsupported drawing model: %d", pluginPackage, drawingModel);
                    return NPERR_GENERIC_ERROR;
            }
        }
        
        case NPPVpluginEventModel:
        {
            // Can only set event model inside NPP_New()
            if (self != [[self class] currentPluginView])
                return NPERR_GENERIC_ERROR;
            
            // Check for valid, supported event model
            NPEventModel newEventModel = (NPEventModel)(uintptr_t)value;
            switch (newEventModel) {
                // Supported event models:
#ifndef NP_NO_CARBON
                case NPEventModelCarbon:
#endif
                case NPEventModelCocoa:
                    eventModel = newEventModel;
                    return NPERR_NO_ERROR;
                    
                    // Unsupported (or unknown) event models:
                default:
                    LOG(Plugins, "Plugin %@ uses unsupported event model: %d", pluginPackage, eventModel);
                    return NPERR_GENERIC_ERROR;
            }
        }
            
        default:
            return NPERR_GENERIC_ERROR;
    }
}

- (uint32)scheduleTimerWithInterval:(uint32)interval repeat:(NPBool)repeat timerFunc:(void (*)(NPP npp, uint32 timerID))timerFunc
{
    if (!timerFunc)
        return 0;
    
    if (!timers)
        timers = new HashMap<uint32, PluginTimer*>;
    
    uint32 timerID = ++currentTimerID;
    
    PluginTimer* timer = new PluginTimer(plugin, timerID, interval, repeat, timerFunc);
    timers->set(timerID, timer);

    if (shouldFireTimers)
        timer->start(isCompletelyObscured);
    
    return 0;
}

- (void)unscheduleTimer:(uint32)timerID
{
    if (!timers)
        return;
    
    if (PluginTimer* timer = timers->take(timerID))
        delete timer;
}

- (NPError)popUpContextMenu:(NPMenu *)menu
{
    NSEvent *currentEvent = [NSApp currentEvent];
    
    // NPN_PopUpContextMenu must be called from within the plug-in's NPP_HandleEvent.
    if (!currentEvent)
        return NPERR_GENERIC_ERROR;
    
    [NSMenu popUpContextMenu:(NSMenu *)menu withEvent:currentEvent forView:self];
    return NPERR_NO_ERROR;
}

@end

@implementation WebPluginRequest

- (id)initWithRequest:(NSURLRequest *)request frameName:(NSString *)frameName notifyData:(void *)notifyData sendNotification:(BOOL)sendNotification didStartFromUserGesture:(BOOL)currentEventIsUserGesture
{
    [super init];
    _didStartFromUserGesture = currentEventIsUserGesture;
    _request = [request retain];
    _frameName = [frameName retain];
    _notifyData = notifyData;
    _sendNotification = sendNotification;
    return self;
}

- (void)dealloc
{
    [_request release];
    [_frameName release];
    [super dealloc];
}

- (NSURLRequest *)request
{
    return _request;
}

- (NSString *)frameName
{
    return _frameName;
}

- (BOOL)isCurrentEventUserGesture
{
    return _didStartFromUserGesture;
}

- (BOOL)sendNotification
{
    return _sendNotification;
}

- (void *)notifyData
{
    return _notifyData;
}

@end

@implementation WebBaseNetscapePluginView (Internal)

- (NPError)_createPlugin
{
    plugin = (NPP)calloc(1, sizeof(NPP_t));
    plugin->ndata = self;

    ASSERT(NPP_New);

    // NPN_New(), which creates the plug-in instance, should never be called while calling a plug-in function for that instance.
    ASSERT(pluginFunctionCallDepth == 0);

    Frame* frame = core([self webFrame]);
    if (!frame)
        return NPERR_GENERIC_ERROR;
    Page* page = frame->page();
    if (!page)
        return NPERR_GENERIC_ERROR;
    
    bool wasDeferring = page->defersLoading();
    if (!wasDeferring)
        page->setDefersLoading(true);
    
    PluginMainThreadScheduler::scheduler().registerPlugin(plugin);
    
    [[self class] setCurrentPluginView:self];
    NPError npErr = NPP_New((char *)[MIMEType cString], plugin, mode, argsCount, cAttributes, cValues, NULL);
    [[self class] setCurrentPluginView:nil];
    
    if (!wasDeferring)
        page->setDefersLoading(false);

    LOG(Plugins, "NPP_New: %d", npErr);
    return npErr;
}

- (void)_destroyPlugin
{
    PluginMainThreadScheduler::scheduler().unregisterPlugin(plugin);
    
    NPError npErr;
    npErr = NPP_Destroy(plugin, NULL);
    LOG(Plugins, "NPP_Destroy: %d", npErr);
    
    if (Frame* frame = core([self webFrame]))
        frame->script()->cleanupScriptObjectsForPlugin(self);
        
    free(plugin);
    plugin = NULL;
}

- (void)_viewHasMoved
{
    // All of the work this method does may safely be skipped if the view is not in a window.  When the view
    // is moved back into a window, everything should be set up correctly.
    if (![self window])
        return;
    
#ifndef NP_NO_QUICKDRAW
    if (drawingModel == NPDrawingModelQuickDraw)
        [self tellQuickTimeToChill];
#endif
    [self updateAndSetWindow];
    [self resetTrackingRect];
    
    // Check to see if the plugin view is completely obscured (scrolled out of view, for example).
    // For performance reasons, we send null events at a lower rate to plugins which are obscured.
    BOOL oldIsObscured = isCompletelyObscured;
    isCompletelyObscured = NSIsEmptyRect([self visibleRect]);
    if (isCompletelyObscured != oldIsObscured)
        [self restartTimers];
}

- (NSBitmapImageRep *)_printedPluginBitmap
{
#ifdef NP_NO_QUICKDRAW
    return nil;
#else
    // Cannot print plugins that do not implement NPP_Print
    if (!NPP_Print)
        return nil;

    // This NSBitmapImageRep will share its bitmap buffer with a GWorld that the plugin will draw into.
    // The bitmap is created in 32-bits-per-pixel ARGB format, which is the default GWorld pixel format.
    NSBitmapImageRep *bitmap = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                         pixelsWide:window.width
                                                         pixelsHigh:window.height
                                                         bitsPerSample:8
                                                         samplesPerPixel:4
                                                         hasAlpha:YES
                                                         isPlanar:NO
                                                         colorSpaceName:NSDeviceRGBColorSpace
                                                         bitmapFormat:NSAlphaFirstBitmapFormat
                                                         bytesPerRow:0
                                                         bitsPerPixel:0] autorelease];
    ASSERT(bitmap);
    
    // Create a GWorld with the same underlying buffer into which the plugin can draw
    ::Rect printGWorldBounds;
    SetRect(&printGWorldBounds, 0, 0, window.width, window.height);
    GWorldPtr printGWorld;
    if (NewGWorldFromPtr(&printGWorld,
                         k32ARGBPixelFormat,
                         &printGWorldBounds,
                         NULL,
                         NULL,
                         0,
                         (Ptr)[bitmap bitmapData],
                         [bitmap bytesPerRow]) != noErr) {
        LOG_ERROR("Could not create GWorld for printing");
        return nil;
    }
    
    /// Create NPWindow for the GWorld
    NPWindow printNPWindow;
    printNPWindow.window = &printGWorld; // Normally this is an NP_Port, but when printing it is the actual CGrafPtr
    printNPWindow.x = 0;
    printNPWindow.y = 0;
    printNPWindow.width = window.width;
    printNPWindow.height = window.height;
    printNPWindow.clipRect.top = 0;
    printNPWindow.clipRect.left = 0;
    printNPWindow.clipRect.right = window.width;
    printNPWindow.clipRect.bottom = window.height;
    printNPWindow.type = NPWindowTypeDrawable; // Offscreen graphics port as opposed to a proper window
    
    // Create embed-mode NPPrint
    NPPrint npPrint;
    npPrint.mode = NP_EMBED;
    npPrint.print.embedPrint.window = printNPWindow;
    npPrint.print.embedPrint.platformPrint = printGWorld;
    
    // Tell the plugin to print into the GWorld
    [self willCallPlugInFunction];
    {
        JSC::JSLock::DropAllLocks dropAllLocks(false);
        NPP_Print(plugin, &npPrint);
    }
    [self didCallPlugInFunction];

    // Don't need the GWorld anymore
    DisposeGWorld(printGWorld);
        
    return bitmap;
#endif
}

- (void)_redeliverStream
{
    if ([self dataSource] && [self isStarted]) {
        // Deliver what has not been passed to the plug-in up to this point.
        if (_dataLengthReceived > 0) {
            NSData *data = [[[self dataSource] data] subdataWithRange:NSMakeRange(0, _dataLengthReceived)];
            _dataLengthReceived = 0;
            [self pluginView:self receivedData:data];
            if (![[self dataSource] isLoading]) {
                if (_error)
                    [self pluginView:self receivedError:_error];
                else
                    [self pluginViewFinishedLoading:self];
            }
        }
    }
}

@end

@implementation NSData (PluginExtras)

- (BOOL)_web_startsWithBlankLine
{
    return [self length] > 0 && ((const char *)[self bytes])[0] == '\n';
}


- (NSInteger)_web_locationAfterFirstBlankLine
{
    const char *bytes = (const char *)[self bytes];
    unsigned length = [self length];
    
    unsigned i;
    for (i = 0; i < length - 4; i++) {
        
        //  Support for Acrobat. It sends "\n\n".
        if (bytes[i] == '\n' && bytes[i+1] == '\n') {
            return i+2;
        }
        
        // Returns the position after 2 CRLF's or 1 CRLF if it is the first line.
        if (bytes[i] == '\r' && bytes[i+1] == '\n') {
            i += 2;
            if (i == 2) {
                return i;
            } else if (bytes[i] == '\n') {
                // Support for Director. It sends "\r\n\n" (3880387).
                return i+1;
            } else if (bytes[i] == '\r' && bytes[i+1] == '\n') {
                // Support for Flash. It sends "\r\n\r\n" (3758113).
                return i+2;
            }
        }
    }
    return NSNotFound;
}

@end
#endif
