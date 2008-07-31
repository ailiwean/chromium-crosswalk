/*
 * Copyright (C) 2005, 2006, 2007 Apple Inc. All rights reserved.
 *           (C) 2007 Graham Dennis (graham.dennis@gmail.com)
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
 
#import "DumpRenderTree.h"

#import "AccessibilityController.h"
#import "CheckedMalloc.h"
#import "DumpRenderTreePasteboard.h"
#import "DumpRenderTreeWindow.h"
#import "EditingDelegate.h"
#import "EventSendingController.h"
#import "FrameLoadDelegate.h"
#import "JavaScriptThreading.h"
#import "LayoutTestController.h"
#import "NavigationController.h"
#import "ObjCPlugin.h"
#import "ObjCPluginFunction.h"
#import "PixelDumpSupport.h"
#import "PolicyDelegate.h"
#import "ResourceLoadDelegate.h"
#import "UIDelegate.h"
#import "WorkQueue.h"
#import "WorkQueueItem.h"
#import <CoreFoundation/CoreFoundation.h>
#import <WebKit/DOMElementPrivate.h>
#import <WebKit/DOMExtensions.h>
#import <WebKit/DOMRange.h>
#import <WebKit/WebBackForwardList.h>
#import <WebKit/WebCache.h>
#import <WebKit/WebCoreStatistics.h>
#import <WebKit/WebDataSourcePrivate.h>
#import <WebKit/WebDatabaseManagerPrivate.h>
#import <WebKit/WebDocumentPrivate.h>
#import <WebKit/WebEditingDelegate.h>
#import <WebKit/WebFrameView.h>
#import <WebKit/WebHTMLRepresentationInternal.h>
#import <WebKit/WebHistory.h>
#import <WebKit/WebHistoryItemPrivate.h>
#import <WebKit/WebPluginDatabase.h>
#import <WebKit/WebPreferences.h>
#import <WebKit/WebPreferencesPrivate.h>
#import <WebKit/WebResourceLoadDelegate.h>
#import <WebKit/WebTypesInternal.h>
#import <WebKit/WebViewPrivate.h>
#import <getopt.h>
#import <mach-o/getsect.h>
#import <objc/objc-runtime.h>
#import <wtf/Assertions.h>
#import <wtf/RetainPtr.h>
#import <wtf/OwnPtr.h>

@interface DumpRenderTreeEvent : NSEvent
@end

static void runTest(const char *pathOrURL);

// Deciding when it's OK to dump out the state is a bit tricky.  All these must be true:
// - There is no load in progress
// - There is no work queued up (see workQueue var, below)
// - waitToDump==NO.  This means either waitUntilDone was never called, or it was called
//       and notifyDone was called subsequently.
// Note that the call to notifyDone and the end of the load can happen in either order.

volatile bool done;

NavigationController* navigationController = 0;
LayoutTestController* layoutTestController = 0;

WebFrame *mainFrame = 0;
// This is the topmost frame that is loading, during a given load, or nil when no load is 
// in progress.  Usually this is the same as the main frame, but not always.  In the case
// where a frameset is loaded, and then new content is loaded into one of the child frames,
// that child frame is the "topmost frame that is loading".
WebFrame *topLoadingFrame = nil;     // !nil iff a load is in progress


CFMutableSetRef disallowedURLs = 0;
CFRunLoopTimerRef waitToDumpWatchdog = 0;

// Delegates
static FrameLoadDelegate *frameLoadDelegate;
static UIDelegate *uiDelegate;
static EditingDelegate *editingDelegate;
static ResourceLoadDelegate *resourceLoadDelegate;
PolicyDelegate *policyDelegate;

static int dumpPixels;
static int dumpAllPixels;
static int threaded;
static int testRepaintDefault;
static int repaintSweepHorizontallyDefault;
static int dumpTree = YES;
static BOOL printSeparators;
static NSString *currentTest = nil;
static RetainPtr<CFStringRef> persistentUserStyleSheetLocation;

static WebHistoryItem *prevTestBFItem = nil;  // current b/f item at the end of the previous test

const unsigned maxViewHeight = 600;
const unsigned maxViewWidth = 800;

#if __OBJC2__
static void swizzleAllMethods(Class imposter, Class original)
{
    unsigned int imposterMethodCount;
    Method* imposterMethods = class_copyMethodList(imposter, &imposterMethodCount);

    unsigned int originalMethodCount;
    Method* originalMethods = class_copyMethodList(original, &originalMethodCount);

    for (unsigned int i = 0; i < imposterMethodCount; i++) {
        SEL imposterMethodName = method_getName(imposterMethods[i]);

        // Attempt to add the method to the original class.  If it fails, the method already exists and we should
        // instead exchange the implementations.
        if (class_addMethod(original, imposterMethodName, method_getImplementation(originalMethods[i]), method_getTypeEncoding(originalMethods[i])))
            continue;

        unsigned int j = 0;
        for (; j < originalMethodCount; j++) {
            SEL originalMethodName = method_getName(originalMethods[j]);
            if (sel_isEqual(imposterMethodName, originalMethodName))
                break;
        }

        // If class_addMethod failed above then the method must exist on the original class.
        ASSERT(j < originalMethodCount);
        method_exchangeImplementations(imposterMethods[i], originalMethods[j]);
    }

    free(imposterMethods);
    free(originalMethods);
}
#endif

static void poseAsClass(const char* imposter, const char* original)
{
    Class imposterClass = objc_getClass(imposter);
    Class originalClass = objc_getClass(original);

#if !__OBJC2__
    class_poseAs(imposterClass, originalClass);
#else

    // Swizzle instance methods
    swizzleAllMethods(imposterClass, originalClass);
    // and then class methods
    swizzleAllMethods(object_getClass(imposterClass), object_getClass(originalClass));
#endif
}

void setPersistentUserStyleSheetLocation(CFStringRef url)
{
    persistentUserStyleSheetLocation = url;
}

static BOOL shouldIgnoreWebCoreNodeLeaks(CFStringRef URLString)
{
    static CFStringRef const ignoreSet[] = {
        // Keeping this infrastructure around in case we ever need it again.
    };
    static const int ignoreSetCount = sizeof(ignoreSet) / sizeof(CFStringRef);
    
    for (int i = 0; i < ignoreSetCount; i++) {
        CFStringRef ignoreString = ignoreSet[i];
        CFRange range = CFRangeMake(0, CFStringGetLength(URLString));
        CFOptionFlags flags = kCFCompareAnchored | kCFCompareBackwards | kCFCompareCaseInsensitive;
        if (CFStringFindWithOptions(URLString, ignoreString, range, flags, NULL))
            return YES;
    }
    return NO;
}

static void activateFonts()
{
    static const char* fontSectionNames[] = {
        "Ahem",
        "WeightWatcher100",
        "WeightWatcher200",
        "WeightWatcher300",
        "WeightWatcher400",
        "WeightWatcher500",
        "WeightWatcher600",
        "WeightWatcher700",
        "WeightWatcher800",
        "WeightWatcher900",
        0
    };

    for (unsigned i = 0; fontSectionNames[i]; ++i) {
        unsigned long fontDataLength;
        char* fontData = getsectdata("__DATA", fontSectionNames[i], &fontDataLength);
        if (!fontData) {
            fprintf(stderr, "Failed to locate the %s font.\n", fontSectionNames[i]);
            exit(1);
        }

        ATSFontContainerRef fontContainer;
        OSStatus status = ATSFontActivateFromMemory(fontData, fontDataLength, kATSFontContextLocal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, &fontContainer);

        if (status != noErr) {
            fprintf(stderr, "Failed to activate the %s font.\n", fontSectionNames[i]);
            exit(1);
        }
    }
}

WebView *createWebViewAndOffscreenWindow()
{
    NSRect rect = NSMakeRect(0, 0, maxViewWidth, maxViewHeight);
    WebView *webView = [[WebView alloc] initWithFrame:rect frameName:nil groupName:@"org.webkit.DumpRenderTree"];
        
    [webView setUIDelegate:uiDelegate];
    [webView setFrameLoadDelegate:frameLoadDelegate];
    [webView setEditingDelegate:editingDelegate];
    [webView setResourceLoadDelegate:resourceLoadDelegate];

    // Register the same schemes that Safari does
    [WebView registerURLSchemeAsLocal:@"feed"];
    [WebView registerURLSchemeAsLocal:@"feeds"];
    [WebView registerURLSchemeAsLocal:@"feedsearch"];
    
    [webView setContinuousSpellCheckingEnabled:YES];
    
    // To make things like certain NSViews, dragging, and plug-ins work, put the WebView a window, but put it off-screen so you don't see it.
    // Put it at -10000, -10000 in "flipped coordinates", since WebCore and the DOM use flipped coordinates.
    NSRect windowRect = NSOffsetRect(rect, -10000, [[[NSScreen screens] objectAtIndex:0] frame].size.height - rect.size.height + 10000);
    DumpRenderTreeWindow *window = [[DumpRenderTreeWindow alloc] initWithContentRect:windowRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];
    [[window contentView] addSubview:webView];
    [window orderBack:nil];
    [window setAutodisplay:NO];
    
    // For reasons that are not entirely clear, the following pair of calls makes WebView handle its
    // dynamic scrollbars properly. Without it, every frame will always have scrollbars.
    NSBitmapImageRep *imageRep = [webView bitmapImageRepForCachingDisplayInRect:[webView bounds]];
    [webView cacheDisplayInRect:[webView bounds] toBitmapImageRep:imageRep];
        
    return webView;
}

void testStringByEvaluatingJavaScriptFromString()
{
    // maps expected result <= JavaScript expression
    NSDictionary *expressions = [NSDictionary dictionaryWithObjectsAndKeys:
        @"0", @"0", 
        @"0", @"'0'", 
        @"", @"",
        @"", @"''", 
        @"", @"new String()", 
        @"", @"new String('0')", 
        @"", @"throw 1", 
        @"", @"{ }", 
        @"", @"[ ]", 
        @"", @"//", 
        @"", @"a.b.c", 
        @"", @"(function() { throw 'error'; })()", 
        @"", @"null",
        @"", @"undefined",
        @"true", @"true",
        @"false", @"false",
        nil
    ];

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    WebView *webView = [[WebView alloc] initWithFrame:NSZeroRect frameName:@"" groupName:@""];

    NSEnumerator *enumerator = [expressions keyEnumerator];
    id expression;
    while ((expression = [enumerator nextObject])) {
        NSString *expectedResult = [expressions objectForKey:expression];
        NSString *result = [webView stringByEvaluatingJavaScriptFromString:expression];
        assert([result isEqualToString:expectedResult]);
    }

    [webView close];
    [webView release];
    [pool release];
}

static void setDefaultsToConsistentValuesForTesting()
{
    // Give some clear to undocumented defaults values
    static const int MediumFontSmoothing = 2;
    static const int BlueTintedAppearance = 1;

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:@"DoubleMax" forKey:@"AppleScrollBarVariant"];
    [defaults setInteger:4 forKey:@"AppleAntiAliasingThreshold"]; // smallest font size to CG should perform antialiasing on
    [defaults setInteger:MediumFontSmoothing forKey:@"AppleFontSmoothing"];
    [defaults setInteger:BlueTintedAppearance forKey:@"AppleAquaColorVariant"];
    [defaults setObject:@"0.709800 0.835300 1.000000" forKey:@"AppleHighlightColor"];
    [defaults setObject:@"0.500000 0.500000 0.500000" forKey:@"AppleOtherHighlightColor"];
    [defaults setObject:[NSArray arrayWithObject:@"en"] forKey:@"AppleLanguages"];

    NSString *libraryPath = [@"~/Library/Application Support/DumpRenderTree" stringByExpandingTildeInPath];
    [defaults setObject:[libraryPath stringByAppendingPathComponent:@"Databases"] forKey:WebDatabaseDirectoryDefaultsKey];
    
    WebPreferences *preferences = [WebPreferences standardPreferences];

    [preferences setStandardFontFamily:@"Times"];
    [preferences setFixedFontFamily:@"Courier"];
    [preferences setSerifFontFamily:@"Times"];
    [preferences setSansSerifFontFamily:@"Helvetica"];
    [preferences setCursiveFontFamily:@"Apple Chancery"];
    [preferences setFantasyFontFamily:@"Papyrus"];
    [preferences setDefaultFontSize:16];
    [preferences setDefaultFixedFontSize:13];
    [preferences setMinimumFontSize:1];
    [preferences setJavaEnabled:NO];
    [preferences setEditableLinkBehavior:WebKitEditableLinkOnlyLiveWithShiftKey];
    [preferences setTabsToLinks:NO];
    [preferences setDOMPasteAllowed:YES];
    [preferences setFullDocumentTeardownEnabled:YES];
    [preferences setShouldPrintBackgrounds:YES];

    // The back/forward cache is causing problems due to layouts during transition from one page to another.
    // So, turn it off for now, but we might want to turn it back on some day.
    [preferences setUsesPageCache:NO];
}

static void crashHandler(int sig)
{
    char *signalName = strsignal(sig);
    write(STDERR_FILENO, signalName, strlen(signalName));
    write(STDERR_FILENO, "\n", 1);
    restoreColorSpace(0);
    exit(128 + sig);
}

static void installSignalHandlers()
{
    signal(SIGILL, crashHandler);    /* 4:   illegal instruction (not reset when caught) */
    signal(SIGTRAP, crashHandler);   /* 5:   trace trap (not reset when caught) */
    signal(SIGEMT, crashHandler);    /* 7:   EMT instruction */
    signal(SIGFPE, crashHandler);    /* 8:   floating point exception */
    signal(SIGBUS, crashHandler);    /* 10:  bus error */
    signal(SIGSEGV, crashHandler);   /* 11:  segmentation violation */
    signal(SIGSYS, crashHandler);    /* 12:  bad argument to system call */
    signal(SIGPIPE, crashHandler);   /* 13:  write on a pipe with no reader */
    signal(SIGXCPU, crashHandler);   /* 24:  exceeded CPU time limit */
    signal(SIGXFSZ, crashHandler);   /* 25:  exceeded file size limit */
}

