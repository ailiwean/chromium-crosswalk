/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Internals.h"

#include <v8.h>
#include "HTMLNames.h"
#include "InspectorFrontendClientLocal.h"
#include "InternalProfilers.h"
#include "InternalRuntimeFlags.h"
#include "InternalSettings.h"
#include "LayerRect.h"
#include "LayerRectList.h"
#include "MallocStatistics.h"
#include "MockPagePopupDriver.h"
#include "RuntimeEnabledFeatures.h"
#include "TypeConversions.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/SerializedScriptValue.h"
#include "bindings/v8/V8ThrowException.h"
#include "core/animation/DocumentTimeline.h"
#include "core/css/StyleSheetContents.h"
#include "core/css/resolver/StyleResolver.h"
#include "core/css/resolver/StyleResolverStats.h"
#include "core/css/resolver/ViewportStyleResolver.h"
#include "core/dom/ClientRect.h"
#include "core/dom/ClientRectList.h"
#include "core/dom/DOMStringList.h"
#include "core/dom/Document.h"
#include "core/dom/DocumentMarker.h"
#include "core/dom/DocumentMarkerController.h"
#include "core/dom/Element.h"
#include "core/dom/ExceptionCode.h"
#include "core/dom/FullscreenElementStack.h"
#include "core/dom/PseudoElement.h"
#include "core/dom/Range.h"
#include "core/dom/StaticNodeList.h"
#include "core/dom/TreeScope.h"
#include "core/dom/ViewportDescription.h"
#include "core/dom/WheelController.h"
#include "core/dom/shadow/ComposedTreeWalker.h"
#include "core/dom/shadow/ElementShadow.h"
#include "core/dom/shadow/SelectRuleFeatureSet.h"
#include "core/dom/shadow/ShadowRoot.h"
#include "core/editing/Editor.h"
#include "core/editing/PlainTextRange.h"
#include "core/editing/SpellCheckRequester.h"
#include "core/editing/SpellChecker.h"
#include "core/editing/TextIterator.h"
#include "core/fetch/MemoryCache.h"
#include "core/fetch/ResourceFetcher.h"
#include "core/frame/DOMPoint.h"
#include "core/frame/Frame.h"
#include "core/history/HistoryItem.h"
#include "core/html/HTMLIFrameElement.h"
#include "core/html/HTMLInputElement.h"
#include "core/html/HTMLMediaElement.h"
#include "core/html/HTMLSelectElement.h"
#include "core/html/HTMLTextAreaElement.h"
#include "core/html/forms/FormController.h"
#include "core/html/shadow/HTMLContentElement.h"
#include "core/inspector/InspectorClient.h"
#include "core/inspector/InspectorConsoleAgent.h"
#include "core/inspector/InspectorController.h"
#include "core/inspector/InspectorCounters.h"
#include "core/inspector/InspectorFrontendChannel.h"
#include "core/inspector/InspectorInstrumentation.h"
#include "core/inspector/InspectorOverlay.h"
#include "core/inspector/InstrumentingAgents.h"
#include "core/loader/FrameLoader.h"
#include "core/page/Chrome.h"
#include "core/page/ChromeClient.h"
#include "core/frame/DOMWindow.h"
#include "core/page/EventHandler.h"
#include "core/frame/FrameView.h"
#include "core/page/Page.h"
#include "core/page/PagePopupController.h"
#include "core/page/PrintContext.h"
#include "core/frame/Settings.h"
#include "core/frame/animation/AnimationController.h"
#include "core/rendering/CompositedLayerMapping.h"
#include "core/rendering/RenderLayer.h"
#include "core/rendering/RenderLayerCompositor.h"
#include "core/rendering/RenderMenuList.h"
#include "core/rendering/RenderObject.h"
#include "core/rendering/RenderTreeAsText.h"
#include "core/rendering/RenderView.h"
#include "core/testing/GCObservation.h"
#include "core/workers/WorkerThread.h"
#include "platform/ColorChooser.h"
#include "platform/Cursor.h"
#include "platform/Language.h"
#include "platform/TraceEvent.h"
#include "platform/geometry/IntRect.h"
#include "platform/geometry/LayoutRect.h"
#include "platform/graphics/GraphicsLayer.h"
#include "platform/graphics/filters/FilterOperation.h"
#include "platform/graphics/filters/FilterOperations.h"
#include "platform/graphics/gpu/SharedGraphicsContext3D.h"
#include "platform/weborigin/SchemeRegistry.h"
#include "public/platform/WebLayer.h"
#include "wtf/InstanceCounter.h"
#include "wtf/dtoa.h"
#include "wtf/text/StringBuffer.h"

