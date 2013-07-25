/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
    Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#ifndef ResourceFetcher_h
#define ResourceFetcher_h

#include "core/loader/ResourceLoaderHost.h"
#include "core/loader/cache/CachePolicy.h"
#include "core/loader/cache/CachedResource.h"
#include "core/loader/cache/CachedResourceHandle.h"
#include "core/loader/cache/CachedResourceInitiatorInfo.h"
#include "core/loader/cache/FetchRequest.h"
#include "core/platform/Timer.h"
#include "wtf/Deque.h"
#include "wtf/HashMap.h"
#include "wtf/HashSet.h"
#include "wtf/ListHashSet.h"
#include "wtf/text/StringHash.h"

namespace WebCore {

class CachedCSSStyleSheet;
class CachedDocument;
class CachedFont;
class CachedImage;
class CachedRawResource;
class CachedScript;
class CachedShader;
class CachedTextTrack;
class CachedXSLStyleSheet;
class Document;
class DocumentLoader;
class Frame;
class FrameLoader;
class ImageLoader;
class KURL;
class ResourceTimingInfo;

// The ResourceFetcher provides a per-context interface to the MemoryCache
// and enforces a bunch of security checks and rules for resource revalidation.
// Its lifetime is roughly per-DocumentLoader, in that it is generally created
// in the DocumentLoader constructor and loses its ability to generate network
// requests when the DocumentLoader is destroyed. Documents also hold a
// RefPtr<ResourceFetcher> for their lifetime (and will create one if they
// are initialized without a Frame), so a Document can keep a ResourceFetcher
// alive past detach if scripts still reference the Document.
class ResourceFetcher : public RefCounted<ResourceFetcher>, public ResourceLoaderHost {
    WTF_MAKE_NONCOPYABLE(ResourceFetcher); WTF_MAKE_FAST_ALLOCATED;
friend class ImageLoader;
friend class ResourceCacheValidationSuppressor;

public:
    static PassRefPtr<ResourceFetcher> create(DocumentLoader* documentLoader) { return adoptRef(new ResourceFetcher(documentLoader)); }
    virtual ~ResourceFetcher();

    using RefCounted<ResourceFetcher>::ref;
    using RefCounted<ResourceFetcher>::deref;

    CachedResourceHandle<CachedImage> requestImage(FetchRequest&);
    CachedResourceHandle<CachedCSSStyleSheet> requestCSSStyleSheet(FetchRequest&);
    CachedResourceHandle<CachedCSSStyleSheet> requestUserCSSStyleSheet(FetchRequest&);
    CachedResourceHandle<CachedScript> requestScript(FetchRequest&);
    CachedResourceHandle<CachedFont> requestFont(FetchRequest&);
    CachedResourceHandle<CachedRawResource> requestRawResource(FetchRequest&);
    CachedResourceHandle<CachedRawResource> requestMainResource(FetchRequest&);
    CachedResourceHandle<CachedDocument> requestSVGDocument(FetchRequest&);
    CachedResourceHandle<CachedXSLStyleSheet> requestXSLStyleSheet(FetchRequest&);
    CachedResourceHandle<CachedResource> requestLinkResource(CachedResource::Type, FetchRequest&);
    CachedResourceHandle<CachedTextTrack> requestTextTrack(FetchRequest&);
    CachedResourceHandle<CachedShader> requestShader(FetchRequest&);
    CachedResourceHandle<CachedRawResource> requestImport(FetchRequest&);

    // Logs an access denied message to the console for the specified URL.
    void printAccessDeniedMessage(const KURL&) const;

    CachedResource* cachedResource(const String& url) const;
    CachedResource* cachedResource(const KURL&) const;

    typedef HashMap<String, CachedResourceHandle<CachedResource> > DocumentResourceMap;
    const DocumentResourceMap& allCachedResources() const { return m_documentResources; }

    bool autoLoadImages() const { return m_autoLoadImages; }
    void setAutoLoadImages(bool);

    void setImagesEnabled(bool);

    bool shouldDeferImageLoad(const KURL&) const;

    CachePolicy cachePolicy(CachedResource::Type) const;

    Frame* frame() const; // Can be null
    Document* document() const { return m_document; } // Can be null
    void setDocument(Document* document) { m_document = document; }

    DocumentLoader* documentLoader() const { return m_documentLoader; }
    void clearDocumentLoader() { m_documentLoader = 0; }

    void garbageCollectDocumentResources();

    int requestCount() const { return m_requestCount; }

    bool isPreloaded(const String& urlString) const;
    void clearPreloads();
    void clearPendingPreloads();
    void preload(CachedResource::Type, FetchRequest&, const String& charset);
    void checkForPendingPreloads();
    void printPreloadStats();
    bool canRequest(CachedResource::Type, const KURL&, const ResourceLoaderOptions&, bool forPreload = false);
    bool canAccess(CachedResource*);