static void allocateGlobalControllers()
{
    // FIXME: We should remove these and move to the ObjC standard [Foo sharedInstance] model
    navigationController = [[NavigationController alloc] init];
    frameLoadDelegate = [[FrameLoadDelegate alloc] init];
    uiDelegate = [[UIDelegate alloc] init];
    editingDelegate = [[EditingDelegate alloc] init];
    resourceLoadDelegate = [[ResourceLoadDelegate alloc] init];
    policyDelegate = [[PolicyDelegate alloc] init];
}

// ObjC++ doens't seem to let me pass NSObject*& sadly.
static inline void releaseAndZero(NSObject** object)
{
    [*object release];
    *object = nil;
}

static void releaseGlobalControllers()
{
    releaseAndZero(&navigationController);
    releaseAndZero(&frameLoadDelegate);
    releaseAndZero(&editingDelegate);
    releaseAndZero(&resourceLoadDelegate);
    releaseAndZero(&uiDelegate);
    releaseAndZero(&policyDelegate);
}

static void initializeGlobalsFromCommandLineOptions(int argc, const char *argv[])
{
    struct option options[] = {
        {"dump-all-pixels", no_argument, &dumpAllPixels, YES},
        {"horizontal-sweep", no_argument, &repaintSweepHorizontallyDefault, YES},
        {"notree", no_argument, &dumpTree, NO},
        {"pixel-tests", no_argument, &dumpPixels, YES},
        {"repaint", no_argument, &testRepaintDefault, YES},
        {"tree", no_argument, &dumpTree, YES},
        {"threaded", no_argument, &threaded, YES},
        {NULL, 0, NULL, 0}
    };
    
    int option;
    while ((option = getopt_long(argc, (char * const *)argv, "", options, NULL)) != -1) {
        switch (option) {
            case '?':   // unknown or ambiguous option
            case ':':   // missing argument
                exit(1);
                break;
        }
    }
}

