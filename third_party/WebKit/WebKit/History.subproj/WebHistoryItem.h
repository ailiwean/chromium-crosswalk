/*	
    WebHistoryItem.h
    Copyright (C) 2003 Apple Computer, Inc. All rights reserved.    

    Public header file.
 */

#import <Cocoa/Cocoa.h>

@class WebHistoryItemPrivate;
@class NSURL;

/*!
    @class WebHistoryItem
    @discussion  WebHistoryItems are created by WebKit to represent pages visited.
    The WebBackForwardList and WebHistory classes both use WebHistoryItems to represent
    pages visited.  With the exception of the displayTitle, the properties of 
    WebHistoryItems are set by WebKit.  WebHistoryItems are normally never created directly.
*/
@interface WebHistoryItem : NSObject <NSCopying>
{
@private
    WebHistoryItemPrivate *_private;
}

/*!
    @method initWithURLString:title:lastVisitedTimeInterval:
    @param URLString The URL string for the item.
    @param title The title to use for the item.  This is normally the <title> of a page.
    @param time The time used to indicate when the item was used.
    @abstract Initialize a new WebHistoryItem
    @discussion WebHistoryItems are normally created for you by the WebKit.
    You may use this method to prepopulate a WebBackForwardList, or create
    'artificial' items to add to a WebBackForwardList.  When first initialized
    the URLString and originalURLString will be the same.
*/
- (id)initWithURLString:(NSString *)URLString title:(NSString *)title lastVisitedTimeInterval:(NSTimeInterval)time;

/*!
    @method originalURLString
    @abstract The string representation of the originial URL of this item.
    This value is normally set by the WebKit.
    @result The string corresponding to the initial URL of this item.
*/
- (NSString *)originalURLString;

/*!
    @method URLString
    @abstract The string representation of the URL represented by this item.
    @discussion The URLString may be different than the originalURLString if the page
    redirected to a new location.  This value is normally set by the WebKit.
    @result The string corresponding to the final URL of this item.
*/
- (NSString *)URLString;


/*!
    @method title
    @abstract The title of the page represented by this item.
    @discussion This title cannot be changed by the client.  This value
    is normally set by the WebKit when a page title for the item is received.
    @result The title of this item.
*/
- (NSString *)title;

/*!
    @method lastVisitedTimeInterval
    @abstract The last time the page represented by this item was visited. The interval
    is since the reference date as determined by NSDate.  This value is normally set by
    the WebKit.
    @result The last time this item was visited.
*/
- (NSTimeInterval)lastVisitedTimeInterval;

/*!
    @method setAlternateTitle:
    @param alternateTitle The new display title for this item.
    @abstract A title that may be used by the client to display this item.
*/
- (void)setAlternateTitle:(NSString *)alternateTitle;

/*
    @method title
    @abstract A title that may be used by the client to display this item.
    @result The alternate title for this item.
*/
- (NSString *)alternateTitle;

/*!
    @method icon
    @abstract The favorite icon of the page represented by this item.
    @discussion This icon returned will be determined by the WebKit.
    @result The icon associated with this item's URL.
*/
- (NSImage *)icon;

@end
