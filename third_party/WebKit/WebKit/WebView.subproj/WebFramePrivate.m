/*	
    WebFramePrivate.m
	    
    Copyright 2001, 2002, Apple Computer, Inc. All rights reserved.
*/

#import <WebKit/WebFramePrivate.h>

#import <WebKit/WebBackForwardList.h>
#import <WebKit/WebBridge.h>
#import <WebKit/WebController.h>
#import <WebKit/WebControllerPolicyDelegate.h>
#import <WebKit/WebControllerPrivate.h>
#import <WebKit/WebDataSource.h>
#import <WebKit/WebDataSourcePrivate.h>
#import <WebKit/WebDocument.h>
#import <WebKit/WebDynamicScrollBarsView.h>
#import <WebKit/WebHistory.h>
#import <WebKit/WebHistoryItem.h>
#import <WebKit/WebHTMLView.h>
#import <WebKit/WebHTMLViewPrivate.h>
#import <WebKit/WebKitLogging.h>
#import <WebKit/WebKitErrors.h>
#import <WebKit/WebLocationChangeDelegate.h>
#import <WebKit/WebPluginController.h>
#import <WebKit/WebPreferencesPrivate.h>
#import <WebKit/WebViewPrivate.h>

#import <WebFoundation/WebError.h>
#import <WebFoundation/WebNSURLExtras.h>
#import <WebFoundation/WebNSStringExtras.h>
#import <WebFoundation/WebResourceHandle.h>
#import <WebFoundation/WebResourceRequest.h>
#import <WebFoundation/WebHTTPResourceRequest.h>

static const char * const stateNames[] = {
    "WebFrameStateProvisional",
    "WebFrameStateCommittedPage",
    "WebFrameStateLayoutAcceptable",
    "WebFrameStateComplete"
};

@implementation WebFramePrivate

- init
{
    self = [super init];
    if (!self) {
        return nil;
    }
    
    state = WebFrameStateComplete;
    loadType = WebFrameLoadTypeStandard;
    
    return self;
}

- (void)dealloc
{
    [webView _setController:nil];
    [dataSource _setController:nil];
    [provisionalDataSource _setController:nil];

    [name release];
    [webView release];
    [dataSource release];
    [provisionalDataSource release];
    [bridge release];
    [scheduledLayoutTimer release];
    [children release];
    [pluginController release];
    
    [super dealloc];
}

- (NSString *)name { return name; }
- (void)setName:(NSString *)n 
{
    NSString *newName = [n copy];
    [name release];
    name = newName;
}

- (WebView *)webView { return webView; }
- (void)setWebView: (WebView *)v 
{ 
    [v retain];
    [webView release];
    webView = v;
}

- (WebDataSource *)dataSource { return dataSource; }
- (void)setDataSource: (WebDataSource *)d
{
    [d retain];
    [dataSource release];
    dataSource = d;
}

- (WebController *)controller { return controller; }
- (void)setController: (WebController *)c
{ 
    controller = c; // not retained (yet)
}

- (WebDataSource *)provisionalDataSource { return provisionalDataSource; }
- (void)setProvisionalDataSource: (WebDataSource *)d
{ 
    [d retain];
    [provisionalDataSource release];
    provisionalDataSource = d;
}

- (WebFrameLoadType)loadType { return loadType; }
- (void)setLoadType: (WebFrameLoadType)t
{
    loadType = t;
}

@end

@implementation WebFrame (WebPrivate)

- (WebFrame *)_descendantFrameNamed:(NSString *)name
{
    if ([[self name] isEqualToString: name]){
        return self;
    }

    NSArray *children = [self children];
    WebFrame *frame;
    unsigned i;

    for (i = 0; i < [children count]; i++){
        frame = [children objectAtIndex: i];
        frame = [frame _descendantFrameNamed:name];
        if (frame){
            return frame;
        }
    }

    return nil;
}

- (void)_controllerWillBeDeallocated
{
    [self _detachFromParent];
}