static void addTestPluginsToPluginSearchPath(const char* executablePath)
{
    NSString *pwd = [[NSString stringWithUTF8String:executablePath] stringByDeletingLastPathComponent];
    [WebPluginDatabase setAdditionalWebPlugInPaths:[NSArray arrayWithObject:pwd]];
    [[WebPluginDatabase sharedDatabase] refresh];
}

static bool useLongRunningServerMode(int argc, const char *argv[])
{
    // This assumes you've already called getopt_long
    return (argc == optind+1 && strcmp(argv[optind], "-") == 0);
}

static void runTestingServerLoop()
{
    // When DumpRenderTree run in server mode, we just wait around for file names
    // to be passed to us and read each in turn, passing the results back to the client
    char filenameBuffer[2048];
    while (fgets(filenameBuffer, sizeof(filenameBuffer), stdin)) {
        char *newLineCharacter = strchr(filenameBuffer, '\n');
        if (newLineCharacter)
            *newLineCharacter = '\0';

        if (strlen(filenameBuffer) == 0)
            continue;

        runTest(filenameBuffer);
    }
}

static void prepareConsistentTestingEnvironment()
{
    poseAsClass("DumpRenderTreePasteboard", "NSPasteboard");
    poseAsClass("DumpRenderTreeEvent", "NSEvent");

    setDefaultsToConsistentValuesForTesting();
    activateFonts();
    
    if (dumpPixels)
        initializeColorSpaceAndScreeBufferForPixelTests();
    allocateGlobalControllers();
    
    makeLargeMallocFailSilently();
}

