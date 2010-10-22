/*
 * Copyright (C) 2006 Eric Seidel (eric@webkit.org)
 * Copyright (C) 2008, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef EmptyClients_h
#define EmptyClients_h

#include "ChromeClient.h"
#include "Console.h"
#include "ContextMenuClient.h"
#include "DeviceMotionClient.h"
#include "DeviceOrientationClient.h"
#include "DocumentLoader.h"
#include "DragClient.h"
#include "EditCommand.h"
#include "EditorClient.h"
#include "FloatRect.h"
#include "FocusDirection.h"
#include "FrameLoaderClient.h"
#include "FrameNetworkingContext.h"
#include "InspectorClient.h"
#include "PluginHalterClient.h"
#include "PopupMenu.h"
#include "ResourceError.h"
#include "SearchPopupMenu.h"

/*
 This file holds empty Client stubs for use by WebCore.
 Viewless element needs to create a dummy Page->Frame->FrameView tree for use in parsing or executing JavaScript.
 This tree depends heavily on Clients (usually provided by WebKit classes).

 This file was first created for SVGImage as it had no way to access the current Page (nor should it,
 since Images are not tied to a page).
 See http://bugs.webkit.org/show_bug.cgi?id=5971 for the original discussion about this file.

 Ideally, whenever you change a Client class, you should add a stub here.
 Brittle, yes.  Unfortunate, yes.  Hopefully temporary.
*/

namespace WebCore {

class SharedGraphicsContext3D;

class EmptyPopupMenu : public PopupMenu {
public:
    virtual void show(const IntRect&, FrameView*, int) {}
    virtual void hide() {}
    virtual void updateFromElement() {}
    virtual void disconnectClient() {}
};

class EmptySearchPopupMenu : public SearchPopupMenu {
public:
    virtual PopupMenu* popupMenu() { return m_popup.get(); }
    virtual void saveRecentSearches(const AtomicString&, const Vector<String>&) {}
    virtual void loadRecentSearches(const AtomicString&, Vector<String>&) {}
    virtual bool enabled() { return false; }

private:
    RefPtr<EmptyPopupMenu> m_popup;
};

class EmptyChromeClient : public ChromeClient {
public:
    virtual ~EmptyChromeClient() { }
    virtual void chromeDestroyed() { }

    virtual void setWindowRect(const FloatRect&) { }
    virtual FloatRect windowRect() { return FloatRect(); }

    virtual FloatRect pageRect() { return FloatRect(); }

    virtual float scaleFactor() { return 1.f; }

    virtual void focus() { }
    virtual void unfocus() { }

    virtual bool canTakeFocus(FocusDirection) { return false; }
    virtual void takeFocus(FocusDirection) { }

    virtual void focusedNodeChanged(Node*) { }

    virtual Page* createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures&, const NavigationAction&) { return 0; }
    virtual void show() { }

    virtual bool canRunModal() { return false; }
    virtual void runModal() { }

    virtual void setToolbarsVisible(bool) { }
    virtual bool toolbarsVisible() { return false; }

    virtual void setStatusbarVisible(bool) { }
    virtual bool statusbarVisible() { return false; }

    virtual void setScrollbarsVisible(bool) { }
    virtual bool scrollbarsVisible() { return false; }

    virtual void setMenubarVisible(bool) { }
    virtual bool menubarVisible() { return false; }

    virtual void setResizable(bool) { }

    virtual void addMessageToConsole(MessageSource, MessageType, MessageLevel, const String&, unsigned, const String&) { }

    virtual bool canRunBeforeUnloadConfirmPanel() { return false; }
    virtual bool runBeforeUnloadConfirmPanel(const String&, Frame*) { return true; }

    virtual void closeWindowSoon() { }

    virtual void runJavaScriptAlert(Frame*, const String&) { }
    virtual bool runJavaScriptConfirm(Frame*, const String&) { return false; }
    virtual bool runJavaScriptPrompt(Frame*, const String&, const String&, String&) { return false; }
    virtual bool shouldInterruptJavaScript() { return false; }

    virtual bool selectItemWritingDirectionIsNatural() { return false; }
    virtual PassRefPtr<PopupMenu> createPopupMenu(PopupMenuClient*) const { return adoptRef(new EmptyPopupMenu()); }
    virtual PassRefPtr<SearchPopupMenu> createSearchPopupMenu(PopupMenuClient*) const { return adoptRef(new EmptySearchPopupMenu()); }

    virtual void setStatusbarText(const String&) { }

