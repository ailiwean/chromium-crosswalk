/*
 * Copyright (C) 2002 Apple Computer, Inc.  All rights reserved.
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

#ifndef KWQKHTMLPARTIMPL_H
#define KWQKHTMLPARTIMPL_H

#include <qobject.h>
#include <kurl.h>

class KHTMLPart;
class KHTMLPartPrivate;
class KHTMLView;

namespace KParts {
    class ReadOnlyPart;
    class URLArgs;
}

namespace DOM {
    class DOMString;
}

namespace khtml {
    class RenderObject;
    class RenderPart;
}

#ifdef __OBJC__
@class WebCoreBridge;
@class NSMutableSet;
#else
class WebCoreBridge;
class NSMutableSet;
#endif

class KWQKHTMLPartImpl : public QObject
{
public:
    KWQKHTMLPartImpl(KHTMLPart *);
    ~KWQKHTMLPartImpl();
    
    void setBridge(WebCoreBridge *p) { bridge = p; }
    WebCoreBridge *getBridge() const { return bridge; }
    void setView(KHTMLView *view);
    KHTMLView *getView() const;

    bool openURLInFrame(const KURL &, const KParts::URLArgs &);
    void openURL(const KURL &);
    void begin(const KURL &, int xOffset, int yOffset);
    void write(const char *str, int len);
    void end();
    
    void slotData(NSString *, const char *bytes, int length, bool complete = false);

    void scheduleRedirection(int delay, const QString &url);
    void redirectURL();
    virtual void timerEvent(QTimerEvent *);

    bool gotoBaseAnchor();

    void setBaseURL(const KURL &);

    void setTitle(const DOM::DOMString &);
    void setStatusBarText(const QString &status);
    QString documentSource() const;

    bool frameExists(const QString &frameName);
    KHTMLPart *findFrame(const QString &frameName);
    QPtrList<KParts::ReadOnlyPart> frames() const;

    KHTMLPart *parentPart();

    void urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs);
    bool requestFrame(khtml::RenderPart *frame, const QString &url, const QString &frameName, const QStringList &params, bool isIFrame);
    bool requestObject(khtml::RenderPart *frame, const QString &url, const QString &serviceType, const QStringList &args);

    void submitForm(const char *action, const QString &url, const QByteArray &formData, const QString &_target, const QString& contentType, const QString& boundary);

    bool openedByJS();
    void setOpenedByJS(bool _openedByJS);

    void scheduleClose();

    void unfocusWindow();

    bool isFrameSet();

    void overURL(const QString &url, const QString &target, int modifierState);

    // Draw the contents of the view, with the side effect of putting all the widgets into place.
    void paint(QPainter &, int x, int y, int width, int height);
    
    // Used by KWQWidget to tell the paint call that this widget should stay.
    static void addedWidget(QWidget *);

private:
    KHTMLPart *part;
    KHTMLPartPrivate *d;
    
    WebCoreBridge *bridge;

    int m_redirectionTimer;
    
    KURL m_baseURL;
    QString m_documentSource;
    bool m_decodingStarted;
    
    static void buildViewsNotYetAddedSet(khtml::RenderObject *);
    static NSMutableSet *viewsNotYetAdded;
    
    friend class KHTMLPart;
};

#endif
