//
//  WebIconDatabase.m
//  WebKit
//
//  Created by Chris Blumenberg on Tue Aug 27 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import <WebKit/WebKitDebug.h>
#import <WebKit/WebIconDatabase.h>
#import <WebKit/WebIconDatabasePrivate.h>

#import <WebFoundation/WebNSURLExtras.h>
#import <WebFoundation/WebFileDatabase.h>

#define WebIconDatabaseDefaultDirectory ([NSString stringWithFormat:@"%@/%@", NSHomeDirectory(), @"Library/Caches/com.apple.WebKit/Icons"])

#define WebIconsOnDiskKey @"WebIconsOnDisk"
#define WebSiteURLToIconURLKey 	@"WebSiteURLToIconURLKey"
#define WebIconURLToSiteURLsKey @"WebIconURLToSiteURLs"
#define WebHostToSiteURLsKey @"WebHostToSiteURLs"

NSString *WebIconDidChangeNotification = @"WebIconDidChangeNotification";
NSString *WebIconNotificationUserInfoSiteURLKey = @"WebIconNotificationUserInfoSiteURLKey";

NSSize WebIconSmallSize = {16, 16};
NSSize WebIconMediumSize = {32, 32};


@interface WebIconDatabase (WebInternal)

- (void)_createFileDatabase;
- (void)_loadIconDictionaries;
- (void)_updateFileDatabase;
- (NSMutableArray *)_iconsForIconURL:(NSURL *)iconURL;
- (NSImage *)_iconForFileURL:(NSURL *)fileURL withSize:(NSSize)size;
- (void)_retainIconForIconURL:(NSURL *)iconURL;
- (void)_releaseIconForIconURL:(NSURL *)iconURL;
- (void)_retainFutureIconForSiteURL:(NSURL *)siteURL;
- (void)_releaseFutureIconForSiteURL:(NSURL *)siteURL;
- (void)_retainOriginalIconsOnDisk;
- (void)_releaseOriginalIconsOnDisk;
- (void)_sendNotificationForSiteURL:(NSURL *)siteURL;
- (void)_addObject:(id)object toSetForKey:(id)key inDictionary:(NSMutableDictionary *)dictionary;
- (NSURL *)_uniqueIconURL;
- (NSImage *)_cachedIconFromArray:(NSMutableArray *)icons withSize:(NSSize)size;
- (void)_scaleIcon:(NSImage *)icon toSize:(NSSize)size;

@end


@implementation WebIconDatabase

+ (WebIconDatabase *)sharedIconDatabase
{
    static WebIconDatabase *database = nil;
    
    if (!database) {
        database = [[WebIconDatabase alloc] init];
    }
    return database;
}

- init
{
    [super init];
    
    _private = [[WebIconDatabasePrivate alloc] init];
    
    [self _createFileDatabase];
    [self _loadIconDictionaries];

    _private->iconURLToIcons = [[NSMutableDictionary dictionary] retain];
    _private->iconURLToRetainCount = [[NSMutableDictionary dictionary] retain];
    _private->futureSiteURLToRetainCount = [[NSMutableDictionary dictionary] retain];
    _private->hostToBuiltItIcons = [[NSMutableDictionary dictionary] retain];

    _private->iconsToEraseWithURLs = [[NSMutableSet set] retain];
    _private->iconsToSaveWithURLs = [[NSMutableSet set] retain];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillTerminate:)
                                                 name:NSApplicationWillTerminateNotification
                                               object:NSApp];

    // Retain icons on disk then release them once clean-up has begun.
    // This gives the client the opportunity to retain them before they are erased.
    [self _retainOriginalIconsOnDisk];
    [self performSelector:@selector(_releaseOriginalIconsOnDisk) withObject:nil afterDelay:0];
    
    return self;
}

- (NSImage *)iconForSiteURL:(NSURL *)siteURL withSize:(NSSize)size
{
    if([siteURL isFileURL]){
        return [self _iconForFileURL:siteURL withSize:size];
    }
    
    NSMutableArray *icons = [_private->hostToBuiltItIcons objectForKey:[siteURL host]];

    if(!icons){
        NSURL *iconURL = [_private->siteURLToIconURL objectForKey:siteURL];
        
        if(!iconURL){
            // Don't have it
            //NSLog(@"iconForSiteURL no iconURL for siteURL: %@", siteURL);
            return nil;
        }

        icons = [self _iconsForIconURL:iconURL];
        if(!icons){
            // This should not happen
            //NSLog(@"iconForSiteURL no icon for iconURL: %@", iconURL);
            return nil;
        }        
    }

    if(size.width == 0 && size.height == 0){
        // Don't cause a resize, just send the original in this case
        return [icons objectAtIndex:0];
    }else{
        return [self _cachedIconFromArray:icons withSize:size];
    }
}