void dumpRenderTree(int argc, const char *argv[])
{
    initializeGlobalsFromCommandLineOptions(argc, argv);
    prepareConsistentTestingEnvironment();
    addTestPluginsToPluginSearchPath(argv[0]);
    if (dumpPixels)
        installSignalHandlers();
    
    WebView *webView = createWebViewAndOffscreenWindow();
    mainFrame = [webView mainFrame];

    [[NSURLCache sharedURLCache] removeAllCachedResponses];

    [WebCache empty];
     
    // <rdar://problem/5222911>
    testStringByEvaluatingJavaScriptFromString();

    if (threaded)
        startJavaScriptThreads();

    if (useLongRunningServerMode(argc, argv)) {
        printSeparators = YES;
        runTestingServerLoop();
    } else {
        printSeparators = (optind < argc-1 || (dumpPixels && dumpTree));
        for (int i = optind; i != argc; ++i)
            runTest(argv[i]);
    }

    if (threaded)
        stopJavaScriptThreads();

    [WebCoreStatistics emptyCache]; // Otherwise SVGImages trigger false positives for Frame/Node counts    
    [webView close];
    mainFrame = nil;

    // Work around problem where registering drag types leaves an outstanding
    // "perform selector" on the window, which retains the window. It's a bit
    // inelegant and perhaps dangerous to just blow them all away, but in practice
    // it probably won't cause any trouble (and this is just a test tool, after all).
    NSWindow *window = [webView window];
    [NSObject cancelPreviousPerformRequestsWithTarget:window];
    
    [window close]; // releases when closed
    [webView release];
    
    releaseGlobalControllers();
    
    [DumpRenderTreePasteboard releaseLocalPasteboards];

    // FIXME: This should be moved onto LayoutTestController and made into a HashSet
    if (disallowedURLs) {
        CFRelease(disallowedURLs);
        disallowedURLs = 0;
    }

    if (dumpPixels)
        restoreColorSpace(0);
}

int main(int argc, const char *argv[])
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication]; // Force AppKit to init itself
    dumpRenderTree(argc, argv);
    [WebCoreStatistics garbageCollectJavaScriptObjects];
    [pool release];
    return 0;
}

static NSInteger compareHistoryItems(id item1, id item2, void *context)
{
    return [[item1 target] caseInsensitiveCompare:[item2 target]];
}

static void dumpHistoryItem(WebHistoryItem *item, int indent, BOOL current)
{
    int start = 0;
    if (current) {
        printf("curr->");
        start = 6;
    }
    for (int i = start; i < indent; i++)
        putchar(' ');
    printf("%s", [[item URLString] UTF8String]);
    NSString *target = [item target];
    if (target && [target length] > 0)
        printf(" (in frame \"%s\")", [target UTF8String]);
    if ([item isTargetItem])
        printf("  **nav target**");
    putchar('\n');
    NSArray *kids = [item children];
    if (kids) {
        // must sort to eliminate arbitrary result ordering which defeats reproducible testing
        kids = [kids sortedArrayUsingFunction:&compareHistoryItems context:nil];
        for (unsigned i = 0; i < [kids count]; i++)
            dumpHistoryItem([kids objectAtIndex:i], indent+4, NO);
    }
}