    virtual bool tabsToLinks() const { return false; }

    virtual IntRect windowResizerRect() const { return IntRect(); }

    virtual void invalidateWindow(const IntRect&, bool) { }
    virtual void invalidateContentsAndWindow(const IntRect&, bool) { }
    virtual void invalidateContentsForSlowScroll(const IntRect&, bool) {};
    virtual void scroll(const IntSize&, const IntRect&, const IntRect&) { }

    virtual IntPoint screenToWindow(const IntPoint& p) const { return p; }
    virtual IntRect windowToScreen(const IntRect& r) const { return r; }
    virtual PlatformPageClient platformPageClient() const { return 0; }
    virtual void contentsSizeChanged(Frame*, const IntSize&) const { }

    virtual void scrollbarsModeDidChange() const { }
    virtual void mouseDidMoveOverElement(const HitTestResult&, unsigned) { }

    virtual void setToolTip(const String&, TextDirection) { }

    virtual void print(Frame*) { }

#if ENABLE(DATABASE)
    virtual void exceededDatabaseQuota(Frame*, const String&) { }
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    virtual void reachedMaxAppCacheSize(int64_t) { }
    virtual void reachedApplicationCacheOriginQuota(SecurityOrigin*) { }
#endif

#if ENABLE(NOTIFICATIONS)
    virtual NotificationPresenter* notificationPresenter() const { return 0; }
#endif

    virtual void runOpenPanel(Frame*, PassRefPtr<FileChooser>) { }
    virtual void chooseIconForFiles(const Vector<String>&, FileChooser*) { }

    virtual void formStateDidChange(const Node*) { }

    virtual void formDidFocus(const Node*) { }
    virtual void formDidBlur(const Node*) { }

    virtual PassOwnPtr<HTMLParserQuirks> createHTMLParserQuirks() { return 0; }

    virtual void setCursor(const Cursor&) { }

    virtual void scrollRectIntoView(const IntRect&, const ScrollView*) const {}

    virtual void requestGeolocationPermissionForFrame(Frame*, Geolocation*) {}
    virtual void cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation*) {}

#if USE(ACCELERATED_COMPOSITING)
    virtual void attachRootGraphicsLayer(Frame*, GraphicsLayer*) {}
    virtual void setNeedsOneShotDrawingSynchronization() {}
    virtual void scheduleCompositingLayerSync() {}
#endif

#if PLATFORM(WIN)
    virtual void setLastSetCursorToCurrentCursor() { }
#endif
#if ENABLE(TOUCH_EVENTS)
    virtual void needTouchEvents(bool) { }
#endif
};

class EmptyFrameLoaderClient : public FrameLoaderClient, public Noncopyable {
public:
    virtual ~EmptyFrameLoaderClient() {  }
    virtual void frameLoaderDestroyed() { }

    virtual bool hasWebView() const { return true; } // mainly for assertions

    virtual void makeRepresentation(DocumentLoader*) { }
    virtual void forceLayout() { }
    virtual void forceLayoutForNonHTML() { }

    virtual void setCopiesOnScroll() { }

    virtual void detachedFromParent2() { }
    virtual void detachedFromParent3() { }

    virtual void download(ResourceHandle*, const ResourceRequest&, const ResourceRequest&, const ResourceResponse&) { }

    virtual void assignIdentifierToInitialRequest(unsigned long, DocumentLoader*, const ResourceRequest&) { }
    virtual bool shouldUseCredentialStorage(DocumentLoader*, unsigned long) { return false; }
    virtual void dispatchWillSendRequest(DocumentLoader*, unsigned long, ResourceRequest&, const ResourceResponse&) { }
    virtual void dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&) { }
    virtual void dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&) { }
#if USE(PROTECTION_SPACE_AUTH_CALLBACK)
    virtual bool canAuthenticateAgainstProtectionSpace(DocumentLoader*, unsigned long, const ProtectionSpace&) { return false; }
