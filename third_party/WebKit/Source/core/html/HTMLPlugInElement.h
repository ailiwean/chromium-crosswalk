/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2006, 2007, 2008, 2009, 2012 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef HTMLPlugInElement_h
#define HTMLPlugInElement_h

#include "bindings/v8/SharedPersistent.h"
#include "core/html/HTMLFrameOwnerElement.h"

struct NPObject;

namespace WebCore {

class HTMLImageLoader;
class RenderEmbeddedObject;
class RenderWidget;
class Widget;

enum PreferPlugInsForImagesOption {
    ShouldPreferPlugInsForImages,
    ShouldNotPreferPlugInsForImages
};

enum PluginCreationOption {
    CreateAnyWidgetType,
    CreateOnlyNonNetscapePlugins,
};

class HTMLPlugInElement : public HTMLFrameOwnerElement {
public:
    virtual ~HTMLPlugInElement();

    void resetInstance();

    SharedPersistent<v8::Object>* pluginWrapper();

    Widget* pluginWidget() const;

    enum DisplayState {
        Restarting,
        RestartingWithPendingMouseClick,
        Playing
    };
    DisplayState displayState() const { return m_displayState; }
    void setDisplayState(DisplayState state) { m_displayState = state; }

    NPObject* getNPObject();

    bool isCapturingMouseEvents() const { return m_isCapturingMouseEvents; }
    void setIsCapturingMouseEvents(bool capturing) { m_isCapturingMouseEvents = capturing; }

    bool canContainRangeEndPoint() const { return false; }

    bool canProcessDrag() const;

    virtual bool willRespondToMouseClickEvents() OVERRIDE;
    virtual void removeAllEventListeners() OVERRIDE FINAL;

    const String& serviceType() const { return m_serviceType; }
    const String& url() const { return m_url; }
    const KURL& loadedUrl() const { return m_loadedUrl; }
    const String loadedMimeType() const;

    // Public for FrameView::addWidgetToUpdate()
    bool needsWidgetUpdate() const { return m_needsWidgetUpdate; }
    void setNeedsWidgetUpdate(bool needsWidgetUpdate) { m_needsWidgetUpdate = needsWidgetUpdate; }

    virtual void updateWidget(PluginCreationOption) = 0;

protected:
    HTMLPlugInElement(const QualifiedName& tagName, Document&, bool createdByParser, PreferPlugInsForImagesOption);

    virtual void didMoveToNewDocument(Document& oldDocument) OVERRIDE;
    virtual bool isPresentationAttribute(const QualifiedName&) const OVERRIDE;
    virtual void collectStyleForPresentationAttribute(const QualifiedName&, const AtomicString&, MutableStylePropertySet*) OVERRIDE;

    virtual bool useFallbackContent() const { return false; }

    virtual bool dispatchBeforeLoadEvent(const String& sourceURL) OVERRIDE;

    // Create or update the RenderWidget and return it, triggering layout if necessary.
    virtual RenderWidget* renderWidgetForJSBindings() const;

    bool isImageType();
    bool shouldPreferPlugInsForImages() const { return m_shouldPreferPlugInsForImages; }
    RenderEmbeddedObject* renderEmbeddedObject() const;
    bool allowedToLoadFrameURL(const String& url);
    bool wouldLoadAsNetscapePlugin(const String& url, const String& serviceType);
    bool requestObject(const String& url, const String& mimeType, const Vector<String>& paramNames, const Vector<String>& paramValues);
    bool shouldUsePlugin(const KURL&, const String& mimeType, bool hasFallback, bool& useFallback);

    String m_serviceType;
    String m_url;
    KURL m_loadedUrl;
    OwnPtr<HTMLImageLoader> m_imageLoader;

private:
    virtual bool areAuthorShadowsAllowed() const OVERRIDE { return false; }
    virtual void attach(const AttachContext& = AttachContext()) OVERRIDE;
    virtual void detach(const AttachContext& = AttachContext()) OVERRIDE;
    virtual void defaultEventHandler(Event*) OVERRIDE;
    virtual RenderObject* createRenderer(RenderStyle*) OVERRIDE;
    virtual void willRecalcStyle(StyleRecalcChange) OVERRIDE FINAL;
    virtual void finishParsingChildren() OVERRIDE;

    // Return any existing RenderWidget without triggering relayout, or 0 if it doesn't yet exist.
    virtual RenderWidget* existingRenderWidget() const = 0;

    virtual bool supportsFocus() const OVERRIDE { return true; };
    virtual bool rendererIsFocusable() const OVERRIDE;

    virtual bool isKeyboardFocusable() const OVERRIDE;
    virtual bool isPluginElement() const;
    static void updateWidgetCallback(Node*);
    void updateWidgetIfNecessary();
    bool loadPlugin(const KURL&, const String& mimeType, const Vector<String>& paramNames, const Vector<String>& paramValues, bool useFallback);
    bool pluginIsLoadable(const KURL&, const String& mimeType);

    mutable RefPtr<SharedPersistent<v8::Object> > m_pluginWrapper;
    NPObject* m_NPObject;
    bool m_isCapturingMouseEvents;
    bool m_inBeforeLoadEventHandler;
    bool m_needsWidgetUpdate;
    bool m_shouldPreferPlugInsForImages;
    DisplayState m_displayState;
};

DEFINE_NODE_TYPE_CASTS(HTMLPlugInElement, isPluginElement());

} // namespace WebCore

#endif // HTMLPlugInElement_h
