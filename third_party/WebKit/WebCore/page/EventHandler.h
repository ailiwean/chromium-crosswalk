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

#ifndef EventHandler_h
#define EventHandler_h

#include "PlatformMouseEvent.h"
#include "ScrollTypes.h"
#include "Timer.h"
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/Platform.h>
#include <wtf/RefPtr.h>

#if PLATFORM(MAC)
#include "WebCoreKeyboardUIMode.h"
#ifndef __OBJC__
typedef unsigned NSDragOperation;
class NSView;
#endif
#endif

namespace WebCore {

class AtomicString;
class Clipboard;
class EventTargetNode;
class Event;
class FloatPoint;
class FloatRect;
class Frame;
class HitTestResult;
class HTMLFrameSetElement;
class KeyboardEvent;
class MouseEventWithHitTestResults;
class Node;
class PlatformScrollbar;
class PlatformWheelEvent;
class RenderLayer;
class RenderObject;
class RenderWidget;
class VisiblePosition;
class Widget;

struct HitTestRequest;

extern const float LinkDragHysteresis;
extern const float ImageDragHysteresis;
extern const float TextDragHysteresis;
extern const float GeneralDragHysteresis;
extern const double TextDragDelay;

class EventHandler : Noncopyable {
public:
    EventHandler(Frame*);
    ~EventHandler();

    void clear();

    void updateSelectionForMouseDragOverPosition(const VisiblePosition&);

    Node* mousePressNode() const;
    void setMousePressNode(PassRefPtr<Node>);

    void stopAutoscrollTimer(bool rendererIsBeingDestroyed = false);
    RenderObject* autoscrollRenderer() const;

    HitTestResult hitTestResultAtPoint(const IntPoint&, bool allowShadowContent);

    bool mousePressed() const { return m_mousePressed; }
    void setMousePressed(bool pressed) { m_mousePressed = pressed; }

    void setCapturingMouseEventsNode(PassRefPtr<Node>);

    bool updateDragAndDrop(const PlatformMouseEvent&, Clipboard*);
    void cancelDragAndDrop(const PlatformMouseEvent&, Clipboard*);
    bool performDragAndDrop(const PlatformMouseEvent&, Clipboard*);

    void scheduleHoverStateUpdate();

    void setResizingFrameSet(HTMLFrameSetElement*);

    IntPoint currentMousePosition() const;

    void setIgnoreWheelEvents(bool);

    bool scrollOverflow(ScrollDirection, ScrollGranularity);

    bool shouldDragAutoNode(Node*, const IntPoint&) const; // -webkit-user-drag == auto

    bool tabsToLinks(KeyboardEvent*) const;
    bool tabsToAllControls(KeyboardEvent*) const;

    bool mouseDownMayStartSelect() const { return m_mouseDownMayStartSelect; }
    bool inputManagerHasMarkedText() const;

    bool handleMousePressEvent(const PlatformMouseEvent&);
    bool handleMouseMoveEvent(const PlatformMouseEvent&);
    bool handleMouseReleaseEvent(const PlatformMouseEvent&);
    bool handleWheelEvent(PlatformWheelEvent&);

    bool sendContextMenuEvent(PlatformMouseEvent);

#if PLATFORM(MAC)
    PassRefPtr<KeyboardEvent> currentKeyboardEvent() const;

    void mouseDown(NSEvent*);
    void mouseDragged(NSEvent*);
    void mouseUp(NSEvent*);
    void mouseMoved(NSEvent*);
    bool keyEvent(NSEvent*);
    bool wheelEvent(NSEvent*);

    bool eventMayStartDrag(NSEvent*) const;

    void sendFakeEventsAfterWidgetTracking(NSEvent* initiatingEvent);

    void setActivationEventNumber(int num) { m_activationEventNumber = num; }

    void dragSourceMovedTo(const PlatformMouseEvent&);
    void dragSourceEndedAt(const PlatformMouseEvent&, NSDragOperation);

    NSEvent *currentNSEvent();

#endif

private:
    void selectClosestWordFromMouseEvent(const MouseEventWithHitTestResults& event);

    bool handleMouseDoubleClickEvent(const PlatformMouseEvent&);