#endif
    virtual void dispatchDidReceiveResponse(DocumentLoader*, unsigned long, const ResourceResponse&) { }
    virtual void dispatchDidReceiveContentLength(DocumentLoader*, unsigned long, int) { }
    virtual void dispatchDidFinishLoading(DocumentLoader*, unsigned long) { }
    virtual void dispatchDidFailLoading(DocumentLoader*, unsigned long, const ResourceError&) { }
    virtual bool dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const ResourceRequest&, const ResourceResponse&, int) { return false; }

    virtual void dispatchDidHandleOnloadEvents() { }
    virtual void dispatchDidReceiveServerRedirectForProvisionalLoad() { }
    virtual void dispatchDidCancelClientRedirect() { }
    virtual void dispatchWillPerformClientRedirect(const KURL&, double, double) { }
    virtual void dispatchDidChangeLocationWithinPage() { }
    virtual void dispatchDidPushStateWithinPage() { }
    virtual void dispatchDidReplaceStateWithinPage() { }
    virtual void dispatchDidPopStateWithinPage() { }
    virtual void dispatchWillClose() { }
    virtual void dispatchDidReceiveIcon() { }
    virtual void dispatchDidStartProvisionalLoad() { }
    virtual void dispatchDidReceiveTitle(const String&) { }
    virtual void dispatchDidChangeIcons() { }
    virtual void dispatchDidCommitLoad() { }
    virtual void dispatchDidFailProvisionalLoad(const ResourceError&) { }
    virtual void dispatchDidFailLoad(const ResourceError&) { }
    virtual void dispatchDidFinishDocumentLoad() { }
    virtual void dispatchDidFinishLoad() { }
    virtual void dispatchDidFirstLayout() { }
    virtual void dispatchDidFirstVisuallyNonEmptyLayout() { }

    virtual Frame* dispatchCreatePage(const NavigationAction&) { return 0; }
    virtual void dispatchShow() { }

    virtual void dispatchDecidePolicyForMIMEType(FramePolicyFunction, const String&, const ResourceRequest&) { }
    virtual void dispatchDecidePolicyForNewWindowAction(FramePolicyFunction, const NavigationAction&, const ResourceRequest&, PassRefPtr<FormState>, const String&) { }
    virtual void dispatchDecidePolicyForNavigationAction(FramePolicyFunction, const NavigationAction&, const ResourceRequest&, PassRefPtr<FormState>) { }
    virtual void cancelPolicyCheck() { }

    virtual void dispatchUnableToImplementPolicy(const ResourceError&) { }

    virtual void dispatchWillSendSubmitEvent(HTMLFormElement*) { }
    virtual void dispatchWillSubmitForm(FramePolicyFunction, PassRefPtr<FormState>) { }

    virtual void dispatchDidLoadMainResource(DocumentLoader*) { }
    virtual void revertToProvisionalState(DocumentLoader*) { }
    virtual void setMainDocumentError(DocumentLoader*, const ResourceError&) { }

    virtual void willChangeEstimatedProgress() { }
    virtual void didChangeEstimatedProgress() { }
    virtual void postProgressStartedNotification() { }
    virtual void postProgressEstimateChangedNotification() { }
    virtual void postProgressFinishedNotification() { }

    virtual void setMainFrameDocumentReady(bool) { }

    virtual void startDownload(const ResourceRequest&) { }

    virtual void willChangeTitle(DocumentLoader*) { }
    virtual void didChangeTitle(DocumentLoader*) { }

    virtual void committedLoad(DocumentLoader*, const char*, int) { }
    virtual void finishedLoading(DocumentLoader*) { }

    virtual ResourceError cancelledError(const ResourceRequest&) { ResourceError error("", 0, "", ""); error.setIsCancellation(true); return error; }
    virtual ResourceError blockedError(const ResourceRequest&) { return ResourceError("", 0, "", ""); }
    virtual ResourceError cannotShowURLError(const ResourceRequest&) { return ResourceError("", 0, "", ""); }
    virtual ResourceError interruptForPolicyChangeError(const ResourceRequest&) { return ResourceError("", 0, "", ""); }

    virtual ResourceError cannotShowMIMETypeError(const ResourceResponse&) { return ResourceError("", 0, "", ""); }
    virtual ResourceError fileDoesNotExistError(const ResourceResponse&) { return ResourceError("", 0, "", ""); }
    virtual ResourceError pluginWillHandleLoadError(const ResourceResponse&) { return ResourceError("", 0, "", ""); }

    virtual bool shouldFallBack(const ResourceError&) { return false; }

    virtual bool canHandleRequest(const ResourceRequest&) const { return false; }
    virtual bool canShowMIMEType(const String&) const { return false; }
    virtual bool canShowMIMETypeAsHTML(const String&) const { return false; }
    virtual bool representationExistsForURLScheme(const String&) const { return false; }
    virtual String generatedMIMETypeForURLScheme(const String&) const { return ""; }

    virtual void frameLoadCompleted() { }
    virtual void restoreViewState() { }
    virtual void provisionalLoadStarted() { }
    virtual bool shouldTreatURLAsSameAsCurrent(const KURL&) const { return false; }
    virtual void didFinishLoad() { }
    virtual void prepareForDataSourceReplacement() { }

    virtual PassRefPtr<DocumentLoader> createDocumentLoader(const ResourceRequest& request, const SubstituteData& substituteData) { return DocumentLoader::create(request, substituteData); }
    virtual void setTitle(const String&, const KURL&) { }

    virtual String userAgent(const KURL&) { return ""; }

    virtual void savePlatformDataToCachedFrame(CachedFrame*) { }
    virtual void transitionToCommittedFromCachedFrame(CachedFrame*) { }
    virtual void transitionToCommittedForNewPage() { }    

    virtual void updateGlobalHistory() { }
    virtual void updateGlobalHistoryRedirectLinks() { }
    virtual bool shouldGoToHistoryItem(HistoryItem*) const { return false; }
    virtual void dispatchDidAddBackForwardItem(HistoryItem*) const { }
    virtual void dispatchDidRemoveBackForwardItem(HistoryItem*) const { };
    virtual void dispatchDidChangeBackForwardIndex() const { }
    virtual void saveViewStateToItem(HistoryItem*) { }
    virtual bool canCachePage() const { return false; }
    virtual void didDisplayInsecureContent() { }
    virtual void didRunInsecureContent(SecurityOrigin*) { }
    virtual PassRefPtr<Frame> createFrame(const KURL&, const String&, HTMLFrameOwnerElement*, const String&, bool, int, int) { return 0; }
    virtual void didTransferChildFrameToNewDocument(Page*) { }
    virtual PassRefPtr<Widget> createPlugin(const IntSize&, HTMLPlugInElement*, const KURL&, const Vector<String>&, const Vector<String>&, const String&, bool) { return 0; }
    virtual PassRefPtr<Widget> createJavaAppletWidget(const IntSize&, HTMLAppletElement*, const KURL&, const Vector<String>&, const Vector<String>&) { return 0; }
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    virtual PassRefPtr<Widget> createMediaPlayerProxyPlugin(const IntSize&, HTMLMediaElement*, const KURL&, const Vector<String>&, const Vector<String>&, const String&) { return 0; }
    virtual void hideMediaPlayerProxyPlugin(Widget*) { }
    virtual void showMediaPlayerProxyPlugin(Widget*) { }