- (void)_detachFromParent
{
    WebBridge *bridge = _private->bridge;
    _private->bridge = nil;
    
    [bridge closeURL];

    [[self children] makeObjectsPerformSelector:@selector(_detachFromParent)];
    
    [_private setController:nil];
    [_private->webView _setController:nil];
    [_private->dataSource _setController:nil];
    [_private->provisionalDataSource _setController:nil];

    [_private setDataSource:nil];
    [_private setWebView:nil];

    [_private->scheduledLayoutTimer invalidate];
    [_private->scheduledLayoutTimer release];
    _private->scheduledLayoutTimer = nil;
    
    [bridge release];
}

- (void)_setController: (WebController *)controller
{
    [_private setController:controller];
}

- (void)_setDataSource:(WebDataSource *)ds
{
    ASSERT(ds != _private->dataSource);
    
    if ([_private->dataSource isDocumentHTML] && ![ds isDocumentHTML]) {
        [_private->bridge removeFromFrame];
    }

    [[self children] makeObjectsPerformSelector:@selector(_detachFromParent)];
    [_private->children release];
    _private->children = nil;
    
    [_private setDataSource:ds];
    [ds _setController:[self controller]];
}

- (void)_setLoadType: (WebFrameLoadType)t
{
    [_private setLoadType: t];
}

- (WebFrameLoadType)_loadType
{
    return [_private loadType];
}

- (void)_scheduleLayout:(NSTimeInterval)inSeconds
{
    // FIXME: Maybe this should have the code to move up the deadline if the new interval brings the time even closer.
    if (_private->scheduledLayoutTimer == nil) {
        _private->scheduledLayoutTimer = [[NSTimer scheduledTimerWithTimeInterval:inSeconds target:self selector:@selector(_timedLayout:) userInfo:nil repeats:FALSE] retain];
    }
}

- (void)_timedLayout: (id)userInfo
{
    LOG(Timing, "%s:  state = %s", [[self name] cString], stateNames[_private->state]);
    
    [_private->scheduledLayoutTimer release];
    _private->scheduledLayoutTimer = nil;
    
    if (_private->state == WebFrameStateLayoutAcceptable) {
        NSView <WebDocumentView> *documentView = [[self webView] documentView];
        
        if ([self controller])
            LOG(Timing, "%s:  performing timed layout, %f seconds since start of document load", [[self name] cString], CFAbsoluteTimeGetCurrent() - [[[[self controller] mainFrame] dataSource] _loadingStartedTime]);
            
        [documentView setNeedsLayout: YES];

        if ([documentView isKindOfClass: [NSView class]]) {
            NSView *dview = (NSView *)documentView;
            
            
            NSRect frame = [dview frame];
            
            if (frame.size.width == 0 || frame.size.height == 0){
                // We must do the layout now, rather than depend on
                // display to do a lazy layout because the view
                // may be recently initialized with a zero size
                // and the AppKit will optimize out any drawing.
                
                // Force a layout now.  At this point we could
                // check to see if any CSS is pending and delay
                // the layout further to avoid the flash of unstyled
                // content.                    
                [documentView layout];
            }
        }
            
        [documentView setNeedsDisplay: YES];
    }
    else {
        if ([self controller])
            LOG(Timing, "%s:  NOT performing timed layout (not needed), %f seconds since start of document load", [[self name] cString], CFAbsoluteTimeGetCurrent() - [[[[self controller] mainFrame] dataSource] _loadingStartedTime]);
    }
}


