/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
 
#import <WebCore/WebCoreScrollView.h>

@class KWQTextAreaTextView;
class QTextEdit;
@protocol KWQWidgetHolder;

@interface KWQTextArea : WebCoreScrollView <KWQWidgetHolder>
{
    KWQTextAreaTextView *textView;
    QTextEdit *widget;
    BOOL wrap;
    BOOL inNextValidKeyView;
}

- initWithQTextEdit:(QTextEdit *)w; 

// The following methods corresponds to methods required by KDE.
- (void)setWordWrap:(BOOL)wrap;
- (BOOL)wordWrap;
- (void)setText:(NSString *)text;
- (NSString *)text;
- (int)numLines;
- (NSString *)textForLine:(int)line;
- (void)selectAll;
- (void)setEditable:(BOOL)flag;
- (BOOL)isEditable;
- (void)setFont:(NSFont *)font;

// paragraph-oriented functions for the benefit of QTextEdit
- (int)paragraphs;
- (int)paragraphLength:(int)paragraph;
- (NSString *)textForParagraph:(int)paragraph;
- (int)lineOfCharAtIndex:(int)index inParagraph:(int)paragraph;
- (void)getCursorPositionAsIndex:(int *)index inParagraph:(int *)paragraph;
- (void)setCursorPositionToIndex:(int)index inParagraph:(int)paragraph;

@end
