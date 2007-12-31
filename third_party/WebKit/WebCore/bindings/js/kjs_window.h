/*
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reseved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef kjs_window_h
#define kjs_window_h

#include "PlatformString.h"
#include "kjs_binding.h"
#include <kjs/protect.h>
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/OwnPtr.h>

namespace WebCore {
    class AtomicString;
    class DOMWindow;
    class Frame;
    class JSEventListener;
    class JSLocation;
    class JSUnprotectedEventListener;
    class Location;
    class PausedTimeouts;
    class ScheduledAction;
}

namespace KJS {

    class DOMWindowTimer;
    class Window;
    class WindowFunc;
    class WindowPrivate;

  // This is the only WebCore JS binding which does not inherit from DOMObject
  class Window : public JSGlobalObject {
    typedef JSGlobalObject Base;
    
    friend class WebCore::JSLocation;
    friend class WebCore::ScheduledAction;
  protected:
    Window(WebCore::DOMWindow*, JSValue* prototype);

  public:
    virtual ~Window();

    WebCore::DOMWindow* impl() const { return m_impl.get(); }

    void disconnectFrame();

    // Returns and registers a window object. In case there's already a Window
    // for the specified frame p this will be returned in order to have unique
    // bindings.
    static JSValue* retrieve(WebCore::Frame*);

    // Returns the Window object for a given HTML frame
    static Window* retrieveWindow(WebCore::Frame*);

    // Returns a pointer to the Window object this javascript interpreting instance 
    // was called from.
    static Window* retrieveActive(ExecState*);

    virtual void mark();

    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    JSValue* getValueProperty(ExecState*, int token) const;
    virtual void put(ExecState*, const Identifier& propertyName, JSValue*, int attr = None);

    int installTimeout(const UString& handler, int t, bool singleShot);
    int installTimeout(JSValue* function, const List& args, int t, bool singleShot);
    void clearTimeout(int timerId, bool delAction = true);
    WebCore::PausedTimeouts* pauseTimeouts();
    void resumeTimeouts(WebCore::PausedTimeouts*);

    void timerFired(DOMWindowTimer*);

    WebCore::JSLocation* location() const;

    // Finds a wrapper of a JS EventListener, returns 0 if no existing one.
    WebCore::JSEventListener* findJSEventListener(JSValue*, bool html = false);

    // Finds or creates a wrapper of a JS EventListener. JS EventListener object is GC-protected.
    WebCore::JSEventListener *findOrCreateJSEventListener(JSValue*, bool html = false);

    // Finds a wrapper of a GC-unprotected JS EventListener, returns 0 if no existing one.
    WebCore::JSUnprotectedEventListener* findJSUnprotectedEventListener(JSValue*, bool html = false);

    // Finds or creates a wrapper of a JS EventListener. JS EventListener object is *NOT* GC-protected.
    WebCore::JSUnprotectedEventListener *findOrCreateJSUnprotectedEventListener(JSValue*, bool html = false);

    void clear();

    void setCurrentEvent(WebCore::Event*);
    WebCore::Event* currentEvent();

    // Set a place to put a dialog return value when the window is cleared.
    void setReturnValueSlot(JSValue** slot);

    typedef HashMap<JSObject*, WebCore::JSEventListener*> ListenersMap;
    typedef HashMap<JSObject*, WebCore::JSUnprotectedEventListener*> UnprotectedListenersMap;
    
    ListenersMap& jsEventListeners();
    ListenersMap& jsHTMLEventListeners();
    UnprotectedListenersMap& jsUnprotectedEventListeners();
    UnprotectedListenersMap& jsUnprotectedHTMLEventListeners();
    
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual ExecState* globalExec();

    virtual bool shouldInterruptScript() const;

    virtual bool allowsAccessFrom(const JSGlobalObject*) const;
    bool allowsAccessFrom(ExecState* exec) const { return allowsAccessFrom(exec->dynamicGlobalObject()); }

    virtual void getPropertyNames(ExecState*, PropertyNameArray&);

    enum {
        // Attributes
        Crypto, Event_, Location_, Navigator_,
        ClientInformation,

        // Event Listeners
        Onabort, Onblur, Onchange, Onclick,
        Ondblclick, Onerror, Onfocus, Onkeydown,
        Onkeypress, Onkeyup, Onload, Onmousedown,
        Onmousemove, Onmouseout, Onmouseover, Onmouseup,
        OnWindowMouseWheel, Onreset, Onresize, Onscroll,
        Onsearch, Onselect, Onsubmit, Onunload,
        Onbeforeunload,

        // Constructors
        DOMException, Audio, Image, Option, XMLHttpRequest,
        XSLTProcessor_
    };

  private:
    JSValue* getListener(ExecState*, const WebCore::AtomicString& eventType) const;
    void setListener(ExecState*, const WebCore::AtomicString& eventType, JSValue* func);

    static JSValue* childFrameGetter(ExecState*, JSObject*, const Identifier&, const PropertySlot&);
    static JSValue* namedFrameGetter(ExecState*, JSObject*, const Identifier&, const PropertySlot&);
    static JSValue* indexGetter(ExecState*, JSObject*, const Identifier&, const PropertySlot&);
    static JSValue* namedItemGetter(ExecState*, JSObject*, const Identifier&, const PropertySlot&);

    void clearHelperObjectProperties();
    void clearAllTimeouts();
    int installTimeout(WebCore::ScheduledAction*, int interval, bool singleShot);

    RefPtr<WebCore::DOMWindow> m_impl;
    OwnPtr<WindowPrivate> d;
  };

#define FOR_EACH_CLASS(macro) \
    macro(WindowProtoFuncAToB) \
    macro(WindowProtoFuncBToA) \
    macro(WindowProtoFuncOpen) \
    macro(WindowProtoFuncSetTimeout) \
    macro(WindowProtoFuncClearTimeout) \
    macro(WindowProtoFuncSetInterval) \
    macro(WindowProtoFuncAddEventListener) \
    macro(WindowProtoFuncRemoveEventListener) \
    macro(WindowProtoFuncShowModalDialog) \
    macro(WindowProtoFuncNotImplemented) \

FOR_EACH_CLASS(KJS_IMPLEMENT_PROTOTYPE_FUNCTION_WITH_CREATE)
#undef FOR_EACH_CLASS

} // namespace KJS

namespace WebCore {
    KJS::JSValue* toJS(KJS::ExecState*, DOMWindow*);
} // namespace WebCore

#endif // kjs_window_h