- (void)_transitionToLayoutAcceptable
{
    switch ([self _state]) {
    	case WebFrameStateCommittedPage:
        {
            [self _setState: WebFrameStateLayoutAcceptable];
                    
            // Start a timer to guarantee that we get an initial layout after
            // X interval, even if the document and resources are not completely
            // loaded.
            BOOL timedDelayEnabled = [[WebPreferences standardPreferences] _initialTimedLayoutEnabled];
            if (timedDelayEnabled) {
                NSTimeInterval defaultTimedDelay = [[WebPreferences standardPreferences] _initialTimedLayoutDelay];
                double timeSinceStart;

                // If the delay getting to the commited state exceeds the initial layout delay, go
                // ahead and schedule a layout.
                timeSinceStart = (CFAbsoluteTimeGetCurrent() - [[self dataSource] _loadingStartedTime]);
                if (timeSinceStart > (double)defaultTimedDelay) {
                    LOG(Timing, "performing early layout because commit time, %f, exceeded initial layout interval %f", timeSinceStart, defaultTimedDelay);
                    [self _timedLayout: nil];
                }
                else {
                    NSTimeInterval timedDelay = defaultTimedDelay - timeSinceStart;
                    
                    LOG(Timing, "registering delayed layout after %f seconds, time since start %f", timedDelay, timeSinceStart);
                    [self _scheduleLayout: timedDelay];
                }
            }
            break;
        }

        case WebFrameStateProvisional:
        case WebFrameStateComplete:
        case WebFrameStateLayoutAcceptable:
        {
            break;
        }
        
        default:
        {
	    ASSERT_NOT_REACHED();
        }
    }
}


- (void)_transitionToCommitted
{
    ASSERT([self controller] != nil);
    NSView <WebDocumentView> *documentView;
    WebHistoryItem *backForwardItem;
    WebBackForwardList *backForwardList = [[self controller] backForwardList];
    WebFrame *parentFrame;
    
    documentView = [[self webView] documentView];

    // Destroy plug-ins before blowing away the view.
    [_private->pluginController destroyAllPlugins];
        
    switch ([self _state]) {
    	case WebFrameStateProvisional:
        {
	    ASSERT(documentView != nil);

            // Set the committed data source on the frame.
            [self _setDataSource:_private->provisionalDataSource];
            [_private setProvisionalDataSource: nil];

            [self _setState: WebFrameStateCommittedPage];
        
            // Handle adding the URL to the back/forward list.
            WebDataSource *ds = [self dataSource];
            WebHistoryItem *entry = nil;
            NSString *ptitle = [ds pageTitle];

            if ([[self controller] usesBackForwardList]){
                switch ([self _loadType]) {
                case WebFrameLoadTypeForward:
                case WebFrameLoadTypeBack:
                case WebFrameLoadTypeIndexedBackForward:
                    [backForwardList goToEntry: [ds _provisionalBackForwardItem]];
                    [self _restoreScrollPosition];
                    break;
                    
                case WebFrameLoadTypeReload:
                    [self _scrollToTop];
                    break;
    
                case WebFrameLoadTypeStandard:
                    // Add item to history.
                    entry = [[WebHistory sharedHistory] addEntryForURL: [[[ds _originalRequest] URL] _web_canonicalize]];
                    if (ptitle)
                        [entry setTitle: ptitle];

                    if (![ds _isClientRedirect]) {
                        // Add item to back/forward list.
                        parentFrame = [self parent];
                        backForwardItem = [[WebHistoryItem alloc] initWithURL:[[ds request] URL]
                                                                       target:[self name]
                                                                       parent:[parentFrame name]
                                                                        title:ptitle];
                        [[[self controller] backForwardList] addEntry: backForwardItem];
                        [ds _addBackForwardItem:backForwardItem];
                        [backForwardItem release];
                    } else {
                        // update the URL in the BF list that we made before the redirect
                        [[[[self controller] backForwardList] currentEntry] setURL:[[ds request] URL]];
                    }
                    break;
                    
                case WebFrameLoadTypeInternal:
                    // Do nothing, this was a frame/iframe non user load.
                case WebFrameLoadTypeReloadAllowingStaleData:
                    break;
                    
                // FIXME Remove this check when dummy ds is removed.  An exception should be thrown
                // if we're in the WebFrameLoadTypeUninitialized state.
                default:
		    ASSERT_NOT_REACHED();
                }
            }

            // Tell the client we've committed this URL.
	    [[[self controller] locationChangeDelegate] locationChangeCommittedForDataSource:ds];
            
            // If we have a title let the controller know about it.
            if (ptitle) {
                [entry setTitle:ptitle];
		[[[self controller] locationChangeDelegate] receivedPageTitle:ptitle forDataSource:ds];
            }
            break;
        }
        
        case WebFrameStateCommittedPage:
        case WebFrameStateLayoutAcceptable:
        case WebFrameStateComplete:
        default:
        {
	    ASSERT_NOT_REACHED();
        }
    }
}

