/*	
        WebDefaultPolicyHandler.h
	Copyright 2002, Apple Computer, Inc.

        Public header file.
*/

@class WebController;

@interface WebDefaultPolicyHandler : NSObject <WebControllerPolicyHandler>
{
    WebController *webController;
}
- initWithWebController: (WebController *)wc;
@end

