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

#ifndef BROWSEREXTENSION_H_
#define BROWSEREXTENSION_H_

#include <qpoint.h>
#include <qevent.h>
#include <KWQDataStream.h>

#include <kurl.h>

#include "part.h"
#include "browserinterface.h"

class KXMLGUIClient { };

namespace KParts {

struct URLArgs {

    QString frameName;
    QString serviceType;
    int xOffset;
    int yOffset;

    URLArgs() : xOffset(0), yOffset(0) { }

};

struct WindowArgs {

    int x;
    int y;
    int width;
    int height;
    bool menuBarVisible;
    bool statusBarVisible;
    bool toolBarsVisible;
    bool resizable;
    bool fullscreen;

    WindowArgs() : x(0), y(0), width(0), height(0), menuBarVisible(false), statusBarVisible(false), toolBarsVisible(false), resizable(false), fullscreen(false) { }

};

class BrowserExtension : public QObject {
public:
     BrowserInterface *browserInterface() const { return 0; }
     
     void openURLRequest(const KURL &) { }
     void openURLRequest(const KURL &, const KParts::URLArgs &args) { }
     
     void createNewWindow(const KURL &) { }
     void createNewWindow(const KURL &, const KParts::URLArgs &) { }
     void createNewWindow(const KURL &, const KParts::URLArgs &, 
        const KParts::WindowArgs &, KParts::ReadOnlyPart *&) { }

     void setIconURL(const KURL &) { }
     
     void setURLArgs(const KParts::URLArgs &args) { m_args = args; }
     KParts::URLArgs urlArgs() const { return m_args; }

private:
    KParts::URLArgs m_args;
};

class BrowserHostExtension : public QObject { };

} // namespace KParts

#endif