#endif

    virtual ObjectContentType objectContentType(const KURL&, const String&) { return ObjectContentType(); }
    virtual String overrideMediaType() const { return String(); }

    virtual void redirectDataToPlugin(Widget*) { }
    virtual void dispatchDidClearWindowObjectInWorld(DOMWrapperWorld*) { }
    virtual void documentElementAvailable() { }
    virtual void didPerformFirstNavigation() const { }

    virtual void registerForIconNotification(bool) { }

#if USE(V8)
    virtual void didCreateScriptContextForFrame() { }
    virtual void didDestroyScriptContextForFrame() { }
    virtual void didCreateIsolatedScriptContext() { }
    virtual bool allowScriptExtension(const String& extensionName, int extensionGroup) { return false; }
#endif

#if PLATFORM(MAC)
    virtual NSCachedURLResponse* willCacheResponse(DocumentLoader*, unsigned long, NSCachedURLResponse* response) const { return response; }
#endif
#if USE(CFNETWORK)
    virtual bool shouldCacheResponse(DocumentLoader*, unsigned long, const ResourceResponse&, const unsigned char*, unsigned long long) { return true; }
#endif

    virtual PassRefPtr<FrameNetworkingContext> createNetworkingContext() { return PassRefPtr<FrameNetworkingContext>(); }
};

class EmptyEditorClient : public EditorClient, public Noncopyable {
public:
    virtual ~EmptyEditorClient() { }
    virtual void pageDestroyed() { }

    virtual bool shouldDeleteRange(Range*) { return false; }
    virtual bool shouldShowDeleteInterface(HTMLElement*) { return false; }
    virtual bool smartInsertDeleteEnabled() { return false; }
    virtual bool isSelectTrailingWhitespaceEnabled() { return false; }
    virtual bool isContinuousSpellCheckingEnabled() { return false; }
    virtual void toggleContinuousSpellChecking() { }
    virtual bool isGrammarCheckingEnabled() { return false; }
    virtual void toggleGrammarChecking() { }
    virtual int spellCheckerDocumentTag() { return -1; }