namespace WebCore {

static MockPagePopupDriver* s_pagePopupDriver = 0;

using namespace HTMLNames;

class InspectorFrontendChannelDummy : public InspectorFrontendChannel {
public:
    explicit InspectorFrontendChannelDummy(Page*);
    virtual ~InspectorFrontendChannelDummy() { }
    virtual bool sendMessageToFrontend(const String& message) OVERRIDE;

private:
    Page* m_frontendPage;
};

InspectorFrontendChannelDummy::InspectorFrontendChannelDummy(Page* page)
    : m_frontendPage(page)
{
}

bool InspectorFrontendChannelDummy::sendMessageToFrontend(const String& message)
{
    return InspectorClient::doDispatchMessageOnFrontendPage(m_frontendPage, message);
}

static bool markerTypesFrom(const String& markerType, DocumentMarker::MarkerTypes& result)
{
    if (markerType.isEmpty() || equalIgnoringCase(markerType, "all"))
        result = DocumentMarker::AllMarkers();
    else if (equalIgnoringCase(markerType, "Spelling"))
        result =  DocumentMarker::Spelling;
    else if (equalIgnoringCase(markerType, "Grammar"))
        result =  DocumentMarker::Grammar;
    else if (equalIgnoringCase(markerType, "TextMatch"))
        result =  DocumentMarker::TextMatch;
    else
        return false;

    return true;
}

static SpellCheckRequester* spellCheckRequester(Document* document)
{
    if (!document || !document->frame())
        return 0;
    return &document->frame()->spellChecker().spellCheckRequester();
}

const char* Internals::internalsId = "internals";

PassRefPtr<Internals> Internals::create(Document* document)
{
    return adoptRef(new Internals(document));
}

Internals::~Internals()
{
}

void Internals::resetToConsistentState(Page* page)
{
    ASSERT(page);

    page->setDeviceScaleFactor(1);
    page->setIsCursorVisible(true);
    page->setPageScaleFactor(1, IntPoint(0, 0));
    page->setPagination(Pagination());
    TextRun::setAllowsRoundingHacks(false);
    WebCore::overrideUserPreferredLanguages(Vector<String>());
    delete s_pagePopupDriver;
    s_pagePopupDriver = 0;
    page->chrome().client().resetPagePopupDriver();
    if (!page->mainFrame()->spellChecker().isContinuousSpellCheckingEnabled())
        page->mainFrame()->spellChecker().toggleContinuousSpellChecking();
    if (page->mainFrame()->editor().isOverwriteModeEnabled())
        page->mainFrame()->editor().toggleOverwriteModeEnabled();

    if (ScrollingCoordinator* scrollingCoordinator = page->scrollingCoordinator())
        scrollingCoordinator->reset();

    page->mainFrame()->view()->clear();
}

Internals::Internals(Document* document)
    : ContextLifecycleObserver(document)
    , m_runtimeFlags(InternalRuntimeFlags::create())
    , m_scrollingCoordinator(document->page() ? document->page()->scrollingCoordinator() : 0)
{
}

Document* Internals::contextDocument() const
{
    return toDocument(executionContext());
}

Frame* Internals::frame() const
{
    if (!contextDocument())
        return 0;
    return contextDocument()->frame();
}

InternalSettings* Internals::settings() const
{
    Document* document = contextDocument();
    if (!document)
        return 0;
    Page* page = document->page();
    if (!page)
        return 0;
    return InternalSettings::from(page);
}

InternalRuntimeFlags* Internals::runtimeFlags() const
{
    return m_runtimeFlags.get();
}

InternalProfilers* Internals::profilers()
{
    if (!m_profilers)
        m_profilers = InternalProfilers::create();
    return m_profilers.get();
}

unsigned Internals::workerThreadCount() const
{
    return WorkerThread::workerThreadCount();
}

String Internals::address(Node* node)
{
    char buf[32];
    sprintf(buf, "%p", node);

    return String(buf);
}

PassRefPtr<GCObservation> Internals::observeGC(ScriptValue scriptValue)
{
    v8::Handle<v8::Value> observedValue = scriptValue.v8Value();
    ASSERT(!observedValue.IsEmpty());
    if (observedValue->IsNull() || observedValue->IsUndefined()) {
        V8ThrowException::throwTypeError("value to observe is null or undefined", v8::Isolate::GetCurrent());
        return 0;
    }

    return GCObservation::create(observedValue);
}

unsigned Internals::updateStyleAndReturnAffectedElementCount(ExceptionState& exceptionState) const
{
    Document* document = contextDocument();
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    unsigned beforeCount = document->styleEngine()->resolverAccessCount();
    document->updateStyleIfNeeded();
    return document->styleEngine()->resolverAccessCount() - beforeCount;
}

bool Internals::isPreloaded(const String& url)
{
    Document* document = contextDocument();
    return document->fetcher()->isPreloaded(url);
}

bool Internals::isLoadingFromMemoryCache(const String& url)
{
    if (!contextDocument())
        return false;
    Resource* resource = memoryCache()->resourceForURL(contextDocument()->completeURL(url));
    return resource && resource->status() == Resource::Cached;
}

void Internals::crash()
{
    CRASH();
}

void Internals::setStyleResolverStatsEnabled(bool enabled)
{
    Document* document = contextDocument();
    if (enabled)
        document->ensureStyleResolver().enableStats(StyleResolver::ReportSlowStats);
    else
        document->ensureStyleResolver().disableStats();
}

String Internals::styleResolverStatsReport(ExceptionState& exceptionState) const
{
    Document* document = contextDocument();
    if (!document->ensureStyleResolver().stats()) {
        exceptionState.throwDOMException(InvalidStateError, "Style resolver stats not enabled");
        return String();
    }
    return document->ensureStyleResolver().stats()->report();
}

String Internals::styleResolverStatsTotalsReport(ExceptionState& exceptionState) const
{
    Document* document = contextDocument();
    if (!document->ensureStyleResolver().statsTotals()) {
        exceptionState.throwDOMException(InvalidStateError, "Style resolver stats not enabled");
        return String();
    }
    return document->ensureStyleResolver().statsTotals()->report();
}

PassRefPtr<Element> Internals::createContentElement(ExceptionState& exceptionState)
{
    Document* document = contextDocument();
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return HTMLContentElement::create(*document);
}

bool Internals::isValidContentSelect(Element* insertionPoint, ExceptionState& exceptionState)
{
    if (!insertionPoint || !insertionPoint->isInsertionPoint()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return false;
    }

    return insertionPoint->hasTagName(contentTag) && toHTMLContentElement(insertionPoint)->isSelectValid();
}

Node* Internals::treeScopeRootNode(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return node->treeScope().rootNode();
}

Node* Internals::parentTreeScope(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    const TreeScope* parentTreeScope = node->treeScope().parentTreeScope();
    return parentTreeScope ? parentTreeScope->rootNode() : 0;
}

bool Internals::hasSelectorForIdInShadow(Element* host, const AtomicString& idValue, ExceptionState& exceptionState)
{
    if (!host || !host->shadow()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return host->shadow()->ensureSelectFeatureSet().hasSelectorForId(idValue);
}

bool Internals::hasSelectorForClassInShadow(Element* host, const AtomicString& className, ExceptionState& exceptionState)
{
    if (!host || !host->shadow()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return host->shadow()->ensureSelectFeatureSet().hasSelectorForClass(className);
}

bool Internals::hasSelectorForAttributeInShadow(Element* host, const AtomicString& attributeName, ExceptionState& exceptionState)
{
    if (!host || !host->shadow()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return host->shadow()->ensureSelectFeatureSet().hasSelectorForAttribute(attributeName);
}

bool Internals::hasSelectorForPseudoClassInShadow(Element* host, const String& pseudoClass, ExceptionState& exceptionState)
{
    if (!host || !host->shadow()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    const SelectRuleFeatureSet& featureSet = host->shadow()->ensureSelectFeatureSet();
    if (pseudoClass == "checked")
        return featureSet.hasSelectorForChecked();
    if (pseudoClass == "enabled")
        return featureSet.hasSelectorForEnabled();
    if (pseudoClass == "disabled")
        return featureSet.hasSelectorForDisabled();
    if (pseudoClass == "indeterminate")
        return featureSet.hasSelectorForIndeterminate();
    if (pseudoClass == "link")
        return featureSet.hasSelectorForLink();
    if (pseudoClass == "target")
        return featureSet.hasSelectorForTarget();
    if (pseudoClass == "visited")
        return featureSet.hasSelectorForVisited();

    ASSERT_NOT_REACHED();
    return false;
}

unsigned short Internals::compareTreeScopePosition(const Node* node1, const Node* node2, ExceptionState& exceptionState) const
{
    if (!node1 || !node2) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    const TreeScope* treeScope1 = node1->isDocumentNode() ? static_cast<const TreeScope*>(toDocument(node1)) :
        node1->isShadowRoot() ? static_cast<const TreeScope*>(toShadowRoot(node1)) : 0;
    const TreeScope* treeScope2 = node2->isDocumentNode() ? static_cast<const TreeScope*>(toDocument(node2)) :
        node2->isShadowRoot() ? static_cast<const TreeScope*>(toShadowRoot(node2)) : 0;
    if (!treeScope1 || !treeScope2) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    return treeScope1->comparePosition(*treeScope2);
}

unsigned Internals::numberOfActiveAnimations() const
{
    Frame* contextFrame = frame();
    Document* document = contextFrame->document();
    if (RuntimeEnabledFeatures::webAnimationsCSSEnabled())
        return document->timeline()->numberOfActiveAnimationsForTesting() + document->transitionTimeline()->numberOfActiveAnimationsForTesting();
    return contextFrame->animation().numberOfActiveAnimations(document);
}

void Internals::pauseAnimations(double pauseTime, ExceptionState& exceptionState)
{
    if (pauseTime < 0) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    if (RuntimeEnabledFeatures::webAnimationsCSSEnabled()) {
        frame()->document()->timeline()->pauseAnimationsForTesting(pauseTime);
        frame()->document()->transitionTimeline()->pauseAnimationsForTesting(pauseTime);
    } else {
        frame()->animation().pauseAnimationsForTesting(pauseTime);
    }
}

bool Internals::hasShadowInsertionPoint(const Node* root, ExceptionState& exceptionState) const
{
    if (root && root->isShadowRoot())
        return toShadowRoot(root)->containsShadowElements();

    exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
    return 0;
}

bool Internals::hasContentElement(const Node* root, ExceptionState& exceptionState) const
{
    if (root && root->isShadowRoot())
        return toShadowRoot(root)->containsContentElements();

    exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
    return 0;
}

size_t Internals::countElementShadow(const Node* root, ExceptionState& exceptionState) const
{
    if (!root || !root->isShadowRoot()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    return toShadowRoot(root)->childShadowRootCount();
}

Node* Internals::nextSiblingByWalker(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    ComposedTreeWalker walker(node);
    walker.nextSibling();
    return walker.get();
}

Node* Internals::firstChildByWalker(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    ComposedTreeWalker walker(node);
    walker.firstChild();
    return walker.get();
}

Node* Internals::lastChildByWalker(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    ComposedTreeWalker walker(node);
    walker.lastChild();
    return walker.get();
}

Node* Internals::nextNodeByWalker(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    ComposedTreeWalker walker(node);
    walker.next();
    return walker.get();
}

Node* Internals::previousNodeByWalker(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }
    ComposedTreeWalker walker(node);
    walker.previous();
    return walker.get();
}

String Internals::elementRenderTreeAsText(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    String representation = externalRepresentation(element);
    if (representation.isEmpty()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return representation;
}

size_t Internals::numberOfScopedHTMLStyleChildren(const Node* scope, ExceptionState& exceptionState) const
{
    if (scope && (scope->isElementNode() || scope->isShadowRoot()))
        return scope->numberOfScopedHTMLStyleChildren();

    exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
    return 0;
}

PassRefPtr<CSSComputedStyleDeclaration> Internals::computedStyleIncludingVisitedInfo(Node* node, ExceptionState& exceptionState) const
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    bool allowVisitedStyle = true;
    return CSSComputedStyleDeclaration::create(node, allowVisitedStyle);
}

ShadowRoot* Internals::ensureShadowRoot(Element* host, ExceptionState& exceptionState)
{
    if (!host) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    if (ElementShadow* shadow = host->shadow())
        return shadow->youngestShadowRoot();

    return host->createShadowRoot(exceptionState).get();
}

ShadowRoot* Internals::shadowRoot(Element* host, ExceptionState& exceptionState)
{
    // FIXME: Internals::shadowRoot() in tests should be converted to youngestShadowRoot() or oldestShadowRoot().
    // https://bugs.webkit.org/show_bug.cgi?id=78465
    return youngestShadowRoot(host, exceptionState);
}

ShadowRoot* Internals::youngestShadowRoot(Element* host, ExceptionState& exceptionState)
{
    if (!host) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    if (ElementShadow* shadow = host->shadow())
        return shadow->youngestShadowRoot();
    return 0;
}

ShadowRoot* Internals::oldestShadowRoot(Element* host, ExceptionState& exceptionState)
{
    if (!host) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    if (ElementShadow* shadow = host->shadow())
        return shadow->oldestShadowRoot();
    return 0;
}

ShadowRoot* Internals::youngerShadowRoot(Node* shadow, ExceptionState& exceptionState)
{
    if (!shadow || !shadow->isShadowRoot()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return toShadowRoot(shadow)->youngerShadowRoot();
}

ShadowRoot* Internals::olderShadowRoot(Node* shadow, ExceptionState& exceptionState)
{
    if (!shadow || !shadow->isShadowRoot()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return toShadowRoot(shadow)->olderShadowRoot();
}

String Internals::shadowRootType(const Node* root, ExceptionState& exceptionState) const
{
    if (!root || !root->isShadowRoot()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    switch (toShadowRoot(root)->type()) {
    case ShadowRoot::UserAgentShadowRoot:
        return String("UserAgentShadowRoot");
    case ShadowRoot::AuthorShadowRoot:
        return String("AuthorShadowRoot");
    default:
        ASSERT_NOT_REACHED();
        return String("Unknown");
    }
}

const AtomicString& Internals::shadowPseudoId(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return nullAtom;
    }

    return element->shadowPseudoId();
}

void Internals::setShadowPseudoId(Element* element, const AtomicString& id, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    return element->setShadowPseudoId(id);
}

String Internals::visiblePlaceholder(Element* element)
{
    if (element && isHTMLTextFormControlElement(*element)) {
        if (toHTMLTextFormControlElement(element)->placeholderShouldBeVisible())
            return toHTMLTextFormControlElement(element)->placeholderElement()->textContent();
    }

    return String();
}

void Internals::selectColorInColorChooser(Element* element, const String& colorValue)
{
    if (!element->hasTagName(inputTag))
        return;
    toHTMLInputElement(element)->selectColorInColorChooser(Color(colorValue));
}

Vector<String> Internals::formControlStateOfHistoryItem(ExceptionState& exceptionState)
{
    HistoryItem* mainItem = frame()->loader().currentItem();
    if (!mainItem) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return Vector<String>();
    }
    return mainItem->documentState();
}

void Internals::setFormControlStateOfHistoryItem(const Vector<String>& state, ExceptionState& exceptionState)
{
    HistoryItem* mainItem = frame()->loader().currentItem();
    if (!mainItem) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    mainItem->setDocumentState(state);
}

void Internals::setEnableMockPagePopup(bool enabled, ExceptionState& exceptionState)
{
    Document* document = contextDocument();
    if (!document || !document->page())
        return;
    Page* page = document->page();
    if (!enabled) {
        page->chrome().client().resetPagePopupDriver();
        return;
    }
    if (!s_pagePopupDriver)
        s_pagePopupDriver = MockPagePopupDriver::create(page->mainFrame()).leakPtr();
    page->chrome().client().setPagePopupDriver(s_pagePopupDriver);
}

PassRefPtr<PagePopupController> Internals::pagePopupController()
{
    return s_pagePopupDriver ? s_pagePopupDriver->pagePopupController() : 0;
}

PassRefPtr<ClientRect> Internals::unscaledViewportRect(ExceptionState& exceptionState)
{
    Document* document = contextDocument();
    if (!document || !document->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return ClientRect::create();
    }

    return ClientRect::create(document->view()->visibleContentRect());
}

PassRefPtr<ClientRect> Internals::absoluteCaretBounds(ExceptionState& exceptionState)
{
    Document* document = contextDocument();
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return ClientRect::create();
    }

    return ClientRect::create(document->frame()->selection().absoluteCaretBounds());
}

PassRefPtr<ClientRect> Internals::boundingBox(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return ClientRect::create();
    }

    element->document().updateLayoutIgnorePendingStylesheets();
    RenderObject* renderer = element->renderer();
    if (!renderer)
        return ClientRect::create();
    return ClientRect::create(renderer->absoluteBoundingBoxRectIgnoringTransforms());
}

PassRefPtr<ClientRectList> Internals::inspectorHighlightRects(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->page()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return ClientRectList::create();
    }

    Highlight highlight;
    document->page()->inspectorController().getHighlight(&highlight);
    return ClientRectList::create(highlight.quads);
}

unsigned Internals::markerCountForNode(Node* node, const String& markerType, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    DocumentMarker::MarkerTypes markerTypes = 0;
    if (!markerTypesFrom(markerType, markerTypes)) {
        exceptionState.throwUninformativeAndGenericDOMException(SyntaxError);
        return 0;
    }

    return node->document().markers()->markersFor(node, markerTypes).size();
}

unsigned Internals::activeMarkerCountForNode(Node* node, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    // Only TextMatch markers can be active.
    DocumentMarker::MarkerType markerType = DocumentMarker::TextMatch;
    Vector<DocumentMarker*> markers = node->document().markers()->markersFor(node, markerType);

    unsigned activeMarkerCount = 0;
    for (Vector<DocumentMarker*>::iterator iter = markers.begin(); iter != markers.end(); ++iter) {
        if ((*iter)->activeMatch())
            activeMarkerCount++;
    }

    return activeMarkerCount;
}

DocumentMarker* Internals::markerAt(Node* node, const String& markerType, unsigned index, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    DocumentMarker::MarkerTypes markerTypes = 0;
    if (!markerTypesFrom(markerType, markerTypes)) {
        exceptionState.throwUninformativeAndGenericDOMException(SyntaxError);
        return 0;
    }

    Vector<DocumentMarker*> markers = node->document().markers()->markersFor(node, markerTypes);
    if (markers.size() <= index)
        return 0;
    return markers[index];
}

PassRefPtr<Range> Internals::markerRangeForNode(Node* node, const String& markerType, unsigned index, ExceptionState& exceptionState)
{
    DocumentMarker* marker = markerAt(node, markerType, index, exceptionState);
    if (!marker)
        return 0;
    return Range::create(node->document(), node, marker->startOffset(), node, marker->endOffset());
}

String Internals::markerDescriptionForNode(Node* node, const String& markerType, unsigned index, ExceptionState& exceptionState)
{
    DocumentMarker* marker = markerAt(node, markerType, index, exceptionState);
    if (!marker)
        return String();
    return marker->description();
}

void Internals::addTextMatchMarker(const Range* range, bool isActive)
{
    range->ownerDocument().updateLayoutIgnorePendingStylesheets();
    range->ownerDocument().markers()->addTextMatchMarker(range, isActive);
}

void Internals::setMarkersActive(Node* node, unsigned startOffset, unsigned endOffset, bool active, ExceptionState& exceptionState)
{
    if (!node) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    node->document().markers()->setMarkersActive(node, startOffset, endOffset, active);
}

void Internals::setScrollViewPosition(Document* document, long x, long y, ExceptionState& exceptionState)
{
    if (!document || !document->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    FrameView* frameView = document->view();
    bool constrainsScrollingToContentEdgeOldValue = frameView->constrainsScrollingToContentEdge();
    bool scrollbarsSuppressedOldValue = frameView->scrollbarsSuppressed();

    frameView->setConstrainsScrollingToContentEdge(false);
    frameView->setScrollbarsSuppressed(false);
    frameView->setScrollOffsetFromInternals(IntPoint(x, y));
    frameView->setScrollbarsSuppressed(scrollbarsSuppressedOldValue);
    frameView->setConstrainsScrollingToContentEdge(constrainsScrollingToContentEdgeOldValue);
}

void Internals::setPagination(Document* document, const String& mode, int gap, int pageLength, ExceptionState& exceptionState)
{
    if (!document || !document->page()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    Page* page = document->page();

    Pagination pagination;
    if (mode == "Unpaginated")
        pagination.mode = Pagination::Unpaginated;
    else if (mode == "LeftToRightPaginated")
        pagination.mode = Pagination::LeftToRightPaginated;
    else if (mode == "RightToLeftPaginated")
        pagination.mode = Pagination::RightToLeftPaginated;
    else if (mode == "TopToBottomPaginated")
        pagination.mode = Pagination::TopToBottomPaginated;
    else if (mode == "BottomToTopPaginated")
        pagination.mode = Pagination::BottomToTopPaginated;
    else {
        exceptionState.throwUninformativeAndGenericDOMException(SyntaxError);
        return;
    }

    pagination.gap = gap;
    pagination.pageLength = pageLength;
    page->setPagination(pagination);
}

String Internals::viewportAsText(Document* document, float, int availableWidth, int availableHeight, ExceptionState& exceptionState)
{
    if (!document || !document->page()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    document->updateLayoutIgnorePendingStylesheets();

    Page* page = document->page();

    // Update initial viewport size.
    IntSize initialViewportSize(availableWidth, availableHeight);
    document->page()->mainFrame()->view()->setFrameRect(IntRect(IntPoint::zero(), initialViewportSize));

    ViewportDescription description = page->viewportDescription();
    PageScaleConstraints constraints = description.resolve(initialViewportSize);

    constraints.fitToContentsWidth(constraints.layoutSize.width(), availableWidth);

    StringBuilder builder;

    builder.appendLiteral("viewport size ");
    builder.append(String::number(constraints.layoutSize.width()));
    builder.append('x');
    builder.append(String::number(constraints.layoutSize.height()));

    builder.appendLiteral(" scale ");
    builder.append(String::number(constraints.initialScale));
    builder.appendLiteral(" with limits [");
    builder.append(String::number(constraints.minimumScale));
    builder.appendLiteral(", ");
    builder.append(String::number(constraints.maximumScale));

    builder.appendLiteral("] and userScalable ");
    builder.append(description.userZoom ? "true" : "false");

    return builder.toString();
}

bool Internals::wasLastChangeUserEdit(Element* textField, ExceptionState& exceptionState)
{
    if (!textField) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return false;
    }

    if (textField->hasTagName(inputTag))
        return toHTMLInputElement(textField)->lastChangeWasUserEdit();

    // FIXME: We should be using hasTagName instead but Windows port doesn't link QualifiedNames properly.
    if (textField->tagName() == "TEXTAREA")
        return toHTMLTextAreaElement(textField)->lastChangeWasUserEdit();

    exceptionState.throwUninformativeAndGenericDOMException(InvalidNodeTypeError);
    return false;
}

bool Internals::elementShouldAutoComplete(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return false;
    }

    if (element->hasTagName(inputTag))
        return toHTMLInputElement(element)->shouldAutocomplete();

    exceptionState.throwUninformativeAndGenericDOMException(InvalidNodeTypeError);
    return false;
}

String Internals::suggestedValue(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    if (!element->hasTagName(inputTag)) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidNodeTypeError);
        return String();
    }

    return toHTMLInputElement(element)->suggestedValue();
}

void Internals::setSuggestedValue(Element* element, const String& value, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    if (!element->hasTagName(inputTag)) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidNodeTypeError);
        return;
    }