static void dumpFrameScrollPosition(WebFrame *f)
{
    NSPoint scrollPosition = [[[[f frameView] documentView] superview] bounds].origin;
    if (ABS(scrollPosition.x) > 0.00000001 || ABS(scrollPosition.y) > 0.00000001) {
        if ([f parentFrame] != nil)
            printf("frame '%s' ", [[f name] UTF8String]);
        printf("scrolled to %.f,%.f\n", scrollPosition.x, scrollPosition.y);
    }

    if (layoutTestController->dumpChildFrameScrollPositions()) {
        NSArray *kids = [f childFrames];
        if (kids)
            for (unsigned i = 0; i < [kids count]; i++)
                dumpFrameScrollPosition([kids objectAtIndex:i]);
    }
}

static NSString *dumpFramesAsText(WebFrame *frame)
{
    DOMDocument *document = [frame DOMDocument];
    DOMElement *documentElement = [document documentElement];

    if (!documentElement)
        return @"";

    NSMutableString *result = [[[NSMutableString alloc] init] autorelease];

    // Add header for all but the main frame.
    if ([frame parentFrame])
        result = [NSMutableString stringWithFormat:@"\n--------\nFrame: '%@'\n--------\n", [frame name]];

    [result appendFormat:@"%@\n", [documentElement innerText]];

    if (layoutTestController->dumpChildFramesAsText()) {
        NSArray *kids = [frame childFrames];
        if (kids) {
            for (unsigned i = 0; i < [kids count]; i++)
                [result appendString:dumpFramesAsText([kids objectAtIndex:i])];
        }
    }

    return result;
}

static NSData *dumpFrameAsPDF(WebFrame *frame)
{
    if (!frame)
        return nil;

    // Sadly we have to dump to a file and then read from that file again
    // +[NSPrintOperation PDFOperationWithView:insideRect:] requires a rect and prints to a single page
    // likewise +[NSView dataWithPDFInsideRect:] also prints to a single continuous page
    // The goal of this function is to test "real" printing across multiple pages.
    // FIXME: It's possible there might be printing SPI to let us print a multi-page PDF to an NSData object
    NSString *path = @"/tmp/test.pdf";

    NSMutableDictionary *printInfoDict = [NSMutableDictionary dictionaryWithDictionary:[[NSPrintInfo sharedPrintInfo] dictionary]];
    [printInfoDict setObject:NSPrintSaveJob forKey:NSPrintJobDisposition];
    [printInfoDict setObject:path forKey:NSPrintSavePath];

    NSPrintInfo *printInfo = [[NSPrintInfo alloc] initWithDictionary:printInfoDict];
    [printInfo setHorizontalPagination:NSAutoPagination];
    [printInfo setVerticalPagination:NSAutoPagination];
    [printInfo setVerticallyCentered:NO];

    NSPrintOperation *printOperation = [NSPrintOperation printOperationWithView:[frame frameView] printInfo:printInfo];
    [printOperation setShowPanels:NO];
    [printOperation runOperation];

    [printInfo release];

    NSData *pdfData = [NSData dataWithContentsOfFile:path];
    [[NSFileManager defaultManager] removeFileAtPath:path handler:nil];

    return pdfData;
}

static void convertMIMEType(NSMutableString *mimeType)
{
#ifdef BUILDING_ON_LEOPARD
    // Workaround for <rdar://problem/5539824> on Leopard
    if ([mimeType isEqualToString:@"text/xml"])
        [mimeType setString:@"application/xml"];
#endif
}

static void convertWebResourceDataToString(NSMutableDictionary *resource)
{
    NSMutableString *mimeType = [resource objectForKey:@"WebResourceMIMEType"];
    convertMIMEType(mimeType);
    
    if ([mimeType hasPrefix:@"text/"] || [[WebHTMLRepresentation supportedNonImageMIMETypes] containsObject:mimeType]) {
        NSData *data = [resource objectForKey:@"WebResourceData"];
        NSString *dataAsString = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        [resource setObject:dataAsString forKey:@"WebResourceData"];
    }
}

static void normalizeWebResourceURL(NSMutableString *webResourceURL)
{
    static int fileUrlLength = [(NSString *)@"file://" length];
    NSRange layoutTestsWebArchivePathRange = [webResourceURL rangeOfString:@"/LayoutTests/" options:NSBackwardsSearch];
    if (layoutTestsWebArchivePathRange.location == NSNotFound)
        return;
    NSRange currentWorkingDirectoryRange = NSMakeRange(fileUrlLength, layoutTestsWebArchivePathRange.location - fileUrlLength);
    [webResourceURL replaceCharactersInRange:currentWorkingDirectoryRange withString:@""];
}

