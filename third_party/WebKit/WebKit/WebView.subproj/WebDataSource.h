/*	
        WebDataSource.h
	Copyright 2001, Apple, Inc. All rights reserved.

        Public header file.
*/

#import <Cocoa/Cocoa.h>

@class WebController;
@class WebDataSourcePrivate;
@class WebError;
@class WebFrame;
@class WebResource;
@class WebRequest;
@class WebResponse;

@protocol WebDocumentRepresentation;

/*!
    @class WebDataSource
    @discussion A WebDataSource represents the data associated with a web page.
    A datasource has a WebDocumentRepresentation which holds an appropriate
    representation of the data.  WebDataSources manage a hierarchy of WebFrames.
    WebDataSources are typically related to a view by their containing WebFrame.
*/
@interface WebDataSource : NSObject
{
@private
    WebDataSourcePrivate *_private;
}

/*!
    @method initWithRequest:
    @abstract The designated initializer for WebDataSource.
    @param request The request to use in creating a datasource.
    @result Returns an initialized WebDataSource.
*/
- (id)initWithRequest:(WebRequest *)request;

/*!
    @method data
    @discussion The data associated with a datasource will not be valid until
    a datasource has completely loaded.  
    @result Returns the raw data associated with this datasource.  Returns nil
    if the datasource hasn't loaded.
*/
- (NSData *)data;

/*!
    @method representation
    @discussion A representation holds a type specific representation
    of the datasource's data.  The representation class is determined by mapping
    a MIME type to a class.  The representation is created once the MIME type
    of the datasource content has been determined.
    @result Returns the representation associated with this datasource.
    Returns nil if the datasource hasn't created it's representation.
*/
- (id <WebDocumentRepresentation>)representation;

/*!
    @method webFrame
    @result Return the frame that represents this data source.
*/
- (WebFrame *)webFrame;

/*!
    @method initialRequest
    @result Returns a reference to the original request that created the
    datasource.  This request will be unmodified by WebKit. 
*/
- (WebRequest *)initialRequest;

/*!
    @method request
    @result Returns the request that was used to create this datasource.
*/
-(WebRequest *)request;

/*!
    @method response
    @result returns the WebResourceResponse for the data source.
*/
- (WebResponse *)response;

/*!
    @method URL
    @discussion The value of URL will change if a redirect occurs.
    To monitor change in the URL, override the WebLocationChangeDelegate 
    serverRedirectedForDataSource: method.
    @result Returns the current URL associated with the datasource.
*/
- (NSURL *)URL;

/*!
    @method startLoading
    @discussion Start actually getting and parsing data. If the data source
    is still performing a previous load it will be stopped.
*/
- (void)startLoading;

/*!
    @method stopLoading
    @discussion Cancels any pending loads.  A data source is conceptually only ever loading
    one document at a time, although one document may have many related
    resources.  stopLoading will stop all loads related to the data source.
*/
- (void)stopLoading;

/*!
    @method isLoading
    @discussion Returns YES if there are any pending loads.
*/
- (BOOL)isLoading;

/*!
    @method isDocumentHTML
    @result Returns YES if the representation of the datasource is a WebHTMLRepresentation.
*/
- (BOOL)isDocumentHTML;

/*!
    @method pageTitle
    @result Returns nil or the page title.
    // FIXME move to WebHTMLRepresentation
*/
- (NSString *)pageTitle;

/*!
    @method fileExtension
    @result The extension based on the MIME type 
*/
- (NSString *)fileExtension;

/*!
    @method mainDocumentError
    @result Returns a WebError associated with the load of the main document, or nil if no error occurred.
*/
- (WebError *)mainDocumentError;

/*!
    @method stringWithData:
    @result A string decoded using the determined encoding.
    @discussion The overidden encoding is used if one is present. If no overidden encoding is specified,
    the server specified encoding is used. If no server specified encoding is specified,
    kCFStringEncodingISOLatin1 is used.
*/
- (NSString *)stringWithData:(NSData *)data;


/*!
    @method isDownloading
    @result Description forthcoming.
*/
- (BOOL)isDownloading;

/*!
    @method downloadPath
    @result Description forthcoming.
*/
- (NSString *)downloadPath;


/*!
    @method registerRepresentationClass:forMIMEType:
    @discussion A class that implements WebDocumentRepresentation may be registered 
    with this method.
    A document class may register for a primary MIME type by excluding
    a subtype, i.e. "video/" will match the document class with
    all video types.  More specific matching takes precedence
    over general matching.
    @param repClass The WebDocumentRepresentation class to use to represent data of the given MIME type.
    @param MIMEType The MIME type to represent with an object of the given class.
*/
+ (void) registerRepresentationClass:(Class)repClass forMIMEType:(NSString *)MIMEType;

@end
