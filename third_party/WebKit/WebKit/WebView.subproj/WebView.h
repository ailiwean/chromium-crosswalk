/*	
        IFWebController.h
	Copyright 2001, Apple, Inc. All rights reserved.

        Public header file.
*/
#import <Cocoa/Cocoa.h>

#import <WebKit/IFLoadProgress.h>


/*
   ============================================================================= 

    IFWebController manages the interaction between IFWebView(s) and IFWebDataSource(s).

    The IFWebController implements required behavior.  IFWebView and IFWebDataSource
    cannot function without a controller.  
    
    It it expected that alternate implementations of IFWebController will be written for
    alternate views of the web pages described by IFWebDataSources.  For example, a web
    crawler may implement a IFWebController with no corresponding view.
    
    IFDefaultWebController may be subclassed to modify the behavior of the standard
    IFWebView and IFWebDataSource.

   ============================================================================= 
    
    Changes: 
    
    2001-12-12
        Changed IFConcreteWebController to IFDefaultWebController.
        
        Changed IFLocationChangedHandler naming, replace "loadingXXX" with
        "locationChangeXXX".

        Changed loadingStopped in IFLocationChangedHandler to locationChangeStopped:(IFError *).

        Changed loadingCancelled in IFLocationChangedHandler to locationChangeCancelled:(IFError *).
        
        Changed loadedPageTitle in IFLocationChangedHandler to receivedPageTitle:.

        Added inputURL:(NSURL *) resolvedTo: (NSURL *) to IFLocationChangedHandler.
        
        Added the following two methods to IFLocationChangedHandler:
        
            - (void)inputURL: (NSURL *)inputURL resolvedTo: (NSURL *)resolvedURL;
            - (void)serverRedirectTo: (NSURL *)url;
       
        Put locationWillChangeTo: back on IFLocationChangedHandler.
        
        Changed XXXforLocation in IFLoadHandler to XXXforResource.
        
        Changed timeoutForLocation: in IFLoadHandler to receivedError:forResource:partialProgress:
        
        Added the following two methods to IFDefaultWebController:
        
            - setDirectsAllLinksToSystemBrowser: (BOOL)flag
            - (BOOL)directsAllLinksToSystemBrowser;
            
        Removed IFError.  This will be described in IFError.h.
  
  2001-12-13
  
        Removed IFFrameSetHandler, placed that functionality on IFWebDataSource.
        
        Changed IFLocationChangeHandler to add a parameter specifying the data source
        that sent the message.

  2001-12-14

        Removed inputURL:resolvedTo: methods, per discussion with Don.

        Remove IFContextMenuHandler for want of a better way to describe the
        not-yet-existing IFDOMNode.  We can't think of any initial clients that want
        to override the default behavior anyway.  Put it in IFGrabBag.h for now.

        Simplified IFLocationChangeHandler and updated
	IFAuthenticationHandler to use interfaces instead of structs..
*/


@class IFWebDataSource;
@class IFError;
@class IFWebView;
@class IFWebFrame;

/*
   ============================================================================= 

    A frame-aware version of the the IFLocationChangeHandler

    See the comments in IFWebPageView above for more description about this protocol.
*/
@protocol IFLocationChangeHandler

// This API will need to be extended to support some notion of the context in which
// a location is changing, i.e. was it initiated by a user click, by a programmatic
// manipulation of the DOM, is the frame an iframe, or a frame.

// locationWillChangeTo: is required, but will it be sent by the dataSource?  More
// likely the controller will receive a change request from the view.  That argues for
// placing locationWillChangeTo: in a different protocol, and making it more or a complete
// handshake.
- (BOOL)locationWillChangeTo: (NSURL *)url forFrame: (IFWebFrame *)frame;

- (void)locationChangeStartedForFrame: (IFWebFrame *)frame;

- (void)locationChangeCommittedForFrame: (IFWebFrame *)frame;

- (void)locationChangeDone: (IFError *)error forFrame: (IFWebFrame *)frame;

- (void)receivedPageTitle: (NSString *)title forDataSource: (IFWebDataSource *)dataSource;

- (void)serverRedirectTo: (NSURL *)url forDataSource: (IFWebDataSource *)dataSource;

@end


/*
   ============================================================================= 

    Implementors of this protocol will receive messages indicating
    data has been received.
    
    The methods in this protocol will be called even if the data source
    is initialized with something other than a URL.
*/
@protocol  IFLoadHandler

/*
    A new chunk of data has been received.  This could be a partial load
    of a url.  It may be useful to do incremental layout, although
    typically for non-base URLs this should be done after a URL (i.e. image)
    has been completely downloaded.
*/
- (void)receivedProgress: (IFLoadProgress *)progress forResource: (NSString *)resourceDescription fromDataSource: (IFWebDataSource *)dataSource;

- (void)receivedError: (IFError *)error forResource: (NSString *)resourceDescription partialProgress: (IFLoadProgress *)progress fromDataSource: (IFWebDataSource *)dataSource;

@end



/*
   ============================================================================= 

*/
@protocol IFWebDataSourceErrorHandler
- receivedError: (IFError *)error forDataSource: (IFWebDataSource *)dataSource;
@end


/*
   ============================================================================= 

    A class that implements IFScriptContextHandler provides all the state information
    that may be used by Javascript (AppleScript?).
    
*/
@protocol IFScriptContextHandler

// setStatusText and statusText are used by Javascript's status bar text methods.
- (void)setStatusText: (NSString *)text forDataSource: (IFWebDataSource *)dataSource;
- (NSString *)statusTextForDataSource: (IFWebDataSource *)dataSource;

// Need API for things like window size and position, window ids,
// screen goemetry.  Essentially all the 'view' items that are
// accessible from Javascript.

@end





/*
   ============================================================================= 

    IFWebController implements all the behavior that ties together IFWebView
    and IFWebDataSource.  See each inherited protocol for a more complete
    description.
    
    [Don and I both agree that all these little protocols are useful to cleanly
     describe snippets of behavior, but do we explicity reference them anywhere,
     or do we just use the umbrella protocol?]
*/
@protocol IFWebController <IFLoadHandler, IFScriptContextHandler, IFLocationChangeHandler>


// Called when a data source needs to create a frame.  This method encapsulates the
// specifics of creating and initializaing a view of the appropriate class.
- (IFWebFrame *)createFrameNamed: (NSString *)fname for: (IFWebDataSource *)child inParent: (IFWebDataSource *)parent inScrollView: (BOOL)inScrollView;

- (IFWebFrame *)mainFrame;


// Return the frame associated with the data source.  Traverses the
// frame tree to find the data source.
- (IFWebFrame *)frameForDataSource: (IFWebDataSource *)dataSource;


@end