static void convertWebResourceResponseToDictionary(NSMutableDictionary *propertyList)
{
    NSURLResponse *response = nil;
    NSData *responseData = [propertyList objectForKey:@"WebResourceResponse"]; // WebResourceResponseKey in WebResource.m
    if ([responseData isKindOfClass:[NSData class]]) {
        // Decode NSURLResponse
        NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:responseData];
        response = [unarchiver decodeObjectForKey:@"WebResourceResponse"]; // WebResourceResponseKey in WebResource.m
        [unarchiver finishDecoding];
        [unarchiver release];
    }        
    
    NSMutableDictionary *responseDictionary = [[NSMutableDictionary alloc] init];
    
    NSMutableString *urlString = [[[response URL] description] mutableCopy];
    normalizeWebResourceURL(urlString);
    [responseDictionary setObject:urlString forKey:@"URL"];
    [urlString release];
    
    NSMutableString *mimeTypeString = [[response MIMEType] mutableCopy];
    convertMIMEType(mimeTypeString);
    [responseDictionary setObject:mimeTypeString forKey:@"MIMEType"];
    [mimeTypeString release];

    NSString *textEncodingName = [response textEncodingName];
    if (textEncodingName)
        [responseDictionary setObject:textEncodingName forKey:@"textEncodingName"];
    [responseDictionary setObject:[NSNumber numberWithLongLong:[response expectedContentLength]] forKey:@"expectedContentLength"];
    
    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
        
        [responseDictionary setObject:[httpResponse allHeaderFields] forKey:@"allHeaderFields"];
        [responseDictionary setObject:[NSNumber numberWithInt:[httpResponse statusCode]] forKey:@"statusCode"];
    }
    
    [propertyList setObject:responseDictionary forKey:@"WebResourceResponse"];
    [responseDictionary release];
}

static NSInteger compareResourceURLs(id resource1, id resource2, void *context)
{
    NSString *url1 = [resource1 objectForKey:@"WebResourceURL"];
    NSString *url2 = [resource2 objectForKey:@"WebResourceURL"];
 
    return [url1 compare:url2];
}

static NSString *serializeWebArchiveToXML(WebArchive *webArchive)
{
    NSString *errorString;
    NSMutableDictionary *propertyList = [NSPropertyListSerialization propertyListFromData:[webArchive data]
                                                                         mutabilityOption:NSPropertyListMutableContainersAndLeaves
                                                                                   format:NULL
                                                                         errorDescription:&errorString];
    if (!propertyList)
        return errorString;

    NSMutableArray *resources = [NSMutableArray arrayWithCapacity:1];
    [resources addObject:propertyList];

    while ([resources count]) {
        NSMutableDictionary *resourcePropertyList = [resources objectAtIndex:0];
        [resources removeObjectAtIndex:0];

        NSMutableDictionary *mainResource = [resourcePropertyList objectForKey:@"WebMainResource"];
        normalizeWebResourceURL([mainResource objectForKey:@"WebResourceURL"]);
        convertWebResourceDataToString(mainResource);

        // Add subframeArchives to list for processing
        NSMutableArray *subframeArchives = [resourcePropertyList objectForKey:@"WebSubframeArchives"]; // WebSubframeArchivesKey in WebArchive.m
        if (subframeArchives)
            [resources addObjectsFromArray:subframeArchives];

        NSMutableArray *subresources = [resourcePropertyList objectForKey:@"WebSubresources"]; // WebSubresourcesKey in WebArchive.m
        NSEnumerator *enumerator = [subresources objectEnumerator];
        NSMutableDictionary *subresourcePropertyList;
        while ((subresourcePropertyList = [enumerator nextObject])) {
            normalizeWebResourceURL([subresourcePropertyList objectForKey:@"WebResourceURL"]);
            convertWebResourceResponseToDictionary(subresourcePropertyList);
            convertWebResourceDataToString(subresourcePropertyList);
        }
        
        // Sort the subresources so they're always in a predictable order for the dump
        if (NSArray *sortedSubresources = [subresources sortedArrayUsingFunction:compareResourceURLs context:nil])
            [resourcePropertyList setObject:sortedSubresources forKey:@"WebSubresources"];
    }

    NSData *xmlData = [NSPropertyListSerialization dataFromPropertyList:propertyList
                                                                 format:NSPropertyListXMLFormat_v1_0
                                                       errorDescription:&errorString];
    if (!xmlData)
        return errorString;

    NSMutableString *string = [[[NSMutableString alloc] initWithData:xmlData encoding:NSUTF8StringEncoding] autorelease];

    // Replace "Apple Computer" with "Apple" in the DTD declaration.
    NSRange range = [string rangeOfString:@"-//Apple Computer//"];
    if (range.location != NSNotFound)
        [string replaceCharactersInRange:range withString:@"-//Apple//"];
    
    return string;
}

static void dumpBackForwardListForWebView(WebView *view)
{
    printf("\n============== Back Forward List ==============\n");
    WebBackForwardList *bfList = [view backForwardList];

    // Print out all items in the list after prevTestBFItem, which was from the previous test
    // Gather items from the end of the list, the print them out from oldest to newest
    NSMutableArray *itemsToPrint = [[NSMutableArray alloc] init];
    for (int i = [bfList forwardListCount]; i > 0; i--) {
        WebHistoryItem *item = [bfList itemAtIndex:i];
        // something is wrong if the item from the last test is in the forward part of the b/f list
        assert(item != prevTestBFItem);
        [itemsToPrint addObject:item];
    }
            
    assert([bfList currentItem] != prevTestBFItem);
    [itemsToPrint addObject:[bfList currentItem]];
    int currentItemIndex = [itemsToPrint count] - 1;

    for (int i = -1; i >= -[bfList backListCount]; i--) {
        WebHistoryItem *item = [bfList itemAtIndex:i];
        if (item == prevTestBFItem)
            break;
        [itemsToPrint addObject:item];
    }

    for (int i = [itemsToPrint count]-1; i >= 0; i--)
        dumpHistoryItem([itemsToPrint objectAtIndex:i], 8, i == currentItemIndex);

    [itemsToPrint release];
    printf("===============================================\n");
}