    toHTMLInputElement(element)->setSuggestedValue(value);
}

void Internals::setEditingValue(Element* element, const String& value, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    if (!element->hasTagName(inputTag)) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidNodeTypeError);
        return;
    }

    toHTMLInputElement(element)->setEditingValue(value);
}

void Internals::setAutofilled(Element* element, bool enabled, ExceptionState& exceptionState)
{
    if (!element->isFormControlElement()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    toHTMLFormControlElement(element)->setAutofilled(enabled);
}

void Internals::scrollElementToRect(Element* element, long x, long y, long w, long h, ExceptionState& exceptionState)
{
    if (!element || !element->document().view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    FrameView* frameView = element->document().view();
    frameView->scrollElementToRect(element, IntRect(x, y, w, h));
}

void Internals::paintControlTints(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    FrameView* frameView = document->view();
    frameView->paintControlTints();
}

PassRefPtr<Range> Internals::rangeFromLocationAndLength(Element* scope, int rangeLocation, int rangeLength, ExceptionState& exceptionState)
{
    if (!scope) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    // TextIterator depends on Layout information, make sure layout it up to date.
    scope->document().updateLayoutIgnorePendingStylesheets();

    return PlainTextRange(rangeLocation, rangeLocation + rangeLength).createRange(*scope);
}

unsigned Internals::locationFromRange(Element* scope, const Range* range, ExceptionState& exceptionState)
{
    if (!scope || !range) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    // PlainTextRange depends on Layout information, make sure layout it up to date.
    scope->document().updateLayoutIgnorePendingStylesheets();

    return PlainTextRange::create(*scope, *range).start();
}

unsigned Internals::lengthFromRange(Element* scope, const Range* range, ExceptionState& exceptionState)
{
    if (!scope || !range) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    // PlainTextRange depends on Layout information, make sure layout it up to date.
    scope->document().updateLayoutIgnorePendingStylesheets();

    return PlainTextRange::create(*scope, *range).length();
}

String Internals::rangeAsText(const Range* range, ExceptionState& exceptionState)
{
    if (!range) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return range->text();
}

PassRefPtr<DOMPoint> Internals::touchPositionAdjustedToBestClickableNode(long x, long y, long width, long height, Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    document->updateLayout();

    IntSize radius(width / 2, height / 2);
    IntPoint point(x + radius.width(), y + radius.height());

    Node* targetNode;
    IntPoint adjustedPoint;

    bool foundNode = document->frame()->eventHandler().bestClickableNodeForTouchPoint(point, radius, adjustedPoint, targetNode);
    if (foundNode)
        return DOMPoint::create(adjustedPoint.x(), adjustedPoint.y());

    return 0;
}

Node* Internals::touchNodeAdjustedToBestClickableNode(long x, long y, long width, long height, Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    document->updateLayout();

    IntSize radius(width / 2, height / 2);
    IntPoint point(x + radius.width(), y + radius.height());

    Node* targetNode;
    IntPoint adjustedPoint;
    document->frame()->eventHandler().bestClickableNodeForTouchPoint(point, radius, adjustedPoint, targetNode);
    return targetNode;
}

PassRefPtr<DOMPoint> Internals::touchPositionAdjustedToBestContextMenuNode(long x, long y, long width, long height, Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    document->updateLayout();

    IntSize radius(width / 2, height / 2);
    IntPoint point(x + radius.width(), y + radius.height());

    Node* targetNode = 0;
    IntPoint adjustedPoint;

    bool foundNode = document->frame()->eventHandler().bestContextMenuNodeForTouchPoint(point, radius, adjustedPoint, targetNode);
    if (foundNode)
        return DOMPoint::create(adjustedPoint.x(), adjustedPoint.y());

    return DOMPoint::create(x, y);
}

Node* Internals::touchNodeAdjustedToBestContextMenuNode(long x, long y, long width, long height, Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    document->updateLayout();

    IntSize radius(width / 2, height / 2);
    IntPoint point(x + radius.width(), y + radius.height());

    Node* targetNode = 0;
    IntPoint adjustedPoint;
    document->frame()->eventHandler().bestContextMenuNodeForTouchPoint(point, radius, adjustedPoint, targetNode);
    return targetNode;
}

PassRefPtr<ClientRect> Internals::bestZoomableAreaForTouchPoint(long x, long y, long width, long height, Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    document->updateLayout();

    IntSize radius(width / 2, height / 2);
    IntPoint point(x + radius.width(), y + radius.height());

    Node* targetNode;
    IntRect zoomableArea;
    bool foundNode = document->frame()->eventHandler().bestZoomableAreaForTouchPoint(point, radius, zoomableArea, targetNode);
    if (foundNode)
        return ClientRect::create(zoomableArea);

    return 0;
}


int Internals::lastSpellCheckRequestSequence(Document* document, ExceptionState& exceptionState)
{
    SpellCheckRequester* requester = spellCheckRequester(document);

    if (!requester) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return -1;
    }

    return requester->lastRequestSequence();
}

int Internals::lastSpellCheckProcessedSequence(Document* document, ExceptionState& exceptionState)
{
    SpellCheckRequester* requester = spellCheckRequester(document);

    if (!requester) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return -1;
    }

    return requester->lastProcessedSequence();
}

