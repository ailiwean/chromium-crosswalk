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

#ifndef KWQKHTMLPart_H
#define KWQKHTMLPart_H

#include "khtml_part.h"

#include "dom_nodeimpl.h"
#include "html_formimpl.h"
#include "html_tableimpl.h"

#import "WebCoreKeyboardAccess.h"

#include <CoreFoundation/CoreFoundation.h>

#include <JavaVM/jni.h>
#include <JavaScriptCore/jni_jsobject.h>
#include <JavaScriptCore/npruntime.h>
#include <JavaScriptCore/NP_jsobject.h>
#include <JavaScriptCore/runtime.h>

#include "KWQDict.h"
#include "KWQClipboard.h"

class KHTMLPartPrivate;
class KWQClipboard;
class KWQWindowWidget;

namespace khtml {
    class RenderObject;
}

namespace KJS {
    class SavedProperties;
    class SavedBuiltins;
    class ScheduledAction;
}

#ifdef __OBJC__

// Avoid clashes with KJS::DOMElement in KHTML code.
@class DOMElement;
typedef DOMElement ObjCDOMElement;

@class KWQPageState;
@class NSArray;
@class NSAttributedString;
@class NSColor;
@class NSEvent;
@class NSFileWrapper;
@class NSFont;
@class NSImage;
@class NSMutableDictionary;
@class NSResponder;
@class NSString;
@class NSView;
@class WebCoreBridge;
@class WebScriptObject;

#else

// Avoid clashes with KJS::DOMElement in KHTML code.
class ObjCDOMElement;

class KWQPageState;
class NSArray;
class NSAttributedString;
class NSColor;
class NSEvent;
class NSFileWrapper;
class NSFont;
class NSImage;
class NSMutableDictionary;
class NSResponder;
class NSString;
class NSView;
class WebCoreBridge;
class WebScriptObject;

#endif

enum KWQSelectionDirection {
    KWQSelectingNext,
    KWQSelectingPrevious
};

class KWQKHTMLPart : public KHTMLPart
{
public:
    KWQKHTMLPart();
    ~KWQKHTMLPart();
    
    void clear();

    void setBridge(WebCoreBridge *p);
    WebCoreBridge *bridge() const { return _bridge; }
    void setView(KHTMLView *view);
    KHTMLView *view() const;

    void provisionalLoadStarted();

    virtual bool openURL(const KURL &);
    virtual bool closeURL();
    void didNotOpenURL(const KURL &);
    
    void openURLRequest(const KURL &, const KParts::URLArgs &);
    void submitForm(const KURL &, const KParts::URLArgs &);

    void scheduleHistoryNavigation( int steps );
    
    void scrollToAnchor(const KURL &);
    void jumpToSelection();
    
    void setEncoding(const QString &encoding, bool userChosen);
    void addData(const char *bytes, int length);

    void setTitle(const DOM::DOMString &);
    void setStatusBarText(const QString &status);

    void urlSelected(const KURL &url, int button, int state, const KParts::URLArgs &args);
    bool requestObject(khtml::RenderPart *frame, const QString &url, const QString &serviceType, const QStringList &args);
    KParts::ReadOnlyPart *createPart(const khtml::ChildFrame &child, const KURL &url, const QString &mimeType);

    void scheduleClose();

    void unfocusWindow();

    QMap<int, KJS::ScheduledAction*> *pauseActions(const void *key);
    void resumeActions(QMap<int, KJS::ScheduledAction*> *actions, const void *key);
    
    bool canCachePage();
    void saveWindowProperties(KJS::SavedProperties *windowProperties);
    void saveLocationProperties(KJS::SavedProperties *locationProperties);
    void restoreWindowProperties(KJS::SavedProperties *windowProperties);
    void restoreLocationProperties(KJS::SavedProperties *locationProperties);
    void saveInterpreterBuiltins(KJS::SavedBuiltins &interpreterBuiltins);
    void restoreInterpreterBuiltins(const KJS::SavedBuiltins &interpreterBuiltins);

    void openURLFromPageCache(KWQPageState *state);

    void saveDocumentState();
    void restoreDocumentState();
    
    bool isFrameSet() const;

    void updatePolicyBaseURL();

    NSView *nextKeyView(DOM::NodeImpl *startingPoint, KWQSelectionDirection);
    NSView *nextKeyViewInFrameHierarchy(DOM::NodeImpl *startingPoint, KWQSelectionDirection);
    static NSView *nextKeyViewForWidget(QWidget *startingPoint, KWQSelectionDirection);
    static bool currentEventIsKeyboardOptionTab();
    static bool handleKeyboardOptionTabInView(NSView *view);
    
    virtual bool tabsToLinks() const;
    virtual bool tabsToAllControls() const;
    
