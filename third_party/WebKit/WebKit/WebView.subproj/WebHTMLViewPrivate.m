/*	WebHTMLViewPrivate.mm
	Copyright 2002, Apple, Inc. All rights reserved.
        
        Private header file.  This file may reference classes (both ObjectiveC and C++)
        in WebCore.  Instances of this class are referenced by _private in 
        NSWebPageView.
*/

#import <WebKit/WebHTMLViewPrivate.h>

#import <AppKit/NSResponder_Private.h>

#import <WebFoundation/WebAssertions.h>

#import <WebKit/WebBridge.h>
#import <WebKit/WebContextMenuHandler.h>
#import <WebKit/WebController.h>
#import <WebKit/WebControllerPrivate.h>
#import <WebKit/WebDataSourcePrivate.h>
#import <WebKit/WebFramePrivate.h>
#import <WebKit/WebImageRenderer.h>
#import <WebKit/WebNSViewExtras.h>
#import <WebKit/WebPluginView.h>
#import <WebKit/WebViewPrivate.h>
#import <WebKit/WebWindowContext.h>

@interface NSView (AppKitSecretsIKnowAbout)
- (void)_recursiveDisplayRectIfNeededIgnoringOpacity:(NSRect)rect isVisibleRect:(BOOL)isVisibleRect rectIsVisibleRectForView:(NSView *)visibleView topView:(BOOL)topView;
- (void)_recursiveDisplayAllDirtyWithLockFocus:(BOOL)needsLockFocus visRect:(NSRect)visRect;
- (NSRect)_dirtyRect;
- (void)_drawRect:(NSRect)rect clip:(BOOL)clip;
@end

@interface NSView (WebNSViewDisplayExtras)
- (void)_web_stopIfPluginView;
- (void)_web_propagateDirtyRectToAncestor;
@end

@interface WebNSTextView : NSTextView
{
}
@end

@implementation WebHTMLViewPrivate

- (void)dealloc
{
    [draggedURL release];
    [super dealloc];
}

@end

@implementation WebHTMLView (WebPrivate)

+ (void)initialize
{
    [[WebNSTextView class] poseAsClass:[NSTextView class]];
}

- (void)_adjustFrames
{
    // Ick!  khtml set the frame size during layout and
    // the frame origins during drawing!  So we have to 
    // layout and do a draw with rendering disabled to
    // correclty adjust the frames.
    [[self _bridge] adjustFrames:[self frame]];
}

- (void)_reset
{
    NSArray *subviews = [[self subviews] copy];
    [subviews makeObjectsPerformSelector:@selector(_web_stopIfPluginView)];
    [subviews release];

    [WebImageRenderer stopAnimationsInView:self];
}

- (WebController *)_controller
{
    return [[self _web_parentWebView] _controller];
}

- (WebFrame *)_frame
{
    WebView *webView = [self _web_parentWebView];
    return [[webView _controller] frameForView:webView];
}

- (BOOL)_isMainFrame
{
    WebFrame *frame = [self _frame];
    return frame == [[frame controller] mainFrame];
}

// Required so view can access the part's selection.
- (WebBridge *)_bridge
{
    return [[self _frame] _bridge];
}

BOOL _modifierTrackingEnabled = FALSE;

+ (void)_setModifierTrackingEnabled:(BOOL)enabled
{
    _modifierTrackingEnabled = enabled;
}

+ (BOOL)_modifierTrackingEnabled
{
    return _modifierTrackingEnabled;
}

+ (void)_postFlagsChangedEvent:(NSEvent *)flagsChangedEvent
{
    NSEvent *fakeEvent = [NSEvent mouseEventWithType:NSMouseMoved location:[[flagsChangedEvent window] convertScreenToBase:[NSEvent mouseLocation]] modifierFlags:[flagsChangedEvent modifierFlags] timestamp:[flagsChangedEvent timestamp] windowNumber:[flagsChangedEvent windowNumber] context:[flagsChangedEvent context] eventNumber:0 clickCount:0 pressure:0];

    // pretend it's a mouse move
    [[NSNotificationCenter defaultCenter] postNotificationName:NSMouseMovedNotification object:self userInfo:[NSDictionary dictionaryWithObject:fakeEvent forKey:@"NSEvent"]];
}

- (NSDictionary *)_elementAtPoint:(NSPoint)point
{
    NSDictionary *elementInfoWC = [[self _bridge] elementAtPoint:point];
    NSMutableDictionary *elementInfo = [NSMutableDictionary dictionary];

    [elementInfo addEntriesFromDictionary: elementInfoWC];

    WebView *webView = [self _web_parentWebView];
    WebFrame *webFrame = [[webView _controller] frameForView:webView];
    [elementInfo setObject:webFrame forKey:WebContextMenuElementFrameKey];
       
    return elementInfo;
}