Vector<String> Internals::userPreferredLanguages() const
{
    return WebCore::userPreferredLanguages();
}

void Internals::setUserPreferredLanguages(const Vector<String>& languages)
{
    WebCore::overrideUserPreferredLanguages(languages);
}

unsigned Internals::wheelEventHandlerCount(Document* document, ExceptionState& exceptionState)
{
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return WheelController::from(document)->wheelEventHandlerCount();
}

unsigned Internals::touchEventHandlerCount(Document* document, ExceptionState& exceptionState)
{
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    const TouchEventTargetSet* touchHandlers = document->touchEventTargets();
    if (!touchHandlers)
        return 0;

    unsigned count = 0;
    for (TouchEventTargetSet::const_iterator iter = touchHandlers->begin(); iter != touchHandlers->end(); ++iter)
        count += iter->value;
    return count;
}

static RenderLayer* findRenderLayerForGraphicsLayer(RenderLayer* searchRoot, GraphicsLayer* graphicsLayer, String* layerType)
{
    if (searchRoot->hasCompositedLayerMapping() && graphicsLayer == searchRoot->compositedLayerMapping()->mainGraphicsLayer())
        return searchRoot;

    GraphicsLayer* layerForScrolling = searchRoot->scrollableArea() ? searchRoot->scrollableArea()->layerForScrolling() : 0;
    if (graphicsLayer == layerForScrolling) {
        *layerType = "scrolling";
        return searchRoot;
    }

    GraphicsLayer* layerForHorizontalScrollbar = searchRoot->scrollableArea() ? searchRoot->scrollableArea()->layerForHorizontalScrollbar() : 0;
    if (graphicsLayer == layerForHorizontalScrollbar) {
        *layerType = "horizontalScrollbar";
        return searchRoot;
    }

    GraphicsLayer* layerForVerticalScrollbar = searchRoot->scrollableArea() ? searchRoot->scrollableArea()->layerForVerticalScrollbar() : 0;
    if (graphicsLayer == layerForVerticalScrollbar) {
        *layerType = "verticalScrollbar";
        return searchRoot;
    }

    GraphicsLayer* layerForScrollCorner = searchRoot->scrollableArea() ? searchRoot->scrollableArea()->layerForScrollCorner() : 0;
    if (graphicsLayer == layerForScrollCorner) {
        *layerType = "scrollCorner";
        return searchRoot;
    }

    for (RenderLayer* child = searchRoot->firstChild(); child; child = child->nextSibling()) {
        RenderLayer* foundLayer = findRenderLayerForGraphicsLayer(child, graphicsLayer, layerType);
        if (foundLayer)
            return foundLayer;
    }

    return 0;
}