- (void)setIcon:(NSImage *)icon forSiteURL:(NSURL *)siteURL
{
    if(!icon || !siteURL){
        return;
    }

    NSURL *iconURL = [self _uniqueIconURL];
    
    [self _setIcon:icon forIconURL:iconURL];
    [self _setIconURL:iconURL forSiteURL:siteURL];
}

- (void)setIcon:(NSImage *)icon forHost:(NSString *)host
{
    if(!icon || !host){
        return;
    }

    NSMutableSet *siteURLs = [_private->hostToSiteURLs objectForKey:host];
    NSURL *iconURL = [self _uniqueIconURL];
    NSEnumerator *enumerator;
    NSURL *siteURL;
    
    [self _setIcon:icon forIconURL:iconURL];

    if(siteURLs){
        enumerator = [siteURLs objectEnumerator];
    
        while ((siteURL = [enumerator nextObject]) != nil) {
            [self releaseIconForSiteURL:siteURL];
            [self _setIconURL:iconURL forSiteURL:siteURL];
            [self retainIconForSiteURL:siteURL];
        }
    }
}

- (void)retainIconForSiteURL:(NSURL *)siteURL
{
    NSURL *iconURL = [_private->siteURLToIconURL objectForKey:siteURL];
    
    if(iconURL){
        [self _retainIconForIconURL:iconURL];
    }else{
        // Retaining an icon not in the DB. This is OK. Just remember this.
        [self _retainFutureIconForSiteURL:siteURL];
    }
}

- (void)releaseIconForSiteURL:(NSURL *)siteURL
{
    NSURL *iconURL = [_private->siteURLToIconURL objectForKey:siteURL];
    
    if(iconURL){
        [self _releaseIconForIconURL:iconURL];
    }else{
        // Releasing an icon not in the DB. This is OK. Just remember this.
        [self _releaseFutureIconForSiteURL:siteURL];        
    }
}

- (void)delayDatabaseCleanup
{
    if(_private->didCleanup){
        [NSException raise:NSGenericException format:@"delayDatabaseCleanup cannot be called after cleanup has begun"];
    }
    
    _private->cleanupCount++;
    //NSLog(@"delayDatabaseCleanup %d", _private->cleanupCount);
}