    virtual bool selectWordBeforeMenuEvent() { return false; }
    virtual bool isEditable() { return false; }

    virtual bool shouldBeginEditing(Range*) { return false; }
    virtual bool shouldEndEditing(Range*) { return false; }
    virtual bool shouldInsertNode(Node*, Range*, EditorInsertAction) { return false; }
    //  virtual bool shouldInsertNode(Node*, Range* replacingRange, WebViewInsertAction) { return false; }
    virtual bool shouldInsertText(const String&, Range*, EditorInsertAction) { return false; }
    virtual bool shouldChangeSelectedRange(Range*, Range*, EAffinity, bool) { return false; }

    virtual bool shouldApplyStyle(CSSStyleDeclaration*, Range*) { return false; }
    virtual bool shouldMoveRangeAfterDelete(Range*, Range*) { return false; }
    //  virtual bool shouldChangeTypingStyle(CSSStyleDeclaration* fromStyle, CSSStyleDeclaration* toStyle) { return false; }
    //  virtual bool doCommandBySelector(SEL selector) { return false; }
    //
    virtual void didBeginEditing() { }
    virtual void respondToChangedContents() { }
    virtual void respondToChangedSelection() { }
    virtual void didEndEditing() { }
    virtual void didWriteSelectionToPasteboard() { }
    virtual void didSetSelectionTypesForPasteboard() { }
    //  virtual void webViewDidChangeTypingStyle:(NSNotification *)notification { }
    //  virtual void webViewDidChangeSelection:(NSNotification *)notification { }
    //  virtual NSUndoManager* undoManagerForWebView:(WebView *)webView { return 0; }

    virtual void registerCommandForUndo(PassRefPtr<EditCommand>) { }
    virtual void registerCommandForRedo(PassRefPtr<EditCommand>) { }
    virtual void clearUndoRedoOperations() { }

    virtual bool canUndo() const { return false; }
    virtual bool canRedo() const { return false; }

    virtual void undo() { }
    virtual void redo() { }

    virtual void handleKeyboardEvent(KeyboardEvent*) { }
    virtual void handleInputMethodKeydown(KeyboardEvent*) { }

    virtual void textFieldDidBeginEditing(Element*) { }
    virtual void textFieldDidEndEditing(Element*) { }
    virtual void textDidChangeInTextField(Element*) { }
    virtual bool doTextFieldCommandFromEvent(Element*, KeyboardEvent*) { return false; }
    virtual void textWillBeDeletedInTextField(Element*) { }
    virtual void textDidChangeInTextArea(Element*) { }

#if PLATFORM(MAC)
    virtual void markedTextAbandoned(Frame*) { }

    virtual NSString* userVisibleString(NSURL*) { return 0; }
    virtual DocumentFragment* documentFragmentFromAttributedString(NSAttributedString*, Vector<RefPtr<ArchiveResource> >&) { return 0; };
    virtual void setInsertionPasteboard(NSPasteboard*) { };
#ifdef BUILDING_ON_TIGER
    virtual NSArray* pasteboardTypesForSelection(Frame*) { return 0; }
#endif
#endif
#if PLATFORM(MAC) && !defined(BUILDING_ON_TIGER) && !defined(BUILDING_ON_LEOPARD)
    virtual void uppercaseWord() { }
    virtual void lowercaseWord() { }
    virtual void capitalizeWord() { }
    virtual void showSubstitutionsPanel(bool) { }
    virtual bool substitutionsPanelIsShowing() { return false; }
    virtual void toggleSmartInsertDelete() { }
    virtual bool isAutomaticQuoteSubstitutionEnabled() { return false; }
    virtual void toggleAutomaticQuoteSubstitution() { }
    virtual bool isAutomaticLinkDetectionEnabled() { return false; }
    virtual void toggleAutomaticLinkDetection() { }
    virtual bool isAutomaticDashSubstitutionEnabled() { return false; }
    virtual void toggleAutomaticDashSubstitution() { }
    virtual bool isAutomaticTextReplacementEnabled() { return false; }
    virtual void toggleAutomaticTextReplacement() { }
    virtual bool isAutomaticSpellingCorrectionEnabled() { return false; }
    virtual void toggleAutomaticSpellingCorrection() { }
#endif
    virtual void ignoreWordInSpellDocument(const String&) { }
    virtual void learnWord(const String&) { }
    virtual void checkSpellingOfString(const UChar*, int, int*, int*) { }
    virtual String getAutoCorrectSuggestionForMisspelledWord(const String&) { return String(); }
    virtual void checkGrammarOfString(const UChar*, int, Vector<GrammarDetail>&, int*, int*) { }
#if PLATFORM(MAC) && !defined(BUILDING_ON_TIGER) && !defined(BUILDING_ON_LEOPARD)
    virtual void checkTextOfParagraph(const UChar*, int, uint64_t, Vector<TextCheckingResult>&) { };
#endif
#if PLATFORM(MAC) && !defined(BUILDING_ON_TIGER) && !defined(BUILDING_ON_LEOPARD) && !defined(BUILDING_ON_SNOW_LEOPARD)
    virtual void showCorrectionPanel(const FloatRect&, const String&, const String&, Editor*) { }
    virtual void dismissCorrectionPanel(bool) { }
    virtual bool isShowingCorrectionPanel() { return false; }
#endif
    virtual void updateSpellingUIWithGrammarString(const String&, const GrammarDetail&) { }
    virtual void updateSpellingUIWithMisspelledWord(const String&) { }
    virtual void showSpellingUI(bool) { }
    virtual bool spellingUIIsShowing() { return false; }
    virtual void getGuessesForWord(const String&, Vector<String>&) { }
    virtual void willSetInputMethodState() { }
    virtual void setInputMethodState(bool) { }


};