- (WebFrameState)_state
{
    return _private->state;
}

- (void)_setState: (WebFrameState)newState
{
    LOG(Loading, "%s:  transition from %s to %s", [[self name] cString], stateNames[_private->state], stateNames[newState]);
    if ([self controller])
        LOG(Timing, "%s:  transition from %s to %s, %f seconds since start of document load", [[self name] cString], stateNames[_private->state], stateNames[newState], CFAbsoluteTimeGetCurrent() - [[[[self controller] mainFrame] dataSource] _loadingStartedTime]);
    
    if (newState == WebFrameStateComplete && self == [[self controller] mainFrame]){
        LOG(DocumentLoad, "completed %s (%f seconds)", [[[[[self dataSource] request] URL] absoluteString] cString], CFAbsoluteTimeGetCurrent() - [[self dataSource] _loadingStartedTime]);
    }
    
    NSDictionary *userInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                    [NSNumber numberWithInt:_private->state], WebPreviousFrameState,
                    [NSNumber numberWithInt:newState], WebCurrentFrameState, nil];
                    
    [[NSNotificationCenter defaultCenter] postNotificationName:WebFrameStateChangedNotification object:self userInfo:userInfo];
    
    _private->state = newState;
    
    if (_private->state == WebFrameStateProvisional) {
        // FIXME: This is OK as long as no one resizes the window,
        // but in the case where someone does, it means garbage outside
        // the occupied part of the scroll view.
        [[[self webView] frameScrollView] setDrawsBackground:NO];
    }
    
    if (_private->state == WebFrameStateComplete) {
        NSScrollView *sv = [[self webView] frameScrollView];
        [sv setDrawsBackground:YES];
        // FIXME: This overrides the setCopiesOnScroll setting done by
        // WebCore based on whether the page's contents are dynamic or not.
        [[sv contentView] setCopiesOnScroll:YES];
        [_private->scheduledLayoutTimer fire];
   	ASSERT(_private->scheduledLayoutTimer == nil);
    }
}