    static bool currentEventIsMouseDownInWidget(QWidget *candidate);
    
    static void setDocumentFocus(QWidget *);
    static void clearDocumentFocus(QWidget *);
    
    void runJavaScriptAlert(const QString &message);
    bool runJavaScriptConfirm(const QString &message);
    bool runJavaScriptPrompt(const QString &message, const QString &defaultValue, QString &result);
    void KWQKHTMLPart::addMessageToConsole(const QString &message,  unsigned int lineNumber, const QString &sourceID);
    using KHTMLPart::xmlDocImpl;
    khtml::RenderObject *renderer();
    void forceLayout();
    void forceLayoutWithPageWidthRange(float minPageWidth, float maxPageWidth);
    void sendResizeEvent();
    void sendScrollEvent();
    void paint(QPainter *, const QRect &);

    void adjustPageHeight(float *newBottom, float oldTop, float oldBottom, float bottomLimit);

    void createEmptyDocument();

    static WebCoreBridge *bridgeForWidget(const QWidget *);
    static KWQKHTMLPart *partForWidget(const QWidget *);
    
    QString requestedURLString() const;
    QString incomingReferrer() const;
    QString userAgent() const;

    QString mimeTypeForFileName(const QString &) const;
    
    DOM::NodeImpl *selectionStart() const;
    int selectionStartOffset() const;
    DOM::NodeImpl *selectionEnd() const;
    int selectionEndOffset() const;

    QRect selectionRect() const;
    NSRect visibleSelectionRect() const;
    NSImage *selectionImage() const;
    NSImage *snapshotDragImage(DOM::Node node, NSRect *imageRect, NSRect *elementRect) const;

    NSFont *fontForCurrentPosition() const;

    NSFileWrapper *fileWrapperForElement(DOM::ElementImpl *);
    NSAttributedString *attributedString(DOM::NodeImpl *startNode, int startOffset, DOM::NodeImpl *endNode, int endOffset);

    void addMetaData(const QString &key, const QString &value);

    void mouseDown(NSEvent *);
    void mouseDragged(NSEvent *);
    void mouseUp(NSEvent *);
    void sendFakeEventsAfterWidgetTracking(NSEvent *initiatingEvent);
    void mouseMoved(NSEvent *);
    bool keyEvent(NSEvent *);
    bool lastEventIsMouseUp();
    void setActivationEventNumber(int num) { _activationEventNumber = num; }

    bool eventMayStartDrag(NSEvent *);
    void dragSourceMovedTo(const QPoint &loc);
    void dragSourceEndedAt(const QPoint &loc, NSDragOperation operation);

    bool mayCut();
    bool mayCopy();
    bool mayPaste();
    bool tryCut();
    bool tryCopy();
    bool tryPaste();
    
    bool sendContextMenuEvent(NSEvent *);

    void clearTimers();
    static void clearTimers(KHTMLView *);
    
    bool passSubframeEventToSubframe(DOM::NodeImpl::MouseEvent &);
    
    void redirectionTimerStartedOrStopped();
    
    static const QPtrList<KWQKHTMLPart> &instances() { return mutableInstances(); }

    void clearRecordedFormValues();
    void recordFormValue(const QString &name, const QString &value, DOM::HTMLFormElementImpl *element);
    DOM::HTMLFormElementImpl *currentForm() const;

    NSString *searchForLabelsAboveCell(QRegExp *regExp, DOM::HTMLTableCellElementImpl *cell);
    NSString *searchForLabelsBeforeElement(NSArray *labels, DOM::ElementImpl *element);
    NSString *matchLabelsAgainstElement(NSArray *labels, DOM::ElementImpl *element);

    bool findString(NSString *str, bool forward, bool caseFlag, bool wrapFlag);

    void setSettings(KHTMLSettings *);

    KWQWindowWidget *topLevelWidget();
    
    QString overrideMediaType();
    
    void setMediaType(const QString &);

    void setDisplaysWithFocusAttributes(bool flag);
    bool displaysWithFocusAttributes() const { return _displaysWithFocusAttributes; }
    
    // Convenience, to avoid repeating the code to dig down to get this.
    QChar backslashAsCurrencySymbol() const;

    NSColor *bodyBackgroundColor() const;
    
    WebCoreKeyboardUIMode keyboardUIMode() const;

    void setName(const QString &name);

    void didTellBridgeAboutLoad(const QString &urlString);
    bool haveToldBridgeAboutLoad(const QString &urlString);

    KJS::Bindings::Instance *getEmbedInstanceForView (NSView *aView);
    KJS::Bindings::Instance *getAppletInstanceForView (NSView *aView);
    void addPluginRootObject(const KJS::Bindings::RootObject *root);
    void cleanupPluginRootObjects();
    