static void sizeWebViewForCurrentTest()
{
    // W3C SVG tests expect to be 480x360
    bool isSVGW3CTest = ([currentTest rangeOfString:@"svg/W3C-SVG-1.1"].length);
    if (isSVGW3CTest)
        [[mainFrame webView] setFrameSize:NSMakeSize(480, 360)];
    else
        [[mainFrame webView] setFrameSize:NSMakeSize(maxViewWidth, maxViewHeight)];
}

static const char *methodNameStringForFailedTest()
{
    const char *errorMessage;
    if (layoutTestController->dumpAsText())
        errorMessage = "[documentElement innerText]";
    else if (layoutTestController->dumpDOMAsWebArchive())
        errorMessage = "[[mainFrame DOMDocument] webArchive]";
    else if (layoutTestController->dumpSourceAsWebArchive())
        errorMessage = "[[mainFrame dataSource] webArchive]";
    else
        errorMessage = "[mainFrame renderTreeAsExternalRepresentation]";

    return errorMessage;
}

static void dumpBackForwardListForAllWindows()
{
    CFArrayRef openWindows = (CFArrayRef)[DumpRenderTreeWindow openWindows];
    unsigned count = CFArrayGetCount(openWindows);
    for (unsigned i = 0; i < count; i++) {
        NSWindow *window = (NSWindow *)CFArrayGetValueAtIndex(openWindows, i);
        WebView *webView = [[[window contentView] subviews] objectAtIndex:0];
        dumpBackForwardListForWebView(webView);
    }
}

static void invalidateAnyPreviousWaitToDumpWatchdog()
{
    if (waitToDumpWatchdog) {
        CFRunLoopTimerInvalidate(waitToDumpWatchdog);
        CFRelease(waitToDumpWatchdog);
        waitToDumpWatchdog = 0;
    }
}

void dump()
{
    invalidateAnyPreviousWaitToDumpWatchdog();

    if (dumpTree) {
        NSString *resultString = nil;
        NSData *resultData = nil;
        NSString *resultMimeType = @"text/plain";

        bool dumpAsText = layoutTestController->dumpAsText();
        dumpAsText |= [[[mainFrame dataSource] _responseMIMEType] isEqualToString:@"text/plain"];
        layoutTestController->setDumpAsText(dumpAsText);
        if (layoutTestController->dumpAsText()) {
            resultString = dumpFramesAsText(mainFrame);
        } else if (layoutTestController->dumpAsPDF()) {
            resultData = dumpFrameAsPDF(mainFrame);
            resultMimeType = @"application/pdf";
        } else if (layoutTestController->dumpDOMAsWebArchive()) {
            WebArchive *webArchive = [[mainFrame DOMDocument] webArchive];
            resultString = serializeWebArchiveToXML(webArchive);
            resultMimeType = @"application/x-webarchive";
        } else if (layoutTestController->dumpSourceAsWebArchive()) {
            WebArchive *webArchive = [[mainFrame dataSource] webArchive];
            resultString = serializeWebArchiveToXML(webArchive);
            resultMimeType = @"application/x-webarchive";
        } else {
            sizeWebViewForCurrentTest();
            resultString = [mainFrame renderTreeAsExternalRepresentation];
        }

        if (resultString && !resultData)
            resultData = [resultString dataUsingEncoding:NSUTF8StringEncoding];

        printf("Content-Type: %s\n", [resultMimeType UTF8String]);

        if (resultData) {
            fwrite([resultData bytes], 1, [resultData length], stdout);

            if (!layoutTestController->dumpAsText() && !layoutTestController->dumpDOMAsWebArchive() && !layoutTestController->dumpSourceAsWebArchive())
                dumpFrameScrollPosition(mainFrame);

            if (layoutTestController->dumpBackForwardList())
                dumpBackForwardListForAllWindows();
        } else
            printf("ERROR: nil result from %s", methodNameStringForFailedTest());

        if (printSeparators) {
            puts("#EOF");
            fputs("#EOF\n", stderr);
        }            
    }
    
    if (dumpPixels)
        dumpWebViewAsPixelsAndCompareWithExpected([currentTest UTF8String], dumpAllPixels);

    fflush(stdout);
    fflush(stderr);

    done = YES;
}

static bool shouldLogFrameLoadDelegates(const char *pathOrURL)
{
    return strstr(pathOrURL, "loading/");
}

static CFURLRef createCFURLFromPathOrURL(CFStringRef pathOrURLString)
{
    CFURLRef URL;
    if (CFStringHasPrefix(pathOrURLString, CFSTR("http://")) || CFStringHasPrefix(pathOrURLString, CFSTR("https://")))
        URL = CFURLCreateWithString(NULL, pathOrURLString, NULL);
    else
        URL = CFURLCreateWithFileSystemPath(NULL, pathOrURLString, kCFURLPOSIXPathStyle, FALSE);
    return URL;
}