#if ENABLE(CONTEXT_MENUS)
class EmptyContextMenuClient : public ContextMenuClient, public Noncopyable {
public:
    virtual ~EmptyContextMenuClient() {  }
    virtual void contextMenuDestroyed() { }

    virtual PlatformMenuDescription getCustomMenuFromDefaultItems(ContextMenu*) { return 0; }
    virtual void contextMenuItemSelected(ContextMenuItem*, const ContextMenu*) { }

    virtual void downloadURL(const KURL&) { }
    virtual void copyImageToClipboard(const HitTestResult&) { }
    virtual void searchWithGoogle(const Frame*) { }
    virtual void lookUpInDictionary(Frame*) { }
    virtual bool isSpeaking() { return false; }
    virtual void speak(const String&) { }
    virtual void stopSpeaking() { }

#if PLATFORM(MAC)
    virtual void searchWithSpotlight() { }
#endif
};
#endif // ENABLE(CONTEXT_MENUS)

#if ENABLE(DRAG_SUPPORT)
class EmptyDragClient : public DragClient, public Noncopyable {
public:
    virtual ~EmptyDragClient() {}
    virtual void willPerformDragDestinationAction(DragDestinationAction, DragData*) { }
    virtual void willPerformDragSourceAction(DragSourceAction, const IntPoint&, Clipboard*) { }
    virtual DragDestinationAction actionMaskForDrag(DragData*) { return DragDestinationActionNone; }
    virtual DragSourceAction dragSourceActionMaskForPoint(const IntPoint&) { return DragSourceActionNone; }
    virtual void startDrag(DragImageRef, const IntPoint&, const IntPoint&, Clipboard*, Frame*, bool) { }
    virtual DragImageRef createDragImageForLink(KURL&, const String&, Frame*) { return 0; }
    virtual void dragControllerDestroyed() { }
};
#endif // ENABLE(DRAG_SUPPORT)

class EmptyInspectorClient : public InspectorClient, public Noncopyable {
public:
    virtual ~EmptyInspectorClient() { }

    virtual void inspectorDestroyed() { }
    
    virtual void openInspectorFrontend(InspectorController*) { }

    virtual void highlight(Node*) { }
    virtual void hideHighlight() { }

    virtual void populateSetting(const String&, String*) { }
    virtual void storeSetting(const String&, const String&) { }
    virtual bool sendMessageToFrontend(const String&) { return false; }
};

class EmptyDeviceMotionClient : public DeviceMotionClient {
public:
    virtual void setController(DeviceMotionController*) { }
    virtual void startUpdating() { }
    virtual void stopUpdating() { }
    virtual DeviceMotionData* currentDeviceMotion() const { return 0; }
    virtual void deviceMotionControllerDestroyed() { }
};

class EmptyDeviceOrientationClient : public DeviceOrientationClient {
public:
    virtual void setController(DeviceOrientationController*) { }
    virtual void startUpdating() { }
    virtual void stopUpdating() { }
    virtual DeviceOrientation* lastOrientation() const { return 0; }
    virtual void deviceOrientationControllerDestroyed() { }
};

}

#endif // EmptyClients_h
