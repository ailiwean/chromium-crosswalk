/*
 * Copyright (C) 2001 Apple Computer, Inc.  All rights reserved.
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
#include <Cocoa/Cocoa.h>

class QWidget;

@interface KWQNSTextFieldFormatter : NSFormatter
{
    int maxLength;
    bool isPassword;
}

- (void)setPasswordMode: (bool)flag;
- (bool)passwordMode;
- (void)setMaximumLength: (int)len;
- (int)maximumLength;
- (NSString *)stringForObjectValue:(id)anObject;
- (BOOL)getObjectValue:(id *)obj forString:(NSString *)string errorDescription:(NSString  **)error;
- (BOOL)isPartialStringValid:(NSString *)partialString newEditingString:(NSString **)newString errorDescription:(NSString **)error;
- (NSAttributedString *)attributedStringForObjectValue:(id)anObject withDefaultAttributes:(NSDictionary *)attributes;

@end

@interface KWQNSTextField : NSTextField
{
@private
    NSSecureTextField *secureField;
    QWidget *widget;
    KWQNSTextFieldFormatter *formatter;
}

- initWithFrame: (NSRect)r widget: (QWidget *)w;
- (KWQNSTextFieldFormatter *)formatter;
- (void)setPasswordMode: (bool)flag;
- (bool)passwordMode;
- (void)setMaximumLength: (int)len;
- (int)maximumLength;

@end