- (void)allowDatabaseCleanup
{
    if(_private->didCleanup){
        [NSException raise:NSGenericException format:@"allowDatabaseCleanup cannot be called after cleanup has begun"];
    }
    
    _private->cleanupCount--;
    //NSLog(@"allowDatabaseCleanup %d", _private->cleanupCount);

    if(_private->cleanupCount == 0 && _private->waitingToCleanup){
        [self _releaseOriginalIconsOnDisk];
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    // Should only cause a write if user quit before 3 seconds after the last _updateFileDatabase
    [_private->fileDatabase sync];
}

@end


@implementation WebIconDatabase (WebPrivate)

- (void)_createFileDatabase
{
    // FIXME: Make defaults key public somehow
    NSString *databaseDirectory = [[NSUserDefaults standardUserDefaults] objectForKey:@"WebIconDatabaseDirectory"];

    if (!databaseDirectory) {
        databaseDirectory = WebIconDatabaseDefaultDirectory;
    }

    _private->fileDatabase = [[WebFileDatabase alloc] initWithPath:databaseDirectory];
    [_private->fileDatabase setSizeLimit:20000000];
    [_private->fileDatabase open];
}

- (void)_loadIconDictionaries
{
    WebFileDatabase *fileDB = _private->fileDatabase;

    _private->iconsOnDiskWithURLs = [fileDB objectForKey:WebIconsOnDiskKey];
    _private->siteURLToIconURL = [fileDB objectForKey:WebSiteURLToIconURLKey];
    _private->iconURLToSiteURLs = [fileDB objectForKey:WebIconURLToSiteURLsKey];
    _private->hostToSiteURLs = [fileDB objectForKey:WebHostToSiteURLsKey];

    if(!_private->iconsOnDiskWithURLs ||
       !_private->siteURLToIconURL ||
       !_private->iconURLToSiteURLs ||
       !_private->hostToSiteURLs){

        _private->iconsOnDiskWithURLs = [NSMutableSet set];
        _private->siteURLToIconURL = [NSMutableDictionary dictionary];
        _private->iconURLToSiteURLs = [NSMutableDictionary dictionary];
        _private->hostToSiteURLs = [NSMutableDictionary dictionary];
    }

    [_private->iconsOnDiskWithURLs retain];
    [_private->iconURLToSiteURLs retain];
    [_private->siteURLToIconURL retain];
    [_private->hostToSiteURLs retain];
}

// Only called by _setIconURL:forKey:
- (void)_updateFileDatabase
{
    if(_private->cleanupCount != 0){
        return;
    }

    //NSLog(@"_updateFileDatabase");

    WebFileDatabase *fileDB = _private->fileDatabase;

    NSEnumerator *enumerator;
    NSData *iconData;
    NSURL *iconURL;

    // Erase icons that have been released that are on disk
    enumerator = [_private->iconsToEraseWithURLs objectEnumerator];

    while ((iconURL = [enumerator nextObject]) != nil) {
        //NSLog(@"removing %@", iconURL);
        [fileDB removeObjectForKey:iconURL];
        [_private->iconsOnDiskWithURLs removeObject:iconURL];
    }

    // Save icons that have been retained that are not already on disk
    enumerator = [_private->iconsToSaveWithURLs objectEnumerator];

    while ((iconURL = [enumerator nextObject]) != nil) {
        //NSLog(@"writing %@", iconURL);
        iconData = [[self _iconForIconURL:iconURL] TIFFRepresentation];
        [fileDB setObject:iconData forKey:iconURL];
        [_private->iconsOnDiskWithURLs addObject:iconURL];
    }
    
    [_private->iconsToEraseWithURLs removeAllObjects];
    [_private->iconsToSaveWithURLs removeAllObjects];

    // Save the icon dictionaries to disk
    [fileDB setObject:_private->iconsOnDiskWithURLs forKey:WebIconsOnDiskKey];
    [fileDB setObject:_private->siteURLToIconURL forKey:WebSiteURLToIconURLKey];
    [fileDB setObject:_private->iconURLToSiteURLs forKey:WebIconURLToSiteURLsKey];
    [fileDB setObject:_private->hostToSiteURLs forKey:WebHostToSiteURLsKey];
}

- (NSImage *)_iconForIconURL:(NSURL *)iconURL
{
    // The first item in the icon array is the original non-resized icon
    return [[self _iconsForIconURL:iconURL] objectAtIndex:0];
}

- (NSMutableArray *)_iconsForIconURL:(NSURL *)iconURL
{
    if(!iconURL){
        return nil;
    }

    NSMutableArray *icons = [_private->iconURLToIcons objectForKey:iconURL];
    double start, duration;
    
    if(!icons){
        // Not in memory, check disk
        if([_private->iconsOnDiskWithURLs containsObject:iconURL]){
            
            start = CFAbsoluteTimeGetCurrent();
            NSData *iconData = [_private->fileDatabase objectForKey:iconURL];
            
            if(iconData){
                NSImage *icon = [[NSImage alloc] initWithData:iconData];
                if(icon){
                    duration = CFAbsoluteTimeGetCurrent() - start;
                    WEBKITDEBUGLEVEL (WEBKIT_LOG_TIMING, "loading and creating icon %s took %f seconds\n",
                                      DEBUG_OBJECT(iconURL), duration);
                    
                    // Cache it
                    icons = [NSMutableArray arrayWithObject:icon];
                    [icon release];
                    [_private->iconURLToIcons setObject:icons forKey:iconURL];
                }
            }
        }
    }
    
    return icons;
}


- (NSImage *)_iconForFileURL:(NSURL *)fileURL withSize:(NSSize)size
{
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    NSImage *icon;
    
    if([[[fileURL path] pathExtension] rangeOfString:@"htm"].length > 0){
        if(!_private->htmlIcons){
            icon = [workspace iconForFileType:@"html"];
            _private->htmlIcons = [[NSMutableArray arrayWithObject:icon] retain];
        }
        return [self _cachedIconFromArray:_private->htmlIcons withSize:size];
    }else{
        icon = [workspace iconForFile:[fileURL path]];
        [self _scaleIcon:icon toSize:size];
        return icon;
    }
}

- (void)_setIcon:(NSImage *)icon forIconURL:(NSURL *)iconURL
{
    if(!icon || !iconURL){
        return;
    }

    NSMutableArray *icons = [_private->iconURLToIcons objectForKey:iconURL];
    [icons removeAllObjects];
    
    [_private->iconURLToIcons setObject:[NSMutableArray arrayWithObject:icon] forKey:iconURL];

    [self _retainIconForIconURL:iconURL];
    
    // Retain and release the newly created icon much like an autorelease.
    // This gives the client enough time to retain it.
    [self performSelector:@selector(_releaseIconForIconURL:) withObject:iconURL afterDelay:0];
}

// FIXME: fix custom icons
- (void)_setIconURL:(NSURL *)iconURL forSiteURL:(NSURL *)siteURL
{
    if(!iconURL || !siteURL){
        return;
    }

    [_private->siteURLToIconURL setObject:iconURL forKey:siteURL];
    
    [self _addObject:siteURL toSetForKey:iconURL inDictionary:_private->iconURLToSiteURLs];

    [self _addObject:siteURL toSetForKey:[siteURL host] inDictionary:_private->hostToSiteURLs];
    
    NSNumber *predeterminedRetainCount = [_private->futureSiteURLToRetainCount objectForKey:siteURL];

    if(predeterminedRetainCount){
        NSNumber *retainCount = [_private->iconURLToRetainCount objectForKey:iconURL];

        if(!retainCount){
            //NSLog(@"_setIconURL: forKey: no retainCount for iconURL");
            return;
        }

        int newRetainCount = [retainCount intValue] + [predeterminedRetainCount intValue];
        [_private->iconURLToRetainCount setObject:[NSNumber numberWithInt:newRetainCount] forKey:iconURL];
        [_private->futureSiteURLToRetainCount removeObjectForKey:siteURL];
    }

    [self _sendNotificationForSiteURL:siteURL];
    [self _updateFileDatabase];
}

- (void)_setBuiltInIcon:(NSImage *)icon forHost:(NSString *)host
{
    if(!icon || !host){
        return;
    }
    
    NSMutableSet *siteURLs = [_private->hostToSiteURLs objectForKey:host];
    NSURL *siteURL;
    
    [_private->hostToBuiltItIcons setObject:[NSMutableArray arrayWithObject:icon] forKey:host];

    if(siteURLs){
        NSEnumerator *enumerator = [siteURLs objectEnumerator];
        while ((siteURL = [enumerator nextObject]) != nil) {
            [self _sendNotificationForSiteURL:siteURL];
        }
    }
}

- (void)_retainIconForIconURL:(NSURL *)iconURL
{
    NSNumber *retainCount = [_private->iconURLToRetainCount objectForKey:iconURL];
    int newRetainCount;
    
    if(!retainCount){
        newRetainCount = 1;
    }else{
        newRetainCount = [retainCount intValue] + 1;
    }

    [_private->iconURLToRetainCount setObject:[NSNumber numberWithInt:newRetainCount] forKey:iconURL];

    if(newRetainCount == 1 && ![_private->iconsOnDiskWithURLs containsObject:iconURL]){
        [_private->iconsToSaveWithURLs addObject:iconURL];
        [_private->iconsToEraseWithURLs removeObject:iconURL];
    }

    //NSLog(@"_retainIconForIconURL: %@ %d", iconURL, newRetainCount);
}

- (void)_releaseIconForIconURL:(NSURL *)iconURL
{
    NSNumber *retainCount = [_private->iconURLToRetainCount objectForKey:iconURL];

    if(!retainCount){
        //NSLog(@"_retainIconForIconURL: no retainCount for iconURL");
        return;
    }
    
    int newRetainCount = [retainCount intValue] - 1;
    [_private->iconURLToRetainCount setObject:[NSNumber numberWithInt:newRetainCount] forKey:iconURL];

    if(newRetainCount == 0){

        if([_private->iconsOnDiskWithURLs containsObject:iconURL]){
            [_private->iconsToEraseWithURLs addObject:iconURL];
            [_private->iconsToSaveWithURLs removeObject:iconURL];
        }

        // Remove the icon's images
        NSMutableArray *icons = [_private->iconURLToIcons objectForKey:iconURL];
        [icons removeAllObjects];
        [_private->iconURLToIcons removeObjectForKey:iconURL];

        // Remove the icon's retain count
        [_private->iconURLToRetainCount removeObjectForKey:iconURL];

        // Remove the icon's associated site URLs
        NSMutableSet *siteURLsForHost, *siteURLs;
        NSEnumerator *enumerator;
        NSURL *siteURL;
        
        siteURLs = [_private->iconURLToSiteURLs objectForKey:iconURL];
        [_private->siteURLToIconURL removeObjectsForKeys:[siteURLs allObjects]];

        enumerator = [siteURLs objectEnumerator];
        while ((siteURL = [enumerator nextObject]) != nil) {
            siteURLsForHost = [_private->hostToSiteURLs objectForKey:[siteURL host]];
            [siteURLsForHost removeObject:siteURL];
            if([siteURLsForHost count] == 0){
                [_private->hostToSiteURLs removeObjectForKey:[siteURL host]];
            }
        }
        
        [siteURLs removeAllObjects];
        [_private->iconURLToSiteURLs removeObjectForKey:iconURL];
    }

    //NSLog(@"_releaseIconForIconURL: %@ %d", iconURL, newRetainCount);
}

- (void)_retainFutureIconForSiteURL:(NSURL *)siteURL
{
    NSNumber *retainCount = [_private->futureSiteURLToRetainCount objectForKey:siteURL];
    int newRetainCount;

    if(retainCount){
        newRetainCount = [retainCount intValue] + 1;
    }else{
        newRetainCount = 1;
    }

    [_private->futureSiteURLToRetainCount setObject:[NSNumber numberWithInt:newRetainCount] forKey:siteURL];

    ////NSLog(@"_setFutureIconRetainToDictionary: %@ %d", key, newRetainCount);
}

- (void)_releaseFutureIconForSiteURL:(NSURL *)siteURL
{
    NSNumber *retainCount = [_private->futureSiteURLToRetainCount objectForKey:siteURL];

    if(!retainCount){
        [NSException raise:NSGenericException
                    format:@"Releasing a future icon that was not previously retained."];
    }

    int newRetainCount = [retainCount intValue] - 1;

    if(newRetainCount == 0){
        [_private->futureSiteURLToRetainCount removeObjectForKey:siteURL];
    }else{
        [_private->futureSiteURLToRetainCount setObject:[NSNumber numberWithInt:newRetainCount] forKey:siteURL];
    }

    ////NSLog(@"_setFutureIconReleaseToDictionary: %@ %d", key, newRetainCount);
}

- (void)_retainOriginalIconsOnDisk
{
    NSEnumerator *enumerator;
    NSURL *iconURL;

    //NSLog(@"_retainOriginalOnDiskIcons");
    enumerator = [_private->iconsOnDiskWithURLs objectEnumerator];

    while ((iconURL = [enumerator nextObject]) != nil) {
        [self _retainIconForIconURL:iconURL];
    }
}

- (void)_releaseOriginalIconsOnDisk
{
    if(_private->cleanupCount > 0){
        _private->waitingToCleanup = YES;
        return;
    }
    //NSLog(@"_releaseOriginalOnDiskIcons, %@", _private->iconsOnDiskWithURLs);
    NSEnumerator *enumerator = [_private->iconsOnDiskWithURLs objectEnumerator];
    NSURL *iconURL;

    while ((iconURL = [enumerator nextObject]) != nil) {
        [self _releaseIconForIconURL:iconURL];
    }

    _private->didCleanup = YES;
}

- (void)_sendNotificationForSiteURL:(NSURL *)siteURL
{
    NSDictionary *userInfo = [NSDictionary dictionaryWithObject:siteURL forKey:WebIconNotificationUserInfoSiteURLKey];
    [[NSNotificationCenter defaultCenter] postNotificationName:WebIconDidChangeNotification
                                                        object:self
                                                      userInfo:userInfo];
}

- (void)_addObject:(id)object toSetForKey:(id)key inDictionary:(NSMutableDictionary *)dictionary
{
    NSMutableSet *set = [dictionary objectForKey:key];

    if(!set){
        set = [NSMutableSet set];
    }
        
    [set addObject:object];
    [dictionary setObject:set forKey:key];
}

- (NSURL *)_uniqueIconURL
{
    CFUUIDRef uid = CFUUIDCreate(NULL);
    NSString *string = (NSString *)CFUUIDCreateString(NULL, uid);
    NSURL *uniqueURL = [NSURL _web_URLWithString:[NSString stringWithFormat:@"icon:%@", string]];

    CFRelease(uid);
    CFRelease(string);

    return uniqueURL;
}

- (NSImage *)_cachedIconFromArray:(NSMutableArray *)icons withSize:(NSSize)size
{
    NSEnumerator *enumerator = [icons objectEnumerator];
    NSImage *icon;
    
    while ((icon = [enumerator nextObject]) != nil) {
        if(NSEqualSizes([icon size], size)){
            return icon;
        }
    }

    // The first item in the icon array is the original non-resized icon
    // Assume that it's best to resize the original
    NSImage *originalIcon = [icons objectAtIndex:0];
    icon = [originalIcon copy];
    [self _scaleIcon:icon toSize:size];

    // Cache it
    [icons addObject:icon];

    return [icon autorelease];
}

- (void)_scaleIcon:(NSImage *)icon toSize:(NSSize)size
{
    [icon setScalesWhenResized:YES];
    [icon setSize:size];
}

@end

@implementation WebIconDatabasePrivate

@end