// Given a vector of rects, merge those that are adjacent, leaving empty rects
// in the place of no longer used slots. This is intended to simplify the list
// of rects returned by an SkRegion (which have been split apart for sorting
// purposes). No attempt is made to do this efficiently (eg. by relying on the
// sort criteria of SkRegion).
static void mergeRects(blink::WebVector<blink::WebRect>& rects)
{
    for (size_t i = 0; i < rects.size(); ++i) {
        if (rects[i].isEmpty())
            continue;
        bool updated;
        do {
            updated = false;
            for (size_t j = i+1; j < rects.size(); ++j) {
                if (rects[j].isEmpty())
                    continue;
                // Try to merge rects[j] into rects[i] along the 4 possible edges.
                if (rects[i].y == rects[j].y && rects[i].height == rects[j].height) {
                    if (rects[i].x + rects[i].width == rects[j].x) {
                        rects[i].width += rects[j].width;
                        rects[j] = blink::WebRect();
                        updated = true;
                    } else if (rects[i].x == rects[j].x + rects[j].width) {
                        rects[i].x = rects[j].x;
                        rects[i].width += rects[j].width;
                        rects[j] = blink::WebRect();
                        updated = true;
                    }
                } else if (rects[i].x == rects[j].x && rects[i].width == rects[j].width) {
                    if (rects[i].y + rects[i].height == rects[j].y) {
                        rects[i].height += rects[j].height;
                        rects[j] = blink::WebRect();
                        updated = true;
                    } else if (rects[i].y == rects[j].y + rects[j].height) {
                        rects[i].y = rects[j].y;
                        rects[i].height += rects[j].height;
                        rects[j] = blink::WebRect();
                        updated = true;
                    }
                }
            }
        } while (updated);
    }
}

static void accumulateLayerRectList(RenderLayerCompositor* compositor, GraphicsLayer* graphicsLayer, LayerRectList* rects)
{
    blink::WebVector<blink::WebRect> layerRects = graphicsLayer->platformLayer()->touchEventHandlerRegion();
    if (!layerRects.isEmpty()) {
        mergeRects(layerRects);
        String layerType;
        RenderLayer* renderLayer = findRenderLayerForGraphicsLayer(compositor->rootRenderLayer(), graphicsLayer, &layerType);
        Node* node = renderLayer ? renderLayer->renderer()->node() : 0;
        for (size_t i = 0; i < layerRects.size(); ++i) {
            if (!layerRects[i].isEmpty())
                rects->append(node, layerType, ClientRect::create(layerRects[i]));
        }
    }

    size_t numChildren = graphicsLayer->children().size();
    for (size_t i = 0; i < numChildren; ++i)
        accumulateLayerRectList(compositor, graphicsLayer->children()[i], rects);
}

PassRefPtr<LayerRectList> Internals::touchEventTargetLayerRects(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->view() || !document->page() || document != contextDocument()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    // Do any pending layout and compositing update (which may call touchEventTargetRectsChange) to ensure this
    // really takes any previous changes into account.
    forceCompositingUpdate(document, exceptionState);
    if (exceptionState.hadException())
        return 0;

    if (RenderView* view = document->renderView()) {
        if (RenderLayerCompositor* compositor = view->compositor()) {
            if (GraphicsLayer* rootLayer = compositor->rootGraphicsLayer()) {
                RefPtr<LayerRectList> rects = LayerRectList::create();
                accumulateLayerRectList(compositor, rootLayer, rects.get());
                return rects;
            }
        }
    }

    return 0;
}