    void registerCommandForUndo(const khtml::EditCommand &);
    void registerCommandForRedo(const khtml::EditCommand &);
    void clearUndoRedoOperations();
    void issueUndoCommand();
    void issueRedoCommand();
    void issueCutCommand();
    void issueCopyCommand();
    void issuePasteCommand();
    void respondToChangedSelection();
    void respondToChangedContents();
    bool isContentEditable() const;
    bool shouldBeginEditing(const DOM::Range &) const;
    bool shouldEndEditing(const DOM::Range &) const;

    KJS::Bindings::RootObject *bindingRootObject();
    
    WebScriptObject *windowScriptObject();
    NPObject *KWQKHTMLPart::windowScriptNPObject();
    
    void partClearedInBegin();
    
    // Implementation of CSS property -khtml-user-drag == auto
    bool shouldDragAutoNode(DOM::NodeImpl*, int x, int y) const;

private:
    virtual void khtmlMousePressEvent(khtml::MousePressEvent *);
    virtual void khtmlMouseDoubleClickEvent(khtml::MouseDoubleClickEvent *);
    virtual void khtmlMouseMoveEvent(khtml::MouseMoveEvent *);
    virtual void khtmlMouseReleaseEvent(khtml::MouseReleaseEvent *);
    
    bool passWidgetMouseDownEventToWidget(khtml::MouseEvent *);
    bool passWidgetMouseDownEventToWidget(khtml::RenderWidget *);
    bool passWidgetMouseDownEventToWidget(QWidget *);
    
    void setPolicyBaseURL(const DOM::DOMString &);
    
    NSView *mouseDownViewIfStillGood();

    QString generateFrameName();

    NSView *nextKeyViewInFrame(DOM::NodeImpl *startingPoint, KWQSelectionDirection);
    static DOM::NodeImpl *nodeForWidget(const QWidget *);
    static KWQKHTMLPart *partForNode(DOM::NodeImpl *);
    static NSView *documentViewForNode(DOM::NodeImpl *);
    
    bool dragHysteresisExceeded(float dragLocationX, float dragLocationY) const;
    bool dispatchCPPEvent(int eventId, KWQClipboard::AccessPolicy policy);
    bool dispatchDragSrcEvent(int eventId, const QPoint &loc) const;

    NSImage *imageFromRect(NSRect rect) const;

    void freeClipboard();

    WebCoreBridge *_bridge;
    
    KWQSignal _started;
    KWQSignal _completed;
    KWQSignal _completedWithBool;
    
    NSView *_mouseDownView;
    bool _mouseDownWasInSubframe;
    bool _sendingEventToSubview;
    bool _mouseDownMayStartDrag;
    bool _mouseDownMayStartSelect;
    // in our window's coords
    int _mouseDownWinX, _mouseDownWinY;
    // in our view's coords
    int _mouseDownX, _mouseDownY;
    float _mouseDownTimestamp;
    int _activationEventNumber;
    
    static NSEvent *_currentEvent;
    static NSResponder *_firstResponderAtMouseDownTime;

    KURL _submittedFormURL;

    NSMutableDictionary *_formValuesAboutToBeSubmitted;
    ObjCDOMElement *_formAboutToBeSubmitted;

    static QPtrList<KWQKHTMLPart> &mutableInstances();

    KWQWindowWidget *_windowWidget;

    bool _displaysWithFocusAttributes;
    mutable bool _drawSelectionOnly;
    bool _haveUndoRedoOperations;
    
    QDict<char> urlsBridgeKnowsAbout;

    friend class KHTMLPart;

    KJS::Bindings::RootObject *_bindingRoot;  // The root object used for objects
                                            // bound outside the context of a plugin.
    QPtrList<KJS::Bindings::RootObject> rootObjects;
    WebScriptObject *_windowScriptObject;
    NPObject *_windowScriptNPObject;
    
    DOM::Node _dragSrc;     // element that may be a drag source, for the current mouse gesture
    bool _dragSrcIsLink;
    bool _dragSrcIsImage;
    bool _dragSrcInSelection;
    bool _dragSrcMayBeDHTML, _dragSrcMayBeUA;   // Are DHTML and/or the UserAgent allowed to drag out?
    bool _dragSrcIsDHTML;
    KWQClipboard *_dragClipboard;   // used on only the source side of dragging
    
    mutable DOM::Node _elementToDraw;
};

inline KWQKHTMLPart *KWQ(KHTMLPart *part) { return static_cast<KWQKHTMLPart *>(part); }
inline const KWQKHTMLPart *KWQ(const KHTMLPart *part) { return static_cast<const KWQKHTMLPart *>(part); }

#endif