- (void)_isLoadComplete
{
    ASSERT([self controller] != nil);

    switch ([self _state]) {
        case WebFrameStateProvisional:
        {
            WebDataSource *pd = [self provisionalDataSource];
            
            LOG(Loading, "%s:  checking complete in WebFrameStateProvisional", [[self name] cString]);
            // If we've received any errors we may be stuck in the provisional state and actually
            // complete.
            if ([pd mainDocumentError]) {
                // Check all children first.
                LOG(Loading, "%s:  checking complete, current state WebFrameStateProvisional", [[self name] cString]);
                if (![pd isLoading]) {
                    LOG(Loading, "%s:  checking complete in WebFrameStateProvisional, load done", [[self name] cString]);

                    [[[self controller] locationChangeDelegate] locationChangeDone: [pd mainDocumentError] forDataSource:pd];

                    // We know the provisional data source didn't cut the mustard, release it.
                    [_private setProvisionalDataSource: nil];
                    
                    [self _setState: WebFrameStateComplete];
                    return;
                }
            }
            return;
        }
        
        case WebFrameStateCommittedPage:
        case WebFrameStateLayoutAcceptable:
        {
            WebDataSource *ds = [self dataSource];
            
            //LOG(Loading, "%s:  checking complete, current state WEBFRAMESTATE_COMMITTED", [[self name] cString]);
            if (![ds isLoading]) {
                id thisView = [self webView];
                NSView <WebDocumentView> *thisDocumentView = [thisView documentView];

                [self _setState: WebFrameStateComplete];

		// FIXME: need to avoid doing this in the non-HTML
		// case or the bridge may assert. Should make sure
		// there is a bridge/part in the proper state even for
		// non-HTML content.

                if ([ds isDocumentHTML]) {
		    [_private->bridge end];
		}

                // Unfortunately we have to get our parent to adjust the frames in this
                // frameset so this frame's geometry is set correctly.  This should
                // be a reasonably inexpensive operation.
                id parentDS = [[self parent] dataSource];
                if ([[parentDS _bridge] isFrameSet]){
                    id parentWebView = [[self parent] webView];
                    if ([parentWebView isDocumentHTML])
                        [[parentWebView documentView] _adjustFrames];
                }

                // Tell the just loaded document to layout.  This may be necessary
                // for non-html content that needs a layout message.
                [thisDocumentView setNeedsLayout: YES];
                [thisDocumentView layout];

                // Unfortunately if this frame has children we have to lay them
                // out too.  This could be an expensive operation.
                // FIXME:  If we can figure out how to avoid the layout of children,
                // (just need for iframe placement/sizing) we could get a few percent
                // speed improvement.
                [ds _layoutChildren];

                [thisDocumentView setNeedsDisplay: YES];
                //[thisDocumentView display];

                // If the user had a scroll point scroll to it.  This will override
                // the anchor point.  After much discussion it was decided by folks
                // that the user scroll point should override the anchor point.
                if ([[self controller] usesBackForwardList]){
                    switch ([self _loadType]) {
                    case WebFrameLoadTypeForward:
                    case WebFrameLoadTypeBack:
                    case WebFrameLoadTypeIndexedBackForward:
                        [self _restoreScrollPosition];
                        break;
                        
                    case WebFrameLoadTypeReload:
                        [self _scrollToTop];
                        break;
        
                    case WebFrameLoadTypeStandard:
                    case WebFrameLoadTypeInternal:
                    case WebFrameLoadTypeReloadAllowingStaleData:
                        // Do nothing.
                        break;
                        
                    default:
                        ASSERT_NOT_REACHED();
                        break;
                    }
                }

                [[[self controller] locationChangeDelegate] locationChangeDone: [ds mainDocumentError] forDataSource:ds];
 
                //if ([ds isDocumentHTML])
                //    [[ds representation] part]->closeURL();        
               
                return;
            }
            // A resource was loaded, but the entire frame isn't complete.  Schedule a
            // layout.
            else {
                if ([self _state] == WebFrameStateLayoutAcceptable) {
                    BOOL resourceTimedDelayEnabled = [[WebPreferences standardPreferences] _resourceTimedLayoutEnabled];
                    if (resourceTimedDelayEnabled) {
                        NSTimeInterval timedDelay = [[WebPreferences standardPreferences] _resourceTimedLayoutDelay];
                        [self _scheduleLayout: timedDelay];
                    }
                }
            }
            return;
        }
        
        case WebFrameStateComplete:
        {
            LOG(Loading, "%s:  checking complete, current state WebFrameStateComplete", [[self name] cString]);
            return;
        }
        
        // Yikes!  Serious horkage.
        default:
        {
	    ASSERT_NOT_REACHED();
        }
    }
}

+ (void)_recursiveCheckCompleteFromFrame: (WebFrame *)fromFrame
{
    int i, count;
    NSArray *childFrames;
    
    childFrames = [fromFrame children];
    count = [childFrames count];
    for (i = 0; i < count; i++) {
        WebFrame *childFrame;
        
        childFrame = [childFrames objectAtIndex: i];
        [WebFrame _recursiveCheckCompleteFromFrame: childFrame];
        [childFrame _isLoadComplete];
    }
    [fromFrame _isLoadComplete];
}

// Called every time a resource is completely loaded, or an error is received.
- (void)_checkLoadComplete
{
    ASSERT([self controller] != nil);

    // Now walk the frame tree to see if any frame that may have initiated a load is done.
    [WebFrame _recursiveCheckCompleteFromFrame: [[self controller] mainFrame]];
}

- (WebBridge *)_bridge
{
    return _private->bridge;
}

- (void)handleUnimplementablePolicy:(WebPolicy *)policy errorCode:(int)code forURL:(NSURL *)URL
{
    WebError *error = [WebError errorWithCode:code
                                     inDomain:WebErrorDomainWebKit
                                   failingURL:[URL absoluteString]];
    [[[self controller] policyDelegate] unableToImplementPolicy:policy error:error forURL:URL inFrame:self];    
}