PassRefPtr<NodeList> Internals::nodesFromRect(Document* document, int centerX, int centerY, unsigned topPadding, unsigned rightPadding,
    unsigned bottomPadding, unsigned leftPadding, bool ignoreClipping, bool allowShadowContent, bool allowChildFrameContent, ExceptionState& exceptionState) const
{
    if (!document || !document->frame() || !document->frame()->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    Frame* frame = document->frame();
    FrameView* frameView = document->view();
    RenderView* renderView = document->renderView();

    if (!renderView)
        return 0;

    float zoomFactor = frame->pageZoomFactor();
    LayoutPoint point = roundedLayoutPoint(FloatPoint(centerX * zoomFactor + frameView->scrollX(), centerY * zoomFactor + frameView->scrollY()));

    HitTestRequest::HitTestRequestType hitType = HitTestRequest::ReadOnly | HitTestRequest::Active;
    if (ignoreClipping)
        hitType |= HitTestRequest::IgnoreClipping;
    if (!allowShadowContent)
        hitType |= HitTestRequest::ConfusingAndOftenMisusedDisallowShadowContent;
    if (allowChildFrameContent)
        hitType |= HitTestRequest::AllowChildFrameContent;

    HitTestRequest request(hitType);

    // When ignoreClipping is false, this method returns null for coordinates outside of the viewport.
    if (!request.ignoreClipping() && !frameView->visibleContentRect().intersects(HitTestLocation::rectForPoint(point, topPadding, rightPadding, bottomPadding, leftPadding)))
        return 0;

    Vector<RefPtr<Node> > matches;

    // Need padding to trigger a rect based hit test, but we want to return a NodeList
    // so we special case this.
    if (!topPadding && !rightPadding && !bottomPadding && !leftPadding) {
        HitTestResult result(point);
        renderView->hitTest(request, result);
        if (result.innerNode())
            matches.append(result.innerNode()->deprecatedShadowAncestorNode());
    } else {
        HitTestResult result(point, topPadding, rightPadding, bottomPadding, leftPadding);
        renderView->hitTest(request, result);
        copyToVector(result.rectBasedTestResult(), matches);
    }

    return StaticNodeList::adopt(matches);
}

void Internals::emitInspectorDidBeginFrame(int frameId)
{
    contextDocument()->page()->inspectorController().didBeginFrame(frameId);
}

void Internals::emitInspectorDidCancelFrame()
{
    contextDocument()->page()->inspectorController().didCancelFrame();
}

bool Internals::hasSpellingMarker(Document* document, int from, int length, ExceptionState&)
{
    if (!document || !document->frame())
        return 0;

    return document->frame()->spellChecker().selectionStartHasMarkerFor(DocumentMarker::Spelling, from, length);
}

void Internals::setContinuousSpellCheckingEnabled(bool enabled, ExceptionState&)
{
    if (!contextDocument() || !contextDocument()->frame())
        return;

    if (enabled != contextDocument()->frame()->spellChecker().isContinuousSpellCheckingEnabled())
        contextDocument()->frame()->spellChecker().toggleContinuousSpellChecking();
}

bool Internals::isOverwriteModeEnabled(Document* document, ExceptionState&)
{
    if (!document || !document->frame())
        return 0;

    return document->frame()->editor().isOverwriteModeEnabled();
}

void Internals::toggleOverwriteModeEnabled(Document* document, ExceptionState&)
{
    if (!document || !document->frame())
        return;

    document->frame()->editor().toggleOverwriteModeEnabled();
}

unsigned Internals::numberOfLiveNodes() const
{
    return InspectorCounters::counterValue(InspectorCounters::NodeCounter);
}

unsigned Internals::numberOfLiveDocuments() const
{
    return InspectorCounters::counterValue(InspectorCounters::DocumentCounter);
}

String Internals::dumpRefCountedInstanceCounts() const
{
    return WTF::dumpRefCountedInstanceCounts();
}

Vector<String> Internals::consoleMessageArgumentCounts(Document* document) const
{
    InstrumentingAgents* instrumentingAgents = instrumentationForPage(document->page());
    if (!instrumentingAgents)
        return Vector<String>();
    InspectorConsoleAgent* consoleAgent = instrumentingAgents->inspectorConsoleAgent();
    if (!consoleAgent)
        return Vector<String>();
    Vector<unsigned> counts = consoleAgent->consoleMessageArgumentCounts();
    Vector<String> result(counts.size());
    for (size_t i = 0; i < counts.size(); i++)
        result[i] = String::number(counts[i]);
    return result;
}

PassRefPtr<DOMWindow> Internals::openDummyInspectorFrontend(const String& url)
{
    Page* page = contextDocument()->frame()->page();
    ASSERT(page);

    DOMWindow* window = page->mainFrame()->domWindow();
    ASSERT(window);

    m_frontendWindow = window->open(url, "", "", window, window);
    ASSERT(m_frontendWindow);

    Page* frontendPage = m_frontendWindow->document()->page();
    ASSERT(frontendPage);

    OwnPtr<InspectorFrontendClientLocal> frontendClient = adoptPtr(new InspectorFrontendClientLocal(page->inspectorController(), frontendPage));

    frontendPage->inspectorController().setInspectorFrontendClient(frontendClient.release());

    m_frontendChannel = adoptPtr(new InspectorFrontendChannelDummy(frontendPage));

    page->inspectorController().connectFrontend(m_frontendChannel.get());

    return m_frontendWindow;
}

void Internals::closeDummyInspectorFrontend()
{
    Page* page = contextDocument()->frame()->page();
    ASSERT(page);
    ASSERT(m_frontendWindow);

    page->inspectorController().disconnectFrontend();

    m_frontendChannel.release();

    m_frontendWindow->close(m_frontendWindow->executionContext());
    m_frontendWindow.release();
}

Vector<unsigned long> Internals::setMemoryCacheCapacities(unsigned long minDeadBytes, unsigned long maxDeadBytes, unsigned long totalBytes)
{
    Vector<unsigned long> result;
    result.append(memoryCache()->minDeadCapacity());
    result.append(memoryCache()->maxDeadCapacity());
    result.append(memoryCache()->capacity());
    memoryCache()->setCapacities(minDeadBytes, maxDeadBytes, totalBytes);
    return result;
}

void Internals::setInspectorResourcesDataSizeLimits(int maximumResourcesContentSize, int maximumSingleResourceContentSize, ExceptionState& exceptionState)
{
    Page* page = contextDocument()->frame()->page();
    if (!page) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    page->inspectorController().setResourcesDataSizeLimitsFromInternals(maximumResourcesContentSize, maximumSingleResourceContentSize);
}

bool Internals::hasGrammarMarker(Document* document, int from, int length, ExceptionState&)
{
    if (!document || !document->frame())
        return 0;

    return document->frame()->spellChecker().selectionStartHasMarkerFor(DocumentMarker::Grammar, from, length);
}

unsigned Internals::numberOfScrollableAreas(Document* document, ExceptionState&)
{
    unsigned count = 0;
    Frame* frame = document->frame();
    if (frame->view()->scrollableAreas())
        count += frame->view()->scrollableAreas()->size();

    for (Frame* child = frame->tree().firstChild(); child; child = child->tree().nextSibling()) {
        if (child->view() && child->view()->scrollableAreas())
            count += child->view()->scrollableAreas()->size();
    }

    return count;
}

bool Internals::isPageBoxVisible(Document* document, int pageNumber, ExceptionState& exceptionState)
{
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return false;
    }

    return document->isPageBoxVisible(pageNumber);
}

String Internals::layerTreeAsText(Document* document, ExceptionState& exceptionState) const
{
    return layerTreeAsText(document, 0, exceptionState);
}

String Internals::elementLayerTreeAsText(Element* element, ExceptionState& exceptionState) const
{
    return elementLayerTreeAsText(element, 0, exceptionState);
}

static PassRefPtr<NodeList> paintOrderList(Element* element, ExceptionState& exceptionState, RenderLayerStackingNode::PaintOrderListType type)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    element->document().updateLayout();

    RenderObject* renderer = element->renderer();
    if (!renderer || !renderer->isBox()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    RenderLayer* layer = toRenderBox(renderer)->layer();
    if (!layer) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    Vector<RefPtr<Node> > nodes;
    layer->stackingNode()->computePaintOrderList(type, nodes);
    return StaticNodeList::adopt(nodes);
}

PassRefPtr<NodeList> Internals::paintOrderListBeforePromote(Element* element, ExceptionState& exceptionState)
{
    return paintOrderList(element, exceptionState, RenderLayerStackingNode::BeforePromote);
}

PassRefPtr<NodeList> Internals::paintOrderListAfterPromote(Element* element, ExceptionState& exceptionState)
{
    return paintOrderList(element, exceptionState, RenderLayerStackingNode::AfterPromote);
}