static void resetWebViewToConsistentStateBeforeTesting()
{
    WebView *webView = [mainFrame webView];
    [(EditingDelegate *)[webView editingDelegate] setAcceptsEditing:YES];
    [webView makeTextStandardSize:nil];
    [webView resetPageZoom:nil];
    [webView setTabKeyCyclesThroughElements:YES];
    [webView setPolicyDelegate:nil];
    [webView _setDashboardBehavior:WebDashboardBehaviorUseBackwardCompatibilityMode to:NO];

    WebPreferences *preferences = [webView preferences];
    [preferences setPrivateBrowsingEnabled:NO];
    [preferences setAuthorAndUserStylesEnabled:YES];
    [preferences setJavaScriptCanOpenWindowsAutomatically:YES];
    [preferences setOfflineWebApplicationCacheEnabled:YES];
    [preferences setFullDocumentTeardownEnabled:YES];
    
    if (persistentUserStyleSheetLocation) {
        [preferences setUserStyleSheetLocation:[NSURL URLWithString:(NSString *)(persistentUserStyleSheetLocation.get())]];
        [preferences setUserStyleSheetEnabled:YES];
    } else
        [preferences setUserStyleSheetEnabled:NO];

    [WebView _setUsesTestModeFocusRingColor:YES];
}

static void runTest(const char *pathOrURL)
{
    CFStringRef pathOrURLString = CFStringCreateWithCString(NULL, pathOrURL, kCFStringEncodingUTF8);
    if (!pathOrURLString) {
        fprintf(stderr, "Failed to parse filename as UTF-8: %s\n", pathOrURL);
        return;
    }

    CFURLRef URL = createCFURLFromPathOrURL(pathOrURLString);
    if (!URL) {
        CFRelease(pathOrURLString);
        fprintf(stderr, "Can't turn %s into a CFURL\n", pathOrURL);
        return;
    }

    resetWebViewToConsistentStateBeforeTesting();

    layoutTestController = new LayoutTestController(testRepaintDefault, repaintSweepHorizontallyDefault);
    topLoadingFrame = nil;
    done = NO;

    if (disallowedURLs)
        CFSetRemoveAllValues(disallowedURLs);
    if (shouldLogFrameLoadDelegates(pathOrURL))
        layoutTestController->setDumpFrameLoadCallbacks(true);

    if ([WebHistory optionalSharedHistory])
        [WebHistory setOptionalSharedHistory:nil];
    lastMousePosition = NSZeroPoint;
    lastClickPosition = NSZeroPoint;

    if (currentTest != nil)
        CFRelease(currentTest);
    currentTest = (NSString *)pathOrURLString;
    [prevTestBFItem release];
    prevTestBFItem = [[[[mainFrame webView] backForwardList] currentItem] retain];

    WorkQueue::shared()->clear();
    WorkQueue::shared()->setFrozen(false);

    BOOL _shouldIgnoreWebCoreNodeLeaks = shouldIgnoreWebCoreNodeLeaks(CFURLGetString(URL));
    if (_shouldIgnoreWebCoreNodeLeaks)
        [WebCoreStatistics startIgnoringWebCoreNodeLeaks];

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [mainFrame loadRequest:[NSURLRequest requestWithURL:(NSURL *)URL]];
    CFRelease(URL);
    [pool release];
    while (!done) {
        pool = [[NSAutoreleasePool alloc] init];
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantPast]];
        [pool release];
    }
    pool = [[NSAutoreleasePool alloc] init];
    [EventSendingController clearSavedEvents];
    [[mainFrame webView] setSelectedDOMRange:nil affinity:NSSelectionAffinityDownstream];

    WorkQueue::shared()->clear();

    if (layoutTestController->closeRemainingWindowsWhenComplete()) {
        NSArray* array = [DumpRenderTreeWindow openWindows];

        unsigned count = [array count];
        for (unsigned i = 0; i < count; i++) {
            NSWindow *window = [array objectAtIndex:i];

            // Don't try to close the main window
            if (window == [[mainFrame webView] window])
                continue;
            
            WebView *webView = [[[window contentView] subviews] objectAtIndex:0];

            [webView close];
            [window close];
        }
    }
    
    [mainFrame loadHTMLString:@"<html></html>" baseURL:[NSURL URLWithString:@"about:blank"]];
    [mainFrame stopLoading];
    
    [pool release];

    // We should only have our main window left open when we're done
    ASSERT(CFArrayGetCount(openWindowsRef) == 1);
    ASSERT(CFArrayGetValueAtIndex(openWindowsRef, 0) == [[mainFrame webView] window]);

    delete layoutTestController;
    layoutTestController = 0;

    if (_shouldIgnoreWebCoreNodeLeaks)
        [WebCoreStatistics stopIgnoringWebCoreNodeLeaks];
}

void displayWebView()
{
    NSView *webView = [mainFrame webView];
    [webView display];
    [webView lockFocus];
    [[[NSColor blackColor] colorWithAlphaComponent:0.66] set];
    NSRectFillUsingOperation([webView frame], NSCompositeSourceOver);
    [webView unlockFocus];
}

@implementation DumpRenderTreeEvent

+ (NSPoint)mouseLocation
{
    return [[[mainFrame webView] window] convertBaseToScreen:lastMousePosition];
}

@end
