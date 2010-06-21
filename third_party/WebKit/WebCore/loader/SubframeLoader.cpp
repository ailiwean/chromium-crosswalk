/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
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

#include "config.h"
#include "SubframeLoader.h"

#include "Frame.h"
#include "FrameLoaderClient.h"
#include "HTMLAppletElement.h"
#include "HTMLFrameElementBase.h"
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
#include "HTMLMediaElement.h"
#endif
#include "HTMLNames.h"
#include "HTMLPlugInElement.h"
#include "MIMETypeRegistry.h"
#include "Node.h"
#include "Page.h"
#include "PluginData.h"
#include "RenderEmbeddedObject.h"
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
#include "RenderVideo.h"
#endif
#include "RenderView.h"
#include "Settings.h"
#include "XSSAuditor.h"

namespace WebCore {
    
using namespace HTMLNames;

SubframeLoader::SubframeLoader(Frame* frame)
    : m_frame(frame)
{
}

static HTMLPlugInElement* toPlugInElement(Node* node)
{
    if (!node)
        return 0;

    ASSERT(node->hasTagName(objectTag) || node->hasTagName(embedTag) || node->hasTagName(appletTag));

    return static_cast<HTMLPlugInElement*>(node);
}

void SubframeLoader::clear()
{
    m_containsPlugins = false;
}

bool SubframeLoader::requestFrame(HTMLFrameOwnerElement* ownerElement, const String& urlString, const AtomicString& frameName, bool lockHistory, bool lockBackForwardList)
{
    // Support for <frame src="javascript:string">
    KURL scriptURL;
    KURL url;
    if (protocolIsJavaScript(urlString)) {
        scriptURL = completeURL(urlString); // completeURL() encodes the URL.
        url = blankURL();
    } else
        url = completeURL(urlString);

    Frame* frame = ownerElement->contentFrame();
    if (frame)
        frame->redirectScheduler()->scheduleLocationChange(url.string(), m_frame->loader()->outgoingReferrer(), lockHistory, lockBackForwardList, m_frame->loader()->isProcessingUserGesture());
    else
        frame = loadSubframe(ownerElement, url, frameName, m_frame->loader()->outgoingReferrer());
    
    if (!frame)
        return false;

    if (!scriptURL.isEmpty())
        frame->script()->executeIfJavaScriptURL(scriptURL);

    return true;
}

bool SubframeLoader::requestObject(RenderEmbeddedObject* renderer, const String& url, const AtomicString& frameName,
    const String& mimeType, const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    if (url.isEmpty() && mimeType.isEmpty())
        return false;
    
    if (!m_frame->script()->xssAuditor()->canLoadObject(url)) {
        // It is unsafe to honor the request for this object.
        return false;
    }

    KURL completedURL;
    if (!url.isEmpty())
        completedURL = completeURL(url);

    bool useFallback;
    if (shouldUsePlugin(completedURL, mimeType, renderer->hasFallbackContent(), useFallback)) {
        Settings* settings = m_frame->settings();
        if ((!allowPlugins(AboutToInstantiatePlugin)
             // Application plugins are plugins implemented by the user agent, for example Qt plugins,
             // as opposed to third-party code such as flash. The user agent decides whether or not they are
             // permitted, rather than WebKit.
             && !MIMETypeRegistry::isApplicationPluginMIMEType(mimeType))
            || (!settings->isJavaEnabled() && MIMETypeRegistry::isJavaAppletMIMEType(mimeType)))
            return false;
        if (m_frame->document() && m_frame->document()->securityOrigin()->isSandboxed(SandboxPlugins))
            return false;
        return loadPlugin(renderer, completedURL, mimeType, paramNames, paramValues, useFallback);
    }

    ASSERT(renderer->node()->hasTagName(objectTag) || renderer->node()->hasTagName(embedTag));
    HTMLPlugInElement* element = static_cast<HTMLPlugInElement*>(renderer->node());

    // If the plug-in element already contains a subframe, requestFrame will re-use it. Otherwise,
    // it will create a new frame and set it as the RenderPart's widget, causing what was previously 
    // in the widget to be torn down.
    return requestFrame(element, completedURL, frameName);
}


#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
PassRefPtr<Widget> FrameLoader::loadMediaPlayerProxyPlugin(Node* node, const KURL& url, 
    const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    ASSERT(node->hasTagName(videoTag) || node->hasTagName(audioTag));

    if (!m_frame->script()->xssAuditor()->canLoadObject(url.string()))
        return 0;

    KURL completedURL;
    if (!url.isEmpty())
        completedURL = completeURL(url);

    if (!SecurityOrigin::canLoad(completedURL, String(), frame()->document())) {
        FrameLoader::reportLocalLoadFailed(m_frame, completedURL.string());
        return 0;
    }

    HTMLMediaElement* mediaElement = static_cast<HTMLMediaElement*>(node);
    RenderPart* renderer = toRenderPart(node->renderer());
    IntSize size;

    if (renderer)
        size = IntSize(renderer->contentWidth(), renderer->contentHeight());
    else if (mediaElement->isVideo())
        size = RenderVideo::defaultSize();

    m_frame->loader()->checkIfRunInsecureContent(m_frame->document()->securityOrigin(), completedURL);

    RefPtr<Widget> widget = m_frame->loader()->client()->createMediaPlayerProxyPlugin(size, mediaElement, completedURL,
                                         paramNames, paramValues, "application/x-media-element-proxy-plugin");

    if (widget && renderer) {
        renderer->setWidget(widget);
        m_containsPlugIns = true;
        renderer->node()->setNeedsStyleRecalc(SyntheticStyleChange);
    }

    return widget ? widget.release() : 0;
}
#endif // ENABLE(PLUGIN_PROXY_FOR_VIDEO)

PassRefPtr<Widget> SubframeLoader::createJavaAppletWidget(const IntSize& size, HTMLAppletElement* element, const HashMap<String, String>& args)
{
    String baseURLString;
    String codeBaseURLString;
    Vector<String> paramNames;
    Vector<String> paramValues;
    HashMap<String, String>::const_iterator end = args.end();
    for (HashMap<String, String>::const_iterator it = args.begin(); it != end; ++it) {
        if (equalIgnoringCase(it->first, "baseurl"))
            baseURLString = it->second;
        else if (equalIgnoringCase(it->first, "codebase"))
            codeBaseURLString = it->second;
        paramNames.append(it->first);
        paramValues.append(it->second);
    }

    if (!codeBaseURLString.isEmpty()) {
        KURL codeBaseURL = completeURL(codeBaseURLString);
        if (!SecurityOrigin::canLoad(codeBaseURL, String(), element->document())) {
            FrameLoader::reportLocalLoadFailed(m_frame, codeBaseURL.string());
            return 0;
        }
    }

    if (baseURLString.isEmpty())
        baseURLString = m_frame->document()->baseURL().string();
    KURL baseURL = completeURL(baseURLString);

    RefPtr<Widget> widget = m_frame->loader()->client()->createJavaAppletWidget(size, element, baseURL, paramNames, paramValues);
    if (!widget)
        return 0;

    m_containsPlugins = true;
    return widget;
}

Frame* SubframeLoader::loadSubframe(HTMLFrameOwnerElement* ownerElement, const KURL& url, const String& name, const String& referrer)
{
    bool allowsScrolling = true;
    int marginWidth = -1;
    int marginHeight = -1;
    if (ownerElement->hasTagName(frameTag) || ownerElement->hasTagName(iframeTag)) {
        HTMLFrameElementBase* o = static_cast<HTMLFrameElementBase*>(ownerElement);
        allowsScrolling = o->scrollingMode() != ScrollbarAlwaysOff;
        marginWidth = o->getMarginWidth();
        marginHeight = o->getMarginHeight();
    }

    if (!SecurityOrigin::canLoad(url, referrer, 0)) {
        FrameLoader::reportLocalLoadFailed(m_frame, url.string());
        return 0;
    }

    bool hideReferrer = SecurityOrigin::shouldHideReferrer(url, referrer);
    RefPtr<Frame> frame = m_frame->loader()->client()->createFrame(url, name, ownerElement, hideReferrer ? String() : referrer, allowsScrolling, marginWidth, marginHeight);

    if (!frame)  {
        m_frame->loader()->checkCallImplicitClose();
        return 0;
    }
    
    // All new frames will have m_isComplete set to true at this point due to synchronously loading
    // an empty document in FrameLoader::init(). But many frames will now be starting an
    // asynchronous load of url, so we set m_isComplete to false and then check if the load is
    // actually completed below. (Note that we set m_isComplete to false even for synchronous
    // loads, so that checkCompleted() below won't bail early.)
    // FIXME: Can we remove this entirely? m_isComplete normally gets set to false when a load is committed.
    frame->loader()->started();
   
    RenderObject* renderer = ownerElement->renderer();
    FrameView* view = frame->view();
    if (renderer && renderer->isWidget() && view)
        toRenderWidget(renderer)->setWidget(view);
    
    m_frame->loader()->checkCallImplicitClose();
    
    // Some loads are performed synchronously (e.g., about:blank and loads
    // cancelled by returning a null ResourceRequest from requestFromDelegate).
    // In these cases, the synchronous load would have finished
    // before we could connect the signals, so make sure to send the 
    // completed() signal for the child by hand and mark the load as being
    // complete.
    // FIXME: In this case the Frame will have finished loading before 
    // it's being added to the child list. It would be a good idea to
    // create the child first, then invoke the loader separately.
    if (frame->loader()->state() == FrameStateComplete)
        frame->loader()->checkCompleted();

    return frame.get();
}

bool SubframeLoader::allowPlugins(ReasonForCallingAllowPlugins reason)
{
    Settings* settings = m_frame->settings();
    bool allowed = m_frame->loader()->client()->allowPlugins(settings && settings->arePluginsEnabled());
    if (!allowed && reason == AboutToInstantiatePlugin)
        m_frame->loader()->client()->didNotAllowPlugins();
    return allowed;
}


bool SubframeLoader::shouldUsePlugin(const KURL& url, const String& mimeType, bool hasFallback, bool& useFallback)
{
    if (m_frame->loader()->client()->shouldUsePluginDocument(mimeType)) {
        useFallback = false;
        return true;
    }

    // Allow other plug-ins to win over QuickTime because if the user has installed a plug-in that
    // can handle TIFF (which QuickTime can also handle) they probably intended to override QT.
    if (m_frame->page() && (mimeType == "image/tiff" || mimeType == "image/tif" || mimeType == "image/x-tiff")) {
        const PluginData* pluginData = m_frame->page()->pluginData();
        String pluginName = pluginData ? pluginData->pluginNameForMimeType(mimeType) : String();
        if (!pluginName.isEmpty() && !pluginName.contains("QuickTime", false)) 
            return true;
    }
        
    ObjectContentType objectType = m_frame->loader()->client()->objectContentType(url, mimeType);
    // If an object's content can't be handled and it has no fallback, let
    // it be handled as a plugin to show the broken plugin icon.
    useFallback = objectType == ObjectContentNone && hasFallback;
    return objectType == ObjectContentNone || objectType == ObjectContentNetscapePlugin || objectType == ObjectContentOtherPlugin;
}
  
bool SubframeLoader::loadPlugin(RenderEmbeddedObject* renderer, const KURL& url, const String& mimeType, 
    const Vector<String>& paramNames, const Vector<String>& paramValues, bool useFallback)
{
    RefPtr<Widget> widget;

    if (renderer && !useFallback) {
        HTMLPlugInElement* element = toPlugInElement(renderer->node());

        if (!SecurityOrigin::canLoad(url, String(), m_frame->document())) {
            FrameLoader::reportLocalLoadFailed(m_frame, url.string());
            return false;
        }

        m_frame->loader()->checkIfRunInsecureContent(m_frame->document()->securityOrigin(), url);

        widget = m_frame->loader()->client()->createPlugin(IntSize(renderer->contentWidth(), renderer->contentHeight()),
                                        element, url, paramNames, paramValues, mimeType,
                                        m_frame->document()->isPluginDocument() && !m_containsPlugins);
        if (widget) {
            renderer->setWidget(widget);
            m_containsPlugins = true;

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
            renderer->node()->setNeedsStyleRecalc(SyntheticStyleChange);
#endif
        } else
            renderer->setShowsMissingPluginIndicator();
    }

    return widget;
}

KURL SubframeLoader::completeURL(const String& url) const
{
    ASSERT(m_frame->document());
    return m_frame->document()->completeURL(url);
}

} // namespace WebCore
