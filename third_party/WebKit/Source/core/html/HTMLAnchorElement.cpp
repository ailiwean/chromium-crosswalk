/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2003, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *           (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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
 */

#include "config.h"
#include "core/html/HTMLAnchorElement.h"

#include "public/platform/Platform.h"
#include "public/platform/WebPrescientNetworking.h"
#include "public/platform/WebURL.h"
#include <wtf/text/StringBuilder.h>
#include "HTMLNames.h"
#include "core/dom/Attribute.h"
#include "core/dom/EventNames.h"
#include "core/dom/KeyboardEvent.h"
#include "core/dom/MouseEvent.h"
#include "core/editing/FrameSelection.h"
#include "core/html/HTMLFormElement.h"
#include "core/html/HTMLImageElement.h"
#include "core/html/parser/HTMLParserIdioms.h"
#include "core/loader/FrameLoadRequest.h"
#include "core/loader/FrameLoader.h"
#include "core/loader/FrameLoaderClient.h"
#include "core/loader/FrameLoaderTypes.h"
#include "core/loader/PingLoader.h"
#include "core/page/Chrome.h"
#include "core/page/ChromeClient.h"
#include "core/page/Frame.h"
#include "core/page/Page.h"
#include "core/page/Settings.h"
#include "core/platform/HistogramSupport.h"
#include "core/platform/PlatformMouseEvent.h"
#include "core/platform/network/DNS.h"
#include "core/platform/network/ResourceRequest.h"
#include "core/rendering/RenderImage.h"
#include "weborigin/KnownPorts.h"
#include "weborigin/SecurityOrigin.h"
#include "weborigin/SecurityPolicy.h"

namespace WebCore {

namespace {

void preconnectToURL(const KURL& url, WebKit::WebPreconnectMotivation motivation)
{
    WebKit::WebPrescientNetworking* prescientNetworking = WebKit::Platform::current()->prescientNetworking();
    if (!prescientNetworking)
        return;

    prescientNetworking->preconnect(url, motivation);
}

}

class HTMLAnchorElement::PrefetchEventHandler {
public:
    static PassOwnPtr<PrefetchEventHandler> create(HTMLAnchorElement* anchorElement)
    {
        return adoptPtr(new HTMLAnchorElement::PrefetchEventHandler(anchorElement));
    }

    void handleEvent(Event* e);
    void didChangeHREF() { m_hadHREFChanged = true; }

private:
    explicit PrefetchEventHandler(HTMLAnchorElement*);

    void reset();

    void handleMouseOver(Event* event);
    void handleMouseOut(Event* event);
    void handleLeftMouseDown(Event* event);
    void handleGestureTapUnconfirmed(Event*);
    void handleGestureTapDown(Event*);
    void handleClick(Event* event);

    bool shouldPrefetch(const KURL&);
    void prefetch(WebKit::WebPreconnectMotivation);

    HTMLAnchorElement* m_anchorElement;
    double m_mouseOverTimestamp;
    double m_mouseDownTimestamp;
    double m_tapDownTimestamp;
    bool m_hadHREFChanged;
    bool m_hadTapUnconfirmed;
    bool m_hasIssuedPreconnect;
};

using namespace HTMLNames;

HTMLAnchorElement::HTMLAnchorElement(const QualifiedName& tagName, Document* document)
    : HTMLElement(tagName, document)
    , m_hasRootEditableElementForSelectionOnMouseDown(false)
    , m_wasShiftKeyDownOnMouseDown(false)
    , m_linkRelations(0)
    , m_cachedVisitedLinkHash(0)
{
    ScriptWrappable::init(this);
}

PassRefPtr<HTMLAnchorElement> HTMLAnchorElement::create(Document* document)
{
    return adoptRef(new HTMLAnchorElement(aTag, document));
}

PassRefPtr<HTMLAnchorElement> HTMLAnchorElement::create(const QualifiedName& tagName, Document* document)
{
    return adoptRef(new HTMLAnchorElement(tagName, document));
}

HTMLAnchorElement::~HTMLAnchorElement()
{
    clearRootEditableElementForSelectionOnMouseDown();
}

// This function does not allow leading spaces before the port number.
static unsigned parsePortFromStringPosition(const String& value, unsigned portStart, unsigned& portEnd)
{
    portEnd = portStart;
    while (isASCIIDigit(value[portEnd]))
        ++portEnd;
    return value.substring(portStart, portEnd - portStart).toUInt();
}

bool HTMLAnchorElement::supportsFocus() const
{
    if (rendererIsEditable())
        return HTMLElement::supportsFocus();
    // If not a link we should still be able to focus the element if it has tabIndex.
    return isLink() || HTMLElement::supportsFocus();
}

bool HTMLAnchorElement::isMouseFocusable() const
{
    // Links are focusable by default, but only allow links with tabindex or contenteditable to be mouse focusable.
    // https://bugs.webkit.org/show_bug.cgi?id=26856
    if (isLink())
        return HTMLElement::supportsFocus();

    return HTMLElement::isMouseFocusable();
}

bool HTMLAnchorElement::isKeyboardFocusable(KeyboardEvent* event) const
{
    if (!isLink())
        return HTMLElement::isKeyboardFocusable(event);

    if (!isFocusable())
        return false;
    
    Page* page = document()->page();
    if (!page)
        return false;

    if (!page->chrome().client()->tabsToLinks())
        return false;

    if (isInCanvasSubtree())
        return true;

    return hasNonEmptyBoundingBox();
}

static void appendServerMapMousePosition(StringBuilder& url, Event* event)
{
    if (!event->isMouseEvent())
        return;

    ASSERT(event->target());
    Node* target = event->target()->toNode();
    ASSERT(target);
    if (!target->hasTagName(imgTag))
        return;

    HTMLImageElement* imageElement = toHTMLImageElement(event->target()->toNode());
    if (!imageElement || !imageElement->isServerMap())
        return;

    if (!imageElement->renderer() || !imageElement->renderer()->isRenderImage())
        return;
    RenderImage* renderer = toRenderImage(imageElement->renderer());

    // FIXME: This should probably pass true for useTransforms.
    FloatPoint absolutePosition = renderer->absoluteToLocal(FloatPoint(toMouseEvent(event)->pageX(), toMouseEvent(event)->pageY()));
    int x = absolutePosition.x();
    int y = absolutePosition.y();
    url.append('?');
    url.appendNumber(x);
    url.append(',');
    url.appendNumber(y);
}

void HTMLAnchorElement::defaultEventHandler(Event* event)
{
    if (isLink()) {
        if (focused() && isEnterKeyKeydownEvent(event) && treatLinkAsLiveForEventType(NonMouseEvent)) {
            event->setDefaultHandled();
            dispatchSimulatedClick(event);
            return;
        }

        prefetchEventHandler()->handleEvent(event);

        if (isLinkClick(event) && treatLinkAsLiveForEventType(eventType(event))) {
            handleClick(event);
            return;
        }

        if (rendererIsEditable()) {
            // This keeps track of the editable block that the selection was in (if it was in one) just before the link was clicked
            // for the LiveWhenNotFocused editable link behavior
            if (event->type() == eventNames().mousedownEvent && event->isMouseEvent() && toMouseEvent(event)->button() != RightButton && document()->frame() && document()->frame()->selection()) {
                setRootEditableElementForSelectionOnMouseDown(document()->frame()->selection()->rootEditableElement());
                m_wasShiftKeyDownOnMouseDown = toMouseEvent(event)->shiftKey();
            } else if (event->type() == eventNames().mouseoverEvent) {
                // These are cleared on mouseover and not mouseout because their values are needed for drag events,
                // but drag events happen after mouse out events.
                clearRootEditableElementForSelectionOnMouseDown();
                m_wasShiftKeyDownOnMouseDown = false;
            }
        }
    }

    HTMLElement::defaultEventHandler(event);
}

void HTMLAnchorElement::setActive(bool down, bool pause)
{
    if (rendererIsEditable()) {
        EditableLinkBehavior editableLinkBehavior = EditableLinkDefaultBehavior;
        if (Settings* settings = document()->settings())
            editableLinkBehavior = settings->editableLinkBehavior();
            
        switch (editableLinkBehavior) {
            default:
            case EditableLinkDefaultBehavior:
            case EditableLinkAlwaysLive:
                break;

            case EditableLinkNeverLive:
                return;

            // Don't set the link to be active if the current selection is in the same editable block as
            // this link
            case EditableLinkLiveWhenNotFocused:
                if (down && document()->frame() && document()->frame()->selection()->rootEditableElement() == rootEditableElement())
                    return;
                break;
            
            case EditableLinkOnlyLiveWithShiftKey:
                return;
        }

    }
    
    ContainerNode::setActive(down, pause);
}

void HTMLAnchorElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
{
    if (name == hrefAttr) {
        bool wasLink = isLink();
        setIsLink(!value.isNull());
        if (wasLink != isLink())
            didAffectSelector(AffectedSelectorLink | AffectedSelectorVisited | AffectedSelectorEnabled);
        if (isLink()) {
            String parsedURL = stripLeadingAndTrailingHTMLSpaces(value);
            if (document()->isDNSPrefetchEnabled()) {
                if (protocolIs(parsedURL, "http") || protocolIs(parsedURL, "https") || parsedURL.startsWith("//"))
                    prefetchDNS(document()->completeURL(parsedURL).host());
            }

            if (wasLink)
                prefetchEventHandler()->didChangeHREF();
        }
        invalidateCachedVisitedLinkHash();
    } else if (name == nameAttr || name == titleAttr) {
        // Do nothing.
    } else if (name == relAttr)
        setRel(value);
    else
        HTMLElement::parseAttribute(name, value);
}

void HTMLAnchorElement::accessKeyAction(bool sendMouseEvents)
{
    dispatchSimulatedClick(0, sendMouseEvents ? SendMouseUpDownEvents : SendNoEvents);
}

bool HTMLAnchorElement::isURLAttribute(const Attribute& attribute) const
{
    return attribute.name().localName() == hrefAttr || HTMLElement::isURLAttribute(attribute);
}

bool HTMLAnchorElement::canStartSelection() const
{
    // FIXME: We probably want this same behavior in SVGAElement too
    if (!isLink())
        return HTMLElement::canStartSelection();
    return rendererIsEditable();
}

bool HTMLAnchorElement::draggable() const
{
    // Should be draggable if we have an href attribute.
    const AtomicString& value = getAttribute(draggableAttr);
    if (equalIgnoringCase(value, "true"))
        return true;
    if (equalIgnoringCase(value, "false"))
        return false;
    return hasAttribute(hrefAttr);
}

KURL HTMLAnchorElement::href() const
{
    return document()->completeURL(stripLeadingAndTrailingHTMLSpaces(getAttribute(hrefAttr)));
}

void HTMLAnchorElement::setHref(const AtomicString& value)
{
    setAttribute(hrefAttr, value);
}

bool HTMLAnchorElement::hasRel(uint32_t relation) const
{
    return m_linkRelations & relation;
}

void HTMLAnchorElement::setRel(const String& value)
{
    m_linkRelations = 0;
    SpaceSplitString newLinkRelations(value, true);
    // FIXME: Add link relations as they are implemented
    if (newLinkRelations.contains("noreferrer"))
        m_linkRelations |= RelationNoReferrer;
}

const AtomicString& HTMLAnchorElement::name() const
{
    return getNameAttribute();
}

short HTMLAnchorElement::tabIndex() const
{
    // Skip the supportsFocus check in HTMLElement.
    return Element::tabIndex();
}

String HTMLAnchorElement::target() const
{
    return getAttribute(targetAttr);
}

String HTMLAnchorElement::hash() const
{
    String fragmentIdentifier = href().fragmentIdentifier();
    return fragmentIdentifier.isEmpty() ? emptyString() : "#" + fragmentIdentifier;
}

void HTMLAnchorElement::setHash(const String& value)
{
    KURL url = href();
    if (value[0] == '#')
        url.setFragmentIdentifier(value.substring(1));
    else
        url.setFragmentIdentifier(value);
    setHref(url.string());
}

String HTMLAnchorElement::host() const
{
    const KURL& url = href();
    if (url.hostEnd() == url.pathStart())
        return url.host();
    if (isDefaultPortForProtocol(url.port(), url.protocol()))
        return url.host();
    return url.host() + ":" + String::number(url.port());
}

void HTMLAnchorElement::setHost(const String& value)
{
    if (value.isEmpty())
        return;
    KURL url = href();
    if (!url.canSetHostOrPort())
        return;

    size_t separator = value.find(':');
    if (!separator)
        return;

    if (separator == notFound)
        url.setHostAndPort(value);
    else {
        unsigned portEnd;
        unsigned port = parsePortFromStringPosition(value, separator + 1, portEnd);
        if (!port) {
            // http://dev.w3.org/html5/spec/infrastructure.html#url-decomposition-idl-attributes
            // specifically goes against RFC 3986 (p3.2) and
            // requires setting the port to "0" if it is set to empty string.
            url.setHostAndPort(value.substring(0, separator + 1) + "0");
        } else {
            if (isDefaultPortForProtocol(port, url.protocol()))
                url.setHostAndPort(value.substring(0, separator));
            else
                url.setHostAndPort(value.substring(0, portEnd));
        }
    }
    setHref(url.string());
}

String HTMLAnchorElement::hostname() const
{
    return href().host();
}

void HTMLAnchorElement::setHostname(const String& value)
{
    // Before setting new value:
    // Remove all leading U+002F SOLIDUS ("/") characters.
    unsigned i = 0;
    unsigned hostLength = value.length();
    while (value[i] == '/')
        i++;

    if (i == hostLength)
        return;

    KURL url = href();
    if (!url.canSetHostOrPort())
        return;

    url.setHost(value.substring(i));
    setHref(url.string());
}

String HTMLAnchorElement::pathname() const
{
    return href().path();
}

void HTMLAnchorElement::setPathname(const String& value)
{
    KURL url = href();
    if (!url.canSetPathname())
        return;

    if (value[0] == '/')
        url.setPath(value);
    else
        url.setPath("/" + value);

    setHref(url.string());
}

String HTMLAnchorElement::port() const
{
    if (href().hasPort())
        return String::number(href().port());

    return emptyString();
}

void HTMLAnchorElement::setPort(const String& value)
{
    KURL url = href();
    if (!url.canSetHostOrPort())
        return;

    // http://dev.w3.org/html5/spec/infrastructure.html#url-decomposition-idl-attributes
    // specifically goes against RFC 3986 (p3.2) and
    // requires setting the port to "0" if it is set to empty string.
    unsigned port = value.toUInt();
    if (isDefaultPortForProtocol(port, url.protocol()))
        url.removePort();
    else
        url.setPort(port);

    setHref(url.string());
}

String HTMLAnchorElement::protocol() const
{
    return href().protocol() + ":";
}

void HTMLAnchorElement::setProtocol(const String& value)
{
    KURL url = href();
    url.setProtocol(value);
    setHref(url.string());
}

String HTMLAnchorElement::search() const
{
    String query = href().query();
    return query.isEmpty() ? emptyString() : "?" + query;
}

String HTMLAnchorElement::origin() const
{
    RefPtr<SecurityOrigin> origin = SecurityOrigin::create(href());
    return origin->toString();
}

void HTMLAnchorElement::setSearch(const String& value)
{
    KURL url = href();
    String newSearch = (value[0] == '?') ? value.substring(1) : value;
    // Make sure that '#' in the query does not leak to the hash.
    url.setQuery(newSearch.replaceWithLiteral('#', "%23"));

    setHref(url.string());
}

String HTMLAnchorElement::text()
{
    return innerText();
}

String HTMLAnchorElement::toString() const
{
    return href().string();
}

bool HTMLAnchorElement::isLiveLink() const
{
    return isLink() && treatLinkAsLiveForEventType(m_wasShiftKeyDownOnMouseDown ? MouseEventWithShiftKey : MouseEventWithoutShiftKey);
}

void HTMLAnchorElement::sendPings(const KURL& destinationURL)
{
    if (!hasAttribute(pingAttr) || !document()->settings()->hyperlinkAuditingEnabled())
        return;

    SpaceSplitString pingURLs(getAttribute(pingAttr), false);
    for (unsigned i = 0; i < pingURLs.size(); i++)
        PingLoader::sendPing(document()->frame(), document()->completeURL(pingURLs[i]), destinationURL);
}

void HTMLAnchorElement::handleClick(Event* event)
{
    event->setDefaultHandled();

    Frame* frame = document()->frame();
    if (!frame)
        return;

    StringBuilder url;
    url.append(stripLeadingAndTrailingHTMLSpaces(fastGetAttribute(hrefAttr)));
    appendServerMapMousePosition(url, event);
    KURL completedURL = document()->completeURL(url.toString());

    if (hasAttribute(downloadAttr)) {
        ResourceRequest request(completedURL);

        if (!hasRel(RelationNoReferrer)) {
            String referrer = SecurityPolicy::generateReferrerHeader(document()->referrerPolicy(), completedURL, frame->loader()->outgoingReferrer());
            if (!referrer.isEmpty())
                request.setHTTPReferrer(referrer);
        }

        frame->loader()->client()->loadURLExternally(request, NavigationPolicyDownload, fastGetAttribute(downloadAttr));
    } else {
        FrameLoadRequest frameRequest(document()->securityOrigin(), ResourceRequest(completedURL), target());
        frame->loader()->loadFrameRequest(frameRequest, false, event, 0, hasRel(RelationNoReferrer) ? NeverSendReferrer : MaybeSendReferrer);
    }

    sendPings(completedURL);
}

HTMLAnchorElement::EventType HTMLAnchorElement::eventType(Event* event)
{
    if (!event->isMouseEvent())
        return NonMouseEvent;
    return toMouseEvent(event)->shiftKey() ? MouseEventWithShiftKey : MouseEventWithoutShiftKey;
}

bool HTMLAnchorElement::treatLinkAsLiveForEventType(EventType eventType) const
{
    if (!rendererIsEditable())
        return true;

    Settings* settings = document()->settings();
    if (!settings)
        return true;

    switch (settings->editableLinkBehavior()) {
    case EditableLinkDefaultBehavior:
    case EditableLinkAlwaysLive:
        return true;

    case EditableLinkNeverLive:
        return false;

    // If the selection prior to clicking on this link resided in the same editable block as this link,
    // and the shift key isn't pressed, we don't want to follow the link.
    case EditableLinkLiveWhenNotFocused:
        return eventType == MouseEventWithShiftKey || (eventType == MouseEventWithoutShiftKey && rootEditableElementForSelectionOnMouseDown() != rootEditableElement());

    case EditableLinkOnlyLiveWithShiftKey:
        return eventType == MouseEventWithShiftKey;
    }

    ASSERT_NOT_REACHED();
    return false;
}

bool isEnterKeyKeydownEvent(Event* event)
{
    return event->type() == eventNames().keydownEvent && event->isKeyboardEvent() && toKeyboardEvent(event)->keyIdentifier() == "Enter";
}

bool isLinkClick(Event* event)
{
    return event->type() == eventNames().clickEvent && (!event->isMouseEvent() || toMouseEvent(event)->button() != RightButton);
}

bool HTMLAnchorElement::willRespondToMouseClickEvents()
{
    return isLink() || HTMLElement::willRespondToMouseClickEvents();
}

typedef HashMap<const HTMLAnchorElement*, RefPtr<Element> > RootEditableElementMap;

static RootEditableElementMap& rootEditableElementMap()
{
    DEFINE_STATIC_LOCAL(RootEditableElementMap, map, ());
    return map;
}

Element* HTMLAnchorElement::rootEditableElementForSelectionOnMouseDown() const
{
    if (!m_hasRootEditableElementForSelectionOnMouseDown)
        return 0;
    return rootEditableElementMap().get(this);
}

void HTMLAnchorElement::clearRootEditableElementForSelectionOnMouseDown()
{
    if (!m_hasRootEditableElementForSelectionOnMouseDown)
        return;
    rootEditableElementMap().remove(this);
    m_hasRootEditableElementForSelectionOnMouseDown = false;
}

void HTMLAnchorElement::setRootEditableElementForSelectionOnMouseDown(Element* element)
{
    if (!element) {
        clearRootEditableElementForSelectionOnMouseDown();
        return;
    }

    rootEditableElementMap().set(this, element);
    m_hasRootEditableElementForSelectionOnMouseDown = true;
}

HTMLAnchorElement::PrefetchEventHandler* HTMLAnchorElement::prefetchEventHandler()
{
    if (!m_prefetchEventHandler)
        m_prefetchEventHandler = PrefetchEventHandler::create(this);

    return m_prefetchEventHandler.get();
}

HTMLAnchorElement::PrefetchEventHandler::PrefetchEventHandler(HTMLAnchorElement* anchorElement)
    : m_anchorElement(anchorElement)
{
    ASSERT(m_anchorElement);

    reset();
}

void HTMLAnchorElement::PrefetchEventHandler::reset()
{
    m_hadHREFChanged = false;
    m_mouseOverTimestamp = 0;
    m_mouseDownTimestamp = 0;
    m_hadTapUnconfirmed = false;
    m_tapDownTimestamp = 0;
    m_hasIssuedPreconnect = false;
}

void HTMLAnchorElement::PrefetchEventHandler::handleEvent(Event* event)
{
    if (!shouldPrefetch(m_anchorElement->href()))
        return;

    if (event->type() == eventNames().mouseoverEvent)
        handleMouseOver(event);
    else if (event->type() == eventNames().mouseoutEvent)
        handleMouseOut(event);
    else if (event->type() == eventNames().mousedownEvent && event->isMouseEvent() && toMouseEvent(event)->button() == LeftButton)
        handleLeftMouseDown(event);
    else if (event->type() == eventNames().gesturetapdownEvent)
        handleGestureTapDown(event);
    else if (event->type() == eventNames().gesturetapunconfirmedEvent)
        handleGestureTapUnconfirmed(event);
    else if (isLinkClick(event))
        handleClick(event);
}

void HTMLAnchorElement::PrefetchEventHandler::handleMouseOver(Event* event)
{
    if (m_mouseOverTimestamp == 0.0) {
        m_mouseOverTimestamp = event->timeStamp();

        HistogramSupport::histogramEnumeration("MouseEventPrefetch.MouseOvers", 0, 2);

        prefetch(WebKit::WebPreconnectMotivationLinkMouseOver);
    }
}

void HTMLAnchorElement::PrefetchEventHandler::handleMouseOut(Event* event)
{
    if (m_mouseOverTimestamp > 0.0) {
        double mouseOverDuration = convertDOMTimeStampToSeconds(event->timeStamp() - m_mouseOverTimestamp);
        HistogramSupport::histogramCustomCounts("MouseEventPrefetch.MouseOverDuration_NoClick", mouseOverDuration * 1000, 0, 10000, 100);

        m_mouseOverTimestamp = 0.0;
    }
}

void HTMLAnchorElement::PrefetchEventHandler::handleLeftMouseDown(Event* event)
{
    m_mouseDownTimestamp = event->timeStamp();

    HistogramSupport::histogramEnumeration("MouseEventPrefetch.MouseDowns", 0, 2);

    prefetch(WebKit::WebPreconnectMotivationLinkMouseDown);
}

void HTMLAnchorElement::PrefetchEventHandler::handleGestureTapUnconfirmed(Event* event)
{
    m_hadTapUnconfirmed = true;

    HistogramSupport::histogramEnumeration("MouseEventPrefetch.TapUnconfirmeds", 0, 2);

    prefetch(WebKit::WebPreconnectMotivationLinkTapUnconfirmed);
}

void HTMLAnchorElement::PrefetchEventHandler::handleGestureTapDown(Event* event)
{
    m_tapDownTimestamp = event->timeStamp();

    HistogramSupport::histogramEnumeration("MouseEventPrefetch.TapDowns", 0, 2);

    prefetch(WebKit::WebPreconnectMotivationLinkTapDown);
}

void HTMLAnchorElement::PrefetchEventHandler::handleClick(Event* event)
{
    bool capturedMouseOver = (m_mouseOverTimestamp > 0.0);
    if (capturedMouseOver) {
        double mouseOverDuration = convertDOMTimeStampToSeconds(event->timeStamp() - m_mouseOverTimestamp);

        HistogramSupport::histogramCustomCounts("MouseEventPrefetch.MouseOverDuration_Click", mouseOverDuration * 1000, 0, 10000, 100);
    }

    bool capturedMouseDown = (m_mouseDownTimestamp > 0.0);
    HistogramSupport::histogramEnumeration("MouseEventPrefetch.MouseDownFollowedByClick", capturedMouseDown, 2);

    if (capturedMouseDown) {
        double mouseDownDuration = convertDOMTimeStampToSeconds(event->timeStamp() - m_mouseDownTimestamp);

        HistogramSupport::histogramCustomCounts("MouseEventPrefetch.MouseDownDuration_Click", mouseDownDuration * 1000, 0, 10000, 100);
    }

    bool capturedTapDown = (m_tapDownTimestamp > 0.0);
    if (capturedTapDown) {
        double tapDownDuration = convertDOMTimeStampToSeconds(event->timeStamp() - m_tapDownTimestamp);

        HistogramSupport::histogramCustomCounts("MouseEventPrefetch.TapDownDuration_Click", tapDownDuration * 1000, 0, 10000, 100);
    }

    int flags = (m_hadTapUnconfirmed ? 2 : 0) | (capturedTapDown ? 1 : 0);
    HistogramSupport::histogramEnumeration("MouseEventPrefetch.PreTapEventsFollowedByClick", flags, 4);

    reset();
}

bool HTMLAnchorElement::PrefetchEventHandler::shouldPrefetch(const KURL& url)
{
    if (m_hadHREFChanged)
        return false;

    if (m_anchorElement->hasEventListeners(eventNames().clickEvent))
        return false;

    if (!url.protocolIsInHTTPFamily())
        return false;

    Document* document = m_anchorElement->document();
    if (!document)
        return false;

    if (!document->securityOrigin()->canDisplay(url))
        return false;

    if (url.hasFragmentIdentifier() && equalIgnoringFragmentIdentifier(document->url(), url))
        return false;

    Frame* frame = document->frame();
    if (!frame)
        return false;

    // Links which create new window/tab are avoided because they may require user approval interaction.
    if (!m_anchorElement->target().isEmpty())
        return false;

    return true;
}

void HTMLAnchorElement::PrefetchEventHandler::prefetch(WebKit::WebPreconnectMotivation motivation)
{
    const KURL& url = m_anchorElement->href();

    if (!shouldPrefetch(url))
        return;

    // The precision of current MouseOver trigger is too low to actually trigger preconnects.
    if (motivation == WebKit::WebPreconnectMotivationLinkMouseOver)
        return;

    preconnectToURL(url, motivation);
}

}