    // ResourceLoaderHost
    virtual void incrementRequestCount(const CachedResource*) OVERRIDE;
    virtual void decrementRequestCount(const CachedResource*) OVERRIDE;
    virtual void didLoadResource(CachedResource*) OVERRIDE;
    virtual void redirectReceived(CachedResource*, const ResourceResponse&) OVERRIDE;
    virtual void didFinishLoading(const CachedResource*, double finishTime, const ResourceLoaderOptions&) OVERRIDE;
    virtual void didChangeLoadingPriority(const CachedResource*, ResourceLoadPriority) OVERRIDE;
    virtual void didFailLoading(const CachedResource*, const ResourceError&, const ResourceLoaderOptions&) OVERRIDE;
    virtual void willSendRequest(const CachedResource*, ResourceRequest&, const ResourceResponse& redirectResponse, const ResourceLoaderOptions&) OVERRIDE;
    virtual void didReceiveResponse(const CachedResource*, const ResourceResponse&, const ResourceLoaderOptions&) OVERRIDE;
    virtual void didReceiveData(const CachedResource*, const char* data, int dataLength, int encodedDataLength, const ResourceLoaderOptions&) OVERRIDE;
    virtual void subresourceLoaderFinishedLoadingOnePart(ResourceLoader*) OVERRIDE;
    virtual void didInitializeResourceLoader(ResourceLoader*) OVERRIDE;
    virtual void willTerminateResourceLoader(ResourceLoader*) OVERRIDE;
    virtual void willStartLoadingResource(ResourceRequest&) OVERRIDE;
    virtual bool defersLoading() const OVERRIDE;
    virtual bool isLoadedBy(ResourceLoaderHost*) const OVERRIDE;
    virtual bool shouldRequest(CachedResource*, const ResourceRequest&, const ResourceLoaderOptions&) OVERRIDE;
    virtual void refResourceLoaderHost() OVERRIDE;
    virtual void derefResourceLoaderHost() OVERRIDE;

    static const ResourceLoaderOptions& defaultCachedResourceOptions();
private:

    explicit ResourceFetcher(DocumentLoader*);

    FrameLoader* frameLoader();
    bool shouldLoadNewResource() const;

    CachedResourceHandle<CachedResource> requestResource(CachedResource::Type, FetchRequest&);
    CachedResourceHandle<CachedResource> revalidateResource(const FetchRequest&, CachedResource*);
    CachedResourceHandle<CachedResource> loadResource(CachedResource::Type, FetchRequest&, const String& charset);
    void preCacheDataURIImage(const FetchRequest&);
    void storeResourceTimingInitiatorInformation(const CachedResourceHandle<CachedResource>&, const FetchRequest&);
    void requestPreload(CachedResource::Type, FetchRequest&, const String& charset);

    enum RevalidationPolicy { Use, Revalidate, Reload, Load };
    RevalidationPolicy determineRevalidationPolicy(CachedResource::Type, ResourceRequest&, bool forPreload, CachedResource* existingResource, FetchRequest::DeferOption) const;

    void determineTargetType(ResourceRequest&, CachedResource::Type);
    ResourceRequestCachePolicy resourceRequestCachePolicy(const ResourceRequest&, CachedResource::Type);
    void addAdditionalRequestHeaders(ResourceRequest&, CachedResource::Type);

    void notifyLoadedFromMemoryCache(CachedResource*);
    bool checkInsecureContent(CachedResource::Type, const KURL&) const;

    void garbageCollectDocumentResourcesTimerFired(Timer<ResourceFetcher>*);
    void performPostLoadActions();

    bool clientDefersImage(const KURL&) const;
    void reloadImagesIfNotDeferred();

    HashSet<String> m_validatedURLs;
    mutable DocumentResourceMap m_documentResources;
    Document* m_document;
    DocumentLoader* m_documentLoader;

    int m_requestCount;

    OwnPtr<ListHashSet<CachedResource*> > m_preloads;
    struct PendingPreload {
        CachedResource::Type m_type;
        FetchRequest m_request;
        String m_charset;
    };
    Deque<PendingPreload> m_pendingPreloads;

    Timer<ResourceFetcher> m_garbageCollectDocumentResourcesTimer;

    typedef HashMap<CachedResource*, RefPtr<ResourceTimingInfo> > ResourceTimingInfoMap;
    ResourceTimingInfoMap m_resourceTimingInfoMap;

    // 29 bits left
    bool m_autoLoadImages : 1;
    bool m_imagesEnabled : 1;
    bool m_allowStaleResources : 1;
};

class ResourceCacheValidationSuppressor {
    WTF_MAKE_NONCOPYABLE(ResourceCacheValidationSuppressor);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ResourceCacheValidationSuppressor(ResourceFetcher* loader)
        : m_loader(loader)
        , m_previousState(false)
    {
        if (m_loader) {
            m_previousState = m_loader->m_allowStaleResources;
            m_loader->m_allowStaleResources = true;
        }
    }
    ~ResourceCacheValidationSuppressor()
    {
        if (m_loader)
            m_loader->m_allowStaleResources = m_previousState;
    }
private:
    ResourceFetcher* m_loader;
    bool m_previousState;
};

} // namespace WebCore

#endif