- (BOOL)_shouldShowRequest:(WebResourceRequest *)request
{
    id <WebControllerPolicyDelegate> policyDelegate = [[self controller] policyDelegate];
    WebURLPolicy *URLPolicy = [policyDelegate URLPolicyForRequest:request inFrame:self];

    switch ([URLPolicy policyAction]) {
        case WebURLPolicyIgnore:
            return NO;

        case WebURLPolicyOpenExternally:
            if(![[NSWorkspace sharedWorkspace] openURL:[request URL]]){
                [self handleUnimplementablePolicy:URLPolicy errorCode:WebErrorCannotNotFindApplicationForURL forURL:[request URL]];
            }
            return NO;

        case WebURLPolicyUseContentPolicy:
            // handle non-file case first because it's short and sweet
            if (![[request URL] isFileURL]) {
                if (![WebResourceHandle canInitWithRequest:request]) {
                    [self handleUnimplementablePolicy:URLPolicy errorCode:WebErrorCannotShowURL forURL:[request URL]];
                    return NO;
                }
                return YES;
            } else {
                // file URL
                NSFileManager *fileManager = [NSFileManager defaultManager];
                NSString *path = [[request URL] path];
                NSString *type = [WebController _MIMETypeForFile: path];
                WebFileURLPolicy *fileURLPolicy = [policyDelegate fileURLPolicyForMIMEType:type andRequest:request inFrame:self];

                if([fileURLPolicy policyAction] == WebFileURLPolicyIgnore)
                    return NO;

		BOOL isDirectory;
		BOOL fileExists = [fileManager fileExistsAtPath:[[request URL] path] isDirectory:&isDirectory];

                if(!fileExists){
                    [self handleUnimplementablePolicy:fileURLPolicy errorCode:WebErrorCannotFindFile forURL:[request URL]];
                    return NO;
                }

                if(![fileManager isReadableFileAtPath:path]){
                    [self handleUnimplementablePolicy:fileURLPolicy errorCode:WebErrorCannotReadFile forURL:[request URL]];
                    return NO;
                }

                switch ([fileURLPolicy policyAction]) {
                    case WebFileURLPolicyUseContentPolicy:
                        if (isDirectory) {
                            [self handleUnimplementablePolicy:fileURLPolicy errorCode:WebErrorCannotShowDirectory forURL:[request URL]];
                            return NO;
                        } else if (![WebController canShowMIMEType: type]) {
                            [self handleUnimplementablePolicy:fileURLPolicy errorCode:WebErrorCannotShowMIMEType forURL:[request URL]];
                            return NO;
                        }
                        return YES;
                        
                    case WebFileURLPolicyOpenExternally:
                        if(![[NSWorkspace sharedWorkspace] openFile:path]){
                            [self handleUnimplementablePolicy:fileURLPolicy errorCode:WebErrorCannotFindApplicationForFile forURL:[request URL]];
                        }
                        return NO;

                    case WebFileURLPolicyRevealInFinder:
                        if(![[NSWorkspace sharedWorkspace] selectFile:path inFileViewerRootedAtPath:@""]){
                            [self handleUnimplementablePolicy:fileURLPolicy errorCode:WebErrorFinderCannotOpenDirectory forURL:[request URL]];
                        }
                        return NO;

                    default:
                        [NSException raise:NSInvalidArgumentException format:
                @"fileURLPolicyForMIMEType:inFrame:isDirectory: returned WebFileURLPolicy with invalid action %d", [fileURLPolicy policyAction]];
                        return NO;
                }
            }

        default:
            [NSException raise:NSInvalidArgumentException format:@"URLPolicyForRequest: returned an invalid WebURLPolicy"];
            return NO;
    }
}

- (void)_setProvisionalDataSource:(WebDataSource *)d
{
    [_private setProvisionalDataSource:d];
}