bool Internals::scrollsWithRespectTo(Element* element1, Element* element2, ExceptionState& exceptionState)
{
    if (!element1 || !element2) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    element1->document().updateLayout();

    RenderObject* renderer1 = element1->renderer();
    RenderObject* renderer2 = element2->renderer();
    if (!renderer1 || !renderer2 || !renderer1->isBox() || !renderer2->isBox()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    RenderLayer* layer1 = toRenderBox(renderer1)->layer();
    RenderLayer* layer2 = toRenderBox(renderer2)->layer();
    if (!layer1 || !layer2) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return layer1->scrollsWithRespectTo(layer2);
}

bool Internals::isUnclippedDescendant(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    element->document().updateLayout();

    RenderObject* renderer = element->renderer();
    if (!renderer || !renderer->isBox()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    RenderLayer* layer = toRenderBox(renderer)->layer();
    if (!layer) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return layer->isUnclippedDescendant();
}

bool Internals::needsCompositedScrolling(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    element->document().updateLayout();

    RenderObject* renderer = element->renderer();
    if (!renderer || !renderer->isBox()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    RenderLayer* layer = toRenderBox(renderer)->layer();
    if (!layer) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return layer->needsCompositedScrolling();
}

String Internals::layerTreeAsText(Document* document, unsigned flags, ExceptionState& exceptionState) const
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return document->frame()->layerTreeAsText(flags);
}

String Internals::elementLayerTreeAsText(Element* element, unsigned flags, ExceptionState& exceptionState) const
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    element->document().updateLayout();

    RenderObject* renderer = element->renderer();
    if (!renderer || !renderer->isBox()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    RenderLayer* layer = toRenderBox(renderer)->layer();
    if (!layer
        || !layer->hasCompositedLayerMapping()
        || !layer->compositedLayerMapping()->mainGraphicsLayer()) {
        // Don't raise exception in these cases which may be normally used in tests.
        return String();
    }

    return layer->compositedLayerMapping()->mainGraphicsLayer()->layerTreeAsText(flags);
}

static RenderLayer* getRenderLayerForElement(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    RenderObject* renderer = element->renderer();
    if (!renderer || !renderer->isBox()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    RenderLayer* layer = toRenderBox(renderer)->layer();
    if (!layer) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return layer;
}

void Internals::setNeedsCompositedScrolling(Element* element, unsigned needsCompositedScrolling, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    element->document().updateLayout();

    if (RenderLayer* layer = getRenderLayerForElement(element, exceptionState))
        layer->scrollableArea()->setForceNeedsCompositedScrolling(static_cast<ForceNeedsCompositedScrollingMode>(needsCompositedScrolling));
}

String Internals::repaintRectsAsText(Document* document, ExceptionState& exceptionState) const
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return document->frame()->trackedRepaintRectsAsText();
}

PassRefPtr<ClientRectList> Internals::repaintRects(Element* element, ExceptionState& exceptionState) const
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    if (RenderLayer* layer = getRenderLayerForElement(element, exceptionState)) {
        if (layer->compositingState() == PaintsIntoOwnBacking) {
            OwnPtr<Vector<FloatRect> > rects = layer->collectTrackedRepaintRects();
            ASSERT(rects.get());
            Vector<FloatQuad> quads(rects->size());
            for (size_t i = 0; i < rects->size(); ++i)
                quads[i] = FloatRect(rects->at(i));
            return ClientRectList::create(quads);
        }
    }

    // It's an error to call this on an element that's not composited.
    exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
    return 0;
}

String Internals::scrollingStateTreeAsText(Document* document, ExceptionState& exceptionState) const
{
    return String();
}

String Internals::mainThreadScrollingReasons(Document* document, ExceptionState& exceptionState) const
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    // Force a re-layout and a compositing update.
    document->updateLayout();
    RenderView* view = document->renderView();
    if (view->compositor())
        view->compositor()->updateCompositingLayers(CompositingUpdateFinishAllDeferredWork);

    Page* page = document->page();
    if (!page)
        return String();

    return page->mainThreadScrollingReasonsAsText();
}

PassRefPtr<ClientRectList> Internals::nonFastScrollableRects(Document* document, ExceptionState& exceptionState) const
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    Page* page = document->page();
    if (!page)
        return 0;

    return page->nonFastScrollableRects(document->frame());
}

void Internals::garbageCollectDocumentResources(Document* document, ExceptionState& exceptionState) const
{
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    ResourceFetcher* fetcher = document->fetcher();
    if (!fetcher)
        return;
    fetcher->garbageCollectDocumentResources();
}

void Internals::evictAllResources() const
{
    memoryCache()->evictResources();
}

void Internals::allowRoundingHacks() const
{
    TextRun::setAllowsRoundingHacks(true);
}

String Internals::counterValue(Element* element)
{
    if (!element)
        return String();

    return counterValueForElement(element);
}

int Internals::pageNumber(Element* element, float pageWidth, float pageHeight)
{
    if (!element)
        return 0;

    return PrintContext::pageNumberForElement(element, FloatSize(pageWidth, pageHeight));
}

Vector<String> Internals::iconURLs(Document* document, int iconTypesMask) const
{
    Vector<IconURL> iconURLs = document->iconURLs(iconTypesMask);
    Vector<String> array;

    Vector<IconURL>::const_iterator iter(iconURLs.begin());
    for (; iter != iconURLs.end(); ++iter)
        array.append(iter->m_iconURL.string());

    return array;
}

Vector<String> Internals::shortcutIconURLs(Document* document) const
{
    return iconURLs(document, Favicon);
}

Vector<String> Internals::allIconURLs(Document* document) const
{
    return iconURLs(document, Favicon | TouchIcon | TouchPrecomposedIcon);
}

int Internals::numberOfPages(float pageWidth, float pageHeight)
{
    if (!frame())
        return -1;

    return PrintContext::numberOfPages(frame(), FloatSize(pageWidth, pageHeight));
}

