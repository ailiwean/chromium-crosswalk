/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef InspectorResourceAgent_h
#define InspectorResourceAgent_h

#include "InspectorFrontend.h"
#include "PlatformString.h"

#include <wtf/PassRefPtr.h>
#include <wtf/Vector.h>

#if ENABLE(INSPECTOR)

namespace WTF {
class String;
}

namespace WebCore {

class CachedResource;
class Document;
class DocumentLoader;
class EventsCollector;
class Frame;
class InspectorArray;
class InspectorFrontend;
class InspectorFrontendProxy;
class InspectorObject;
class InspectorPageAgent;
class InspectorState;
class InstrumentingAgents;
class KURL;
class NetworkResourcesData;
class Page;
class ResourceError;
class ResourceRequest;
class ResourceResponse;
class SharedBuffer;

#if ENABLE(WEB_SOCKETS)
class WebSocketHandshakeRequest;
class WebSocketHandshakeResponse;
#endif

typedef String ErrorString;

class InspectorResourceAgent : public RefCounted<InspectorResourceAgent> {
public:
    static PassRefPtr<InspectorResourceAgent> create(InstrumentingAgents* instrumentingAgents, InspectorPageAgent* pageAgent, InspectorState* state)
    {
        return adoptRef(new InspectorResourceAgent(instrumentingAgents, pageAgent, state));
    }

    void setFrontend(InspectorFrontend*);
    void clearFrontend();
    void restore();

    static PassRefPtr<InspectorResourceAgent> restore(Page*, InspectorState*, InspectorFrontend*);

    ~InspectorResourceAgent();

    void willSendRequest(unsigned long identifier, DocumentLoader*, ResourceRequest&, const ResourceResponse& redirectResponse);
    void markResourceAsCached(unsigned long identifier);
    void didReceiveResponse(unsigned long identifier, DocumentLoader* laoder, const ResourceResponse&);
    void didReceiveContentLength(unsigned long identifier, int dataLength, int encodedDataLength);
    void didFinishLoading(unsigned long identifier, double finishTime);
    void didFailLoading(unsigned long identifier, const ResourceError&);
    void didLoadResourceFromMemoryCache(DocumentLoader*, const CachedResource*);
    void mainFrameNavigated(DocumentLoader*);
    void setInitialScriptContent(unsigned long identifier, const String& sourceString);
    void setInitialXHRContent(unsigned long identifier, const String& sourceString);
    void didReceiveXHRResponse(unsigned long identifier);
    void willLoadXHRSynchronously();
    void didLoadXHRSynchronously();

    void applyUserAgentOverride(String* userAgent);

#if ENABLE(WEB_SOCKETS)
    void didCreateWebSocket(unsigned long identifier, const KURL& requestURL);
    void willSendWebSocketHandshakeRequest(unsigned long identifier, const WebSocketHandshakeRequest&);
    void didReceiveWebSocketHandshakeResponse(unsigned long identifier, const WebSocketHandshakeResponse&);
    void didCloseWebSocket(unsigned long identifier);
#endif

    void isBackgroundEventsCollectionEnabled(ErrorString*, bool* enabled);
    void setBackgroundEventsCollectionEnabled(ErrorString*, bool enabled);

    // Called from frontend
    void enable(ErrorString*);
    void disable(ErrorString*);
    void setUserAgentOverride(ErrorString*, const String& userAgent);
    void setExtraHeaders(ErrorString*, PassRefPtr<InspectorObject>);
    void getResourceContent(ErrorString*, int identifier, const bool* const base64Encode, String* content);
    void clearCache(ErrorString*, const String* const optionalPreservedLoaderId);

private:
    InspectorResourceAgent(InstrumentingAgents*, InspectorPageAgent*, InspectorState*);

    bool isBackgroundEventsCollectionEnabled();
    void enable();
    void initializeBackgroundCollection();

    InstrumentingAgents* m_instrumentingAgents;
    InspectorPageAgent* m_pageAgent;
    InspectorState* m_state;
    InspectorFrontend::Network* m_frontend;
    OwnPtr<EventsCollector> m_eventsCollector;
    OwnPtr<InspectorFrontendProxy> m_inspectorFrontendProxy;
    OwnPtr<InspectorFrontend::Network> m_mockFrontend;
    String m_userAgentOverride;
    OwnPtr<NetworkResourcesData> m_resourcesData;
    bool m_loadingXHRSynchronously;
};

} // namespace WebCore

#endif // ENABLE(INSPECTOR)

#endif // !defined(InspectorResourceAgent_h)