// main funnel for navigating via back/forward
- (void)_goToItem: (WebHistoryItem *)item withFrameLoadType: (WebFrameLoadType)type
{
    NSURL *itemURL = [item URL];
    WebResourceRequest *request;
    NSURL *originalURL = [[[self dataSource] request] URL];
    WebBackForwardList *backForwardList = [[self controller] backForwardList];

    // Are we navigating to an anchor within the page?
    if ([item anchor] && [[itemURL _web_URLByRemovingFragment] isEqual: [originalURL _web_URLByRemovingFragment]]) {
        if (type == WebFrameLoadTypeForward)
            [backForwardList goForward];
        else if (type == WebFrameLoadTypeBack)
            [backForwardList goBack];
        else if (type == WebFrameLoadTypeIndexedBackForward)
            [backForwardList goToEntry:item];
        else 
            [NSException raise:NSInvalidArgumentException format:@"WebFrameLoadType incorrect"];
        [[_private->dataSource _bridge] scrollToAnchor: [item anchor]];
        [[[self controller] locationChangeDelegate] locationChangedWithinPageForDataSource:_private->dataSource];
    }
    else {
        request = [[WebResourceRequest alloc] initWithURL:itemURL];
    
        // set the request cache policy based on the type of request we have
        // however, allow any previously set value to take precendence
        if ([request requestCachePolicy] == WebRequestCachePolicyUseProtocolDefault) {
            switch (type) {
                case WebFrameLoadTypeStandard:
                    // if it's not a GET, reload from origin
                    // unsure whether this is the best policy
                    // other methods might be OK to get from the cache
                    if (![[request method] _web_isCaseInsensitiveEqualToString:@"GET"]) {
                        [request setRequestCachePolicy:WebRequestCachePolicyLoadFromOrigin];
                    }
                    break;
                case WebFrameLoadTypeReload:
                    [request setRequestCachePolicy:WebRequestCachePolicyLoadFromOrigin];
                    break;
                case WebFrameLoadTypeBack:
                case WebFrameLoadTypeForward:
                case WebFrameLoadTypeIndexedBackForward:
                    [request setRequestCachePolicy:WebRequestCachePolicyReturnCacheObjectLoadFromOriginIfNoCacheObject];
                    break;
                case WebFrameLoadTypeInternal:
                case WebFrameLoadTypeReloadAllowingStaleData:
                    // no-op: leave as protocol default
                    break;
                default:
                    ASSERT_NOT_REACHED();
            }
        }

        WebDataSource *newDataSource = [[WebDataSource alloc] initWithRequest:request];
        [request release];
        [self setProvisionalDataSource: newDataSource];
        // Remember this item in order to set the position in the BFList later.
        // Important that this happens after our provDataSrc is set, since setting
        // provDataSrc results in isLoadComplete, which clears provLink!
        [newDataSource _setProvisionalBackForwardItem: item];
        // Set the item for which we will save document state
        [newDataSource _setPreviousBackForwardItem: [backForwardList currentEntry]];
        [self _setLoadType: type];
        [self startLoading];
        [newDataSource release];
    }
}

- (void)_loadRequest:(WebResourceRequest *)request
{
    WebDataSource *newDataSource = [[WebDataSource alloc] initWithRequest:request];
    if ([self setProvisionalDataSource:newDataSource]) {
        [self startLoading];
    }
    [newDataSource release];
}