    bool handleMousePressEvent(const MouseEventWithHitTestResults&);
    bool handleMousePressEventSingleClick(const MouseEventWithHitTestResults&);
    bool handleMousePressEventDoubleClick(const MouseEventWithHitTestResults&);
    bool handleMousePressEventTripleClick(const MouseEventWithHitTestResults&);
    bool handleMouseMoveEvent(const MouseEventWithHitTestResults&);
    bool handleMouseReleaseEvent(const MouseEventWithHitTestResults&);

    void hoverTimerFired(Timer<EventHandler>*);

    bool lastEventIsMouseUp() const;

    static bool canMouseDownStartSelect(Node*);

    void handleAutoscroll(RenderObject*);
    void startAutoscrollTimer();
    void setAutoscrollRenderer(RenderObject*);

    void autoscrollTimerFired(Timer<EventHandler>*);

    void invalidateClick();

    Node* nodeUnderMouse() const;

    MouseEventWithHitTestResults prepareMouseEvent(const HitTestRequest& hitTestRequest, const PlatformMouseEvent& mev);

    bool dispatchMouseEvent(const AtomicString& eventType, Node* target,
        bool cancelable, int clickCount, const PlatformMouseEvent&, bool setUnder);
    bool dispatchDragEvent(const AtomicString& eventType, Node* target,
        const PlatformMouseEvent&, Clipboard*);

    void freeClipboard();

    void focusDocumentView();

    bool handleDrag(const MouseEventWithHitTestResults&);
    bool handleMouseUp(const MouseEventWithHitTestResults&);

    bool dispatchDragSrcEvent(const AtomicString& eventType, const PlatformMouseEvent&);

    bool dragHysteresisExceeded(const FloatPoint&) const;
    bool dragHysteresisExceeded(const IntPoint&) const;

    bool passMousePressEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe);
    bool passMouseMoveEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe);
    bool passMouseReleaseEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe);
    bool passWheelEventToSubframe(PlatformWheelEvent&, Frame* subframe);

    bool passSubframeEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe);

    bool passMousePressEventToScrollbar(MouseEventWithHitTestResults&, PlatformScrollbar*);

    bool passWidgetMouseDownEventToWidget(const MouseEventWithHitTestResults&);
    bool passWidgetMouseDownEventToWidget(RenderWidget*);

    bool passMouseDownEventToWidget(Widget*);
    bool passWheelEventToWidget(Widget*);
    
#if PLATFORM(MAC)
    KeyboardUIMode keyboardUIMode() const;

    NSView *mouseDownViewIfStillGood();
#endif

    Frame* m_frame;

    bool m_mousePressed;
    RefPtr<Node> m_mousePressNode;

    bool m_beganSelectingText;

    IntPoint m_dragStartPos;

    Timer<EventHandler> m_hoverTimer;
    
    Timer<EventHandler> m_autoscrollTimer;
    RenderObject* m_autoscrollRenderer;
    bool m_mouseDownMayStartAutoscroll;
    bool m_mouseDownMayStartDrag;

    RenderLayer* m_resizeLayer;

    RefPtr<Node> m_capturingMouseEventsNode;
    
    RefPtr<Node> m_nodeUnderMouse;
    RefPtr<Node> m_lastNodeUnderMouse;
    RefPtr<Frame> m_lastMouseMoveEventSubframe;
    RefPtr<PlatformScrollbar> m_lastScrollbarUnderMouse;

    bool m_ignoreWheelEvents;

    int m_clickCount;
    RefPtr<Node> m_clickNode;

    RefPtr<Node> m_dragTarget;
    
    RefPtr<HTMLFrameSetElement> m_frameSetBeingResized;

    IntSize m_offsetFromResizeCorner;    
    
    IntPoint m_currentMousePosition;
    IntPoint m_mouseDownPos; // in our view's coords
    double m_mouseDownTimestamp;

    bool m_mouseDownWasInSubframe;
    bool m_mouseDownMayStartSelect;

#if PLATFORM(MAC)
    NSView *m_mouseDownView;
    bool m_sendingEventToSubview;
    PlatformMouseEvent m_mouseDown;
    int m_activationEventNumber;
#endif

};

} // namespace WebCore

#endif // EventHandler_h
