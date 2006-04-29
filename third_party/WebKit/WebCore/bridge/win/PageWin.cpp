/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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

#include "config.h"
#include "Page.h"

#include "Frame.h"
#include "IntRect.h"
#include <windows.h>

namespace WebCore {

Page::Page()
    : m_frameCount(0)
{
    init();
}

static HWND rootWindowForFrame(const Frame* frame)
{
    if (!frame)
        return 0;
    FrameView* frameView = frame->view();
    if (!frameView)
        return 0;
    HWND frameWnd = frameView->windowHandle();
    if (!frameWnd)
        return 0;
    return GetAncestor(frameWnd, GA_ROOT);
}

IntRect Page::windowRect() const
{
    HWND windowHandle = rootWindowForFrame(mainFrame());
    if (!windowHandle)
        return IntRect();
    RECT rect;
    GetWindowRect(windowHandle, &rect);
    return rect;
}

void Page::setWindowRect(const IntRect& r)
{
    HWND windowHandle = rootWindowForFrame(mainFrame());
    if (!windowHandle)
        return;
    MoveWindow(windowHandle, r.x(), r.y(), r.width(), r.height(), true);
}

}
