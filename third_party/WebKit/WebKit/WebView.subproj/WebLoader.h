/*	
    WebBaseResourceHandleDelegate.h
    Copyright (c) 2002, Apple Computer, Inc. All rights reserved.
*/

#import <Foundation/Foundation.h>

#import <WebKit/WebViewPrivate.h>

@class WebView;
@class WebDataSource;
@class WebError;
@class NSURLConnection;
@class NSURLRequest;
@class NSURLResponse;

@protocol NSURLConnectionDelegate;

@interface WebBaseResourceHandleDelegate : NSObject <NSURLConnectionDelegate>
{
@protected
    WebDataSource *dataSource;
    NSURLConnection *connection;
    NSURLRequest *request;
@private
    WebView *controller;
    NSURLResponse *response;
    id identifier;
    id resourceLoadDelegate;
    id downloadDelegate;
    NSURL *currentURL;
    BOOL reachedTerminalState;
    BOOL defersCallbacks;
    WebResourceDelegateImplementationCache implementations;
}

- (BOOL)loadWithRequest:(NSURLRequest *)request;

// this method exists only to be subclassed, don't call it directly
- (void)startLoading:(NSURLRequest *)r;

- (void)setDataSource:(WebDataSource *)d;
- (WebDataSource *)dataSource;

- resourceLoadDelegate;
- downloadDelegate;

- (void)cancel;
- (void)cancelWithError:(WebError *)error;

- (void)setDefersCallbacks:(BOOL)defers;
- (BOOL)defersCallbacks;

- (WebError *)cancelledError;

- (void)setIdentifier: ident;

@end