String Internals::pageProperty(String propertyName, int pageNumber, ExceptionState& exceptionState) const
{
    if (!frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return PrintContext::pageProperty(frame(), propertyName.utf8().data(), pageNumber);
}

String Internals::pageSizeAndMarginsInPixels(int pageNumber, int width, int height, int marginTop, int marginRight, int marginBottom, int marginLeft, ExceptionState& exceptionState) const
{
    if (!frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return PrintContext::pageSizeAndMarginsInPixels(frame(), pageNumber, width, height, marginTop, marginRight, marginBottom, marginLeft);
}

void Internals::setDeviceScaleFactor(float scaleFactor, ExceptionState& exceptionState)
{
    Document* document = contextDocument();
    if (!document || !document->page()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    Page* page = document->page();
    page->setDeviceScaleFactor(scaleFactor);
}

void Internals::setIsCursorVisible(Document* document, bool isVisible, ExceptionState& exceptionState)
{
    if (!document || !document->page()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }
    document->page()->setIsCursorVisible(isVisible);
}

void Internals::webkitWillEnterFullScreenForElement(Document* document, Element* element)
{
    if (!document)
        return;
    FullscreenElementStack::from(document)->webkitWillEnterFullScreenForElement(element);
}

void Internals::webkitDidEnterFullScreenForElement(Document* document, Element* element)
{
    if (!document)
        return;
    FullscreenElementStack::from(document)->webkitDidEnterFullScreenForElement(element);
}

void Internals::webkitWillExitFullScreenForElement(Document* document, Element* element)
{
    if (!document)
        return;
    FullscreenElementStack::from(document)->webkitWillExitFullScreenForElement(element);
}

void Internals::webkitDidExitFullScreenForElement(Document* document, Element* element)
{
    if (!document)
        return;
    FullscreenElementStack::from(document)->webkitDidExitFullScreenForElement(element);
}

void Internals::registerURLSchemeAsBypassingContentSecurityPolicy(const String& scheme)
{
    SchemeRegistry::registerURLSchemeAsBypassingContentSecurityPolicy(scheme);
}

void Internals::removeURLSchemeRegisteredAsBypassingContentSecurityPolicy(const String& scheme)
{
    SchemeRegistry::removeURLSchemeRegisteredAsBypassingContentSecurityPolicy(scheme);
}

PassRefPtr<MallocStatistics> Internals::mallocStatistics() const
{
    return MallocStatistics::create();
}

PassRefPtr<TypeConversions> Internals::typeConversions() const
{
    return TypeConversions::create();
}

Vector<String> Internals::getReferencedFilePaths() const
{
    frame()->loader().saveDocumentAndScrollState();
    return FormController::getReferencedFilePaths(frame()->loader().currentItem()->documentState());
}

void Internals::startTrackingRepaints(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    FrameView* frameView = document->view();
    frameView->setTracksRepaints(true);
}

void Internals::stopTrackingRepaints(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    FrameView* frameView = document->view();
    frameView->setTracksRepaints(false);
}

void Internals::updateLayoutIgnorePendingStylesheetsAndRunPostLayoutTasks(ExceptionState& exceptionState)
{
    updateLayoutIgnorePendingStylesheetsAndRunPostLayoutTasks(0, exceptionState);
}

void Internals::updateLayoutIgnorePendingStylesheetsAndRunPostLayoutTasks(Node* node, ExceptionState& exceptionState)
{
    Document* document;
    if (!node) {
        document = contextDocument();
    } else if (node->isDocumentNode()) {
        document = toDocument(node);
    } else if (node->hasTagName(HTMLNames::iframeTag)) {
        document = toHTMLIFrameElement(node)->contentDocument();
    } else {
        exceptionState.throwUninformativeAndGenericDOMException(TypeError);
        return;
    }
    document->updateLayoutIgnorePendingStylesheets(Document::RunPostLayoutTasksSynchronously);
}

PassRefPtr<ClientRectList> Internals::draggableRegions(Document* document, ExceptionState& exceptionState)
{
    return annotatedRegions(document, true, exceptionState);
}

PassRefPtr<ClientRectList> Internals::nonDraggableRegions(Document* document, ExceptionState& exceptionState)
{
    return annotatedRegions(document, false, exceptionState);
}

PassRefPtr<ClientRectList> Internals::annotatedRegions(Document* document, bool draggable, ExceptionState& exceptionState)
{
    if (!document || !document->view()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return ClientRectList::create();
    }

    document->updateLayout();
    document->view()->updateAnnotatedRegions();
    Vector<AnnotatedRegionValue> regions = document->annotatedRegions();

    Vector<FloatQuad> quads;
    for (size_t i = 0; i < regions.size(); ++i) {
        if (regions[i].draggable == draggable)
            quads.append(FloatQuad(regions[i].bounds));
    }
    return ClientRectList::create(quads);
}

static const char* cursorTypeToString(Cursor::Type cursorType)
{
    switch (cursorType) {
    case Cursor::Pointer: return "Pointer";
    case Cursor::Cross: return "Cross";
    case Cursor::Hand: return "Hand";
    case Cursor::IBeam: return "IBeam";
    case Cursor::Wait: return "Wait";
    case Cursor::Help: return "Help";
    case Cursor::EastResize: return "EastResize";
    case Cursor::NorthResize: return "NorthResize";
    case Cursor::NorthEastResize: return "NorthEastResize";
    case Cursor::NorthWestResize: return "NorthWestResize";
    case Cursor::SouthResize: return "SouthResize";
    case Cursor::SouthEastResize: return "SouthEastResize";
    case Cursor::SouthWestResize: return "SouthWestResize";
    case Cursor::WestResize: return "WestResize";
    case Cursor::NorthSouthResize: return "NorthSouthResize";
    case Cursor::EastWestResize: return "EastWestResize";
    case Cursor::NorthEastSouthWestResize: return "NorthEastSouthWestResize";
    case Cursor::NorthWestSouthEastResize: return "NorthWestSouthEastResize";
    case Cursor::ColumnResize: return "ColumnResize";
    case Cursor::RowResize: return "RowResize";
    case Cursor::MiddlePanning: return "MiddlePanning";
    case Cursor::EastPanning: return "EastPanning";
    case Cursor::NorthPanning: return "NorthPanning";
    case Cursor::NorthEastPanning: return "NorthEastPanning";
    case Cursor::NorthWestPanning: return "NorthWestPanning";
    case Cursor::SouthPanning: return "SouthPanning";
    case Cursor::SouthEastPanning: return "SouthEastPanning";
    case Cursor::SouthWestPanning: return "SouthWestPanning";
    case Cursor::WestPanning: return "WestPanning";
    case Cursor::Move: return "Move";
    case Cursor::VerticalText: return "VerticalText";
    case Cursor::Cell: return "Cell";
    case Cursor::ContextMenu: return "ContextMenu";
    case Cursor::Alias: return "Alias";
    case Cursor::Progress: return "Progress";
    case Cursor::NoDrop: return "NoDrop";
    case Cursor::Copy: return "Copy";
    case Cursor::None: return "None";
    case Cursor::NotAllowed: return "NotAllowed";
    case Cursor::ZoomIn: return "ZoomIn";
    case Cursor::ZoomOut: return "ZoomOut";
    case Cursor::Grab: return "Grab";
    case Cursor::Grabbing: return "Grabbing";
    case Cursor::Custom: return "Custom";
    }

    ASSERT_NOT_REACHED();
    return "UNKNOWN";
}

String Internals::getCurrentCursorInfo(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    Cursor cursor = document->frame()->eventHandler().currentMouseCursor();

    StringBuilder result;
    result.append("type=");
    result.append(cursorTypeToString(cursor.type()));
    result.append(" hotSpot=");
    result.appendNumber(cursor.hotSpot().x());
    result.append(",");
    result.appendNumber(cursor.hotSpot().y());
    if (cursor.image()) {
        IntSize size = cursor.image()->size();
        result.append(" image=");
        result.appendNumber(size.width());
        result.append("x");
        result.appendNumber(size.height());
    }
    if (cursor.imageScaleFactor() != 1) {
        result.append(" scale=");
        NumberToStringBuffer buffer;
        result.append(numberToFixedPrecisionString(cursor.imageScaleFactor(), 8, buffer, true));
    }

    return result.toString();
}

PassRefPtr<ArrayBuffer> Internals::serializeObject(PassRefPtr<SerializedScriptValue> value) const
{
    String stringValue = value->toWireString();
    RefPtr<ArrayBuffer> buffer = ArrayBuffer::createUninitialized(stringValue.length(), sizeof(UChar));
    stringValue.copyTo(static_cast<UChar*>(buffer->data()), 0, stringValue.length());
    return buffer.release();
}

PassRefPtr<SerializedScriptValue> Internals::deserializeBuffer(PassRefPtr<ArrayBuffer> buffer) const
{
    String value(static_cast<const UChar*>(buffer->data()), buffer->byteLength() / sizeof(UChar));
    return SerializedScriptValue::createFromWire(value);
}

void Internals::forceReload(bool endToEnd)
{
    frame()->loader().reload(endToEnd ? EndToEndReload : NormalReload);
}

PassRefPtr<ClientRect> Internals::selectionBounds(ExceptionState& exceptionState)
{
    Document* document = contextDocument();
    if (!document || !document->frame()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return 0;
    }

    return ClientRect::create(document->frame()->selection().bounds());
}

String Internals::markerTextForListItem(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }
    return WebCore::markerTextForListItem(element);
}

String Internals::getImageSourceURL(Element* element, ExceptionState& exceptionState)
{
    if (!element) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }
    return element->imageSourceURL();
}

String Internals::baseURL(Document* document, ExceptionState& exceptionState)
{
    if (!document) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return String();
    }

    return document->baseURL().string();
}

bool Internals::isSelectPopupVisible(Node* node)
{
    if (!node->hasTagName(HTMLNames::selectTag))
        return false;

    HTMLSelectElement* select = toHTMLSelectElement(node);

    RenderObject* renderer = select->renderer();
    if (!renderer->isMenuList())
        return false;

    RenderMenuList* menuList = toRenderMenuList(renderer);
    return menuList->popupIsVisible();
}

bool Internals::loseSharedGraphicsContext3D()
{
    RefPtr<GraphicsContext3D> sharedContext = SharedGraphicsContext3D::get();
    if (!sharedContext)
        return false;
    sharedContext->extensions()->loseContextCHROMIUM(Extensions3D::GUILTY_CONTEXT_RESET_ARB, Extensions3D::INNOCENT_CONTEXT_RESET_ARB);
    // To prevent tests that call loseSharedGraphicsContext3D from being
    // flaky, we call finish so that the context is guaranteed to be lost
    // synchronously (i.e. before returning).
    sharedContext->finish();
    return true;
}

void Internals::forceCompositingUpdate(Document* document, ExceptionState& exceptionState)
{
    if (!document || !document->renderView()) {
        exceptionState.throwUninformativeAndGenericDOMException(InvalidAccessError);
        return;
    }

    document->updateLayout();

    RenderView* view = document->renderView();
    if (view->compositor())
        view->compositor()->updateCompositingLayers(CompositingUpdateFinishAllDeferredWork);
}

void Internals::setZoomFactor(float factor)
{
    frame()->setPageZoomFactor(factor);
}

}