// main funnel for navigating via callback from WebCore (e.g., clicking a link, redirect)
- (void)_loadURL:(NSURL *)URL loadType:(WebFrameLoadType)loadType clientRedirect:(BOOL)clientRedirect
{
    // FIXME: This logic doesn't exactly match what KHTML does in openURL, so it's possible
    // this will screw up in some cases involving framesets.
    if (loadType != WebFrameLoadTypeReload && [[URL _web_URLByRemovingFragment] isEqual:[[_private->bridge URL] _web_URLByRemovingFragment]]) {
        [_private->bridge openURL:URL];

        WebDataSource *dataSource = [self dataSource];
        WebHistoryItem *backForwardItem = [[WebHistoryItem alloc] initWithURL:URL target:[self name] parent:[[self parent] name] title:[dataSource pageTitle]];
        [backForwardItem setAnchor:[URL fragment]];
        [[[self controller] backForwardList] addEntry:backForwardItem];
        [backForwardItem release];

        [dataSource _setURL:URL];
        [dataSource _addBackForwardItem:backForwardItem];
        [[[self controller] locationChangeDelegate] locationChangedWithinPageForDataSource:dataSource];
    } else {
        WebFrameLoadType previousLoadType = [self _loadType];
        WebDataSource *oldDataSource = [[self dataSource] retain];
        WebResourceRequest *request = [[WebResourceRequest alloc] initWithURL:URL];
        [request setReferrer:[_private->bridge referrer]];
        if (loadType == WebFrameLoadTypeReload) {
            [request setRequestCachePolicy:WebRequestCachePolicyLoadFromOrigin];
        }
        [self _loadRequest:request];
        // NB: must be done after loadRequest:, which sets the provDataSource, which
        //     inits the load type to Standard
        [self _setLoadType:loadType];
        if (clientRedirect) {
            // Inherit the loadType from the operation that spawned the redirect
            [self _setLoadType:previousLoadType];

            // need to transfer BF items from the dataSource that we're replacing
            WebDataSource *newDataSource = [self provisionalDataSource];
            [newDataSource _setIsClientRedirect:YES];
            [newDataSource _setProvisionalBackForwardItem:[oldDataSource _provisionalBackForwardItem]];
            [newDataSource _setPreviousBackForwardItem:[oldDataSource _previousBackForwardItem]];
            [newDataSource _addBackForwardItems:[oldDataSource _backForwardItems]];
        }
        [request release];
        [oldDataSource release];
    }
}

- (void)_postWithURL:(NSURL *)URL data:(NSData *)data contentType:(NSString *)contentType
{
    // When posting, use the WebResourceHandleFlagLoadFromOrigin load flag.
    // This prevents a potential bug which may cause a page with a form that uses itself
    // as an action to be returned from the cache without submitting.
    WebResourceRequest *request = [[WebResourceRequest alloc] initWithURL:URL];
    [request setRequestCachePolicy:WebRequestCachePolicyLoadFromOrigin];
    [request setMethod:@"POST"];
    [request setData:data];
    [request setContentType:contentType];
    [request setReferrer:[_private->bridge referrer]];
    [self _loadRequest:request];
    [request release];
}

- (void)_restoreScrollPosition
{
    WebHistoryItem *entry;

    entry = (WebHistoryItem *)[[[self controller] backForwardList] currentEntry];
    [[[self webView] documentView] scrollPoint: [entry scrollPoint]];
}

- (void)_scrollToTop
{
    NSPoint origin;

    origin.x = origin.y = 0.0;
    [[[self webView] documentView] scrollPoint: origin];
}

- (void)_textSizeMultiplierChanged
{
    [_private->bridge setTextSizeMultiplier:[[self controller] textSizeMultiplier]];
    [[self children] makeObjectsPerformSelector:@selector(_textSizeMultiplierChanged)];
}

- (void)_defersCallbacksChanged
{
    [[self provisionalDataSource] _defersCallbacksChanged];
    [[self dataSource] _defersCallbacksChanged];
}

- (void)_reloadAllowingStaleDataWithOverrideEncoding:(NSString *)encoding
{
    WebDataSource *dataSource = [self dataSource];
    if (dataSource == nil) {
	return;
    }

    WebResourceRequest *request = [[dataSource request] copy];
    [request setRequestCachePolicy:WebRequestCachePolicyReturnCacheObjectLoadFromOriginIfNoCacheObject];
    WebDataSource *newDataSource = [[WebDataSource alloc] initWithRequest:request];
    [request release];
    
    [newDataSource _setOverrideEncoding:encoding];
    
    if ([self setProvisionalDataSource:newDataSource]) {
	[self _setLoadType:WebFrameLoadTypeReloadAllowingStaleData];
        [self startLoading];
    }
    
    [newDataSource release];
}

- (void)_addChild:(WebFrame *)child
{
    if (_private->children == nil)
        _private->children = [[NSMutableArray alloc] init];
    [_private->children addObject:child];

    child->_private->parent = self;
    [[child _bridge] setParent:_private->bridge];
    [[child dataSource] _setOverrideEncoding:[[self dataSource] _overrideEncoding]];   
}

- (WebPluginController *)_pluginController
{
    if(!_private->pluginController){
        _private->pluginController = [[WebPluginController alloc] initWithWebFrame:self];
    }

    return _private->pluginController;
}

@end