- (BOOL)_continueAfterClickPolicyForEvent:(NSEvent *)event
{
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    WebController *controller = [self _controller];
    WebClickPolicy *clickPolicy;

    clickPolicy = [[controller policyHandler] clickPolicyForElement:[self _elementAtPoint:point]
                                                             button:[event type]
                                                       modifierMask:[event modifierFlags]];

    WebPolicyAction clickAction = [clickPolicy policyAction];
    NSURL *URL = [clickPolicy URL];

    switch (clickAction) {
        case WebClickPolicyShow:
            return YES;
        case WebClickPolicyOpenNewWindow:
            [[controller windowContext] openNewWindowWithURL:URL referrer:nil];
            break;
        case WebClickPolicySave:
        case WebClickPolicySaveAndOpenExternally:
            [controller _downloadURL:URL toPath:[clickPolicy path]];
            break;
        case WebClickPolicyIgnore:
            break;
        default:
            [NSException raise:NSInvalidArgumentException
                        format:@"clickPolicyForElement:button:modifierMask: returned an invalid WebClickPolicy"];
    }
    return NO;
}


// Don't let AppKit even draw subviews. We take care of that.
- (void)_recursiveDisplayRectIfNeededIgnoringOpacity:(NSRect)rect isVisibleRect:(BOOL)isVisibleRect rectIsVisibleRectForView:(NSView *)visibleView topView:(BOOL)topView
{
    [_subviews makeObjectsPerformSelector:@selector(_web_propagateDirtyRectToAncestor)];

    ASSERT(!_private->subviewsSetAside);
    ASSERT(_private->savedSubviews == nil);
    _private->savedSubviews = _subviews;
    _subviews = nil;
    _private->subviewsSetAside = YES;
    
    [super _recursiveDisplayRectIfNeededIgnoringOpacity:rect isVisibleRect:isVisibleRect
        rectIsVisibleRectForView:visibleView topView:topView];
    
    ASSERT(_subviews == nil);
    _subviews = _private->savedSubviews;
    _private->savedSubviews = nil;
    _private->subviewsSetAside = NO;
}

// Don't let AppKit even draw subviews. We take care of that.
- (void)_recursiveDisplayAllDirtyWithLockFocus:(BOOL)needsLockFocus visRect:(NSRect)visRect
{
    BOOL setAsideSubviews = !_private->subviewsSetAside;
    
    if (setAsideSubviews) {
        [_subviews makeObjectsPerformSelector:@selector(_web_propagateDirtyRectToAncestor)];

        ASSERT(!_private->subviewsSetAside);
        ASSERT(_private->savedSubviews == nil);
        _private->savedSubviews = _subviews;
        _subviews = nil;
        _private->subviewsSetAside = YES;
    }
    
    [super _recursiveDisplayAllDirtyWithLockFocus:needsLockFocus visRect:visRect];
    
    if (setAsideSubviews) {
        ASSERT(_subviews == nil);
        _subviews = _private->savedSubviews;
        _private->savedSubviews = nil;
        _private->subviewsSetAside = NO;
    }
}

@end

@implementation NSView (WebHTMLViewPrivate)

- (void)_web_stopIfPluginView
{
    if ([self isKindOfClass:[WebNetscapePluginView class]]) {
	WebNetscapePluginView *pluginView = (WebNetscapePluginView *)self;
        [pluginView stop];
    }
}

- (void)_web_propagateDirtyRectToAncestor
{
    [_subviews makeObjectsPerformSelector:@selector(_web_propagateDirtyRectToAncestor)];
    if ([self needsDisplay]) {
        [[self superview] setNeedsDisplayInRect:[self convertRect:[self _dirtyRect] toView:[self superview]]];
    }
}

@end

@implementation WebNSTextView

static BOOL inDrawRect;

// This code is here to make insertion point drawing work in a way that respects the
// HTML view layering. If we can find a way to make it work without poseAsClass, we
// should do that.

- (BOOL)_web_inHTMLView
{
    NSView *view = self;
    for (;;) {
        NSView *superview = [view superview];
        if (!superview) {
            return NO;
        }
        view = superview;
        if ([view isKindOfClass:[WebHTMLView class]]) {
            return YES;
        }
    }
}

- (BOOL)isOpaque
{
    if (![self _web_inHTMLView]) {
        return [super isOpaque];
    }

    // Text views in the HTML view all say they are not opaque.
    // This prevents the insertion point rect cache from being used,
    // and all the side effects are good since we want the view to act
    // opaque anyway. This could go in NSView instead of NSTextView,
    // but we need to pose as NSTextView anyway for the other override.
    // If we did this in NSView, we wouldn't need to call _web_propagateDirtyRectToAncestor.
    return NO;
}

- (void)drawInsertionPointInRect:(NSRect)rect color:(NSColor *)color turnedOn:(BOOL)turnedOn
{
    if (![self _web_inHTMLView]) {
        [super drawInsertionPointInRect:rect color:color turnedOn:turnedOn];
        return;
    }
    
    // Use the display mechanism to do all insertion point drawing in the web view.
    if (inDrawRect) {
        [super drawInsertionPointInRect:rect color:color turnedOn:turnedOn];
        return;
    }
    [super drawInsertionPointInRect:rect color:color turnedOn:NO];
    if (turnedOn) {
        rect.size.width = 1;
        [self setNeedsDisplayInRect:rect];
    }
}

- (void)_drawRect:(NSRect)rect clip:(BOOL)clip
{
    inDrawRect = YES;
    [super _drawRect:rect clip:clip];
    inDrawRect = NO;
}

@end
