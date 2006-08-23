/*
 * Copyright 2006 Apple Computer, Inc. All rights reserved.
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

#import "WebSystemInterface.h"

#import <WebCore/WebCoreSystemInterface.h>
#import <WebKitSystemInterface.h>

#define INIT(function) wk##function = WK##function

void InitWebCoreSystemInterface(void)
{
    static bool didInit;
    if (didInit)
        return;

    INIT(CGContextGetShouldSmoothFonts);
    INIT(ClearGlyphVector);
    INIT(ConvertCharToGlyphs);
    INIT(DrawBezeledTextFieldCell);
    INIT(DrawBezeledTextArea);
    INIT(DrawFocusRing);
    INIT(FontSmoothingModeIsLCD);
    INIT(GetATSStyleGroup);
    INIT(GetCGFontFromNSFont);
    INIT(GetDefaultGlyphForChar);
    INIT(GetGlyphTransformedAdvances);
    INIT(GetFontInLanguageForRange);
    INIT(GetFontInLanguageForCharacter);
    INIT(GetFontMetrics);
    INIT(GetGlyphVectorFirstRecord);
    INIT(GetGlyphVectorNumGlyphs);
    INIT(GetGlyphVectorRecordSize);
    INIT(GetMIMETypeForExtension);
    INIT(GetNSFontATSUFontId);
    INIT(InitializeGlyphVector);
    INIT(PathFromFont);
    INIT(PopupMenu);
    INIT(ReleaseStyleGroup);
    INIT(SetCGFontRenderingMode);
    INIT(SetDragImage);
    INIT(SetPatternPhaseInUserSpace);
    INIT(SetUpFontCache);
    INIT(SignalCFReadStreamEnd);
    INIT(SignalCFReadStreamHasBytes);
    INIT(SignalCFReadStreamError);
    INIT(CreateCustomCFReadStream);
    INIT(SetNSURLConnectionDefersCallbacks);
    INIT(CreateNSURLConnectionDelegateProxy);
    
    didInit = true;
}
