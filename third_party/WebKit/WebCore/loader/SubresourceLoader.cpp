/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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
#include "SubresourceLoader.h"

#include "Document.h"
#include "Frame.h"
#include "ResourceHandle.h"
#include "ResourceRequest.h"
#include "SubresourceLoaderClient.h"

namespace WebCore {

#if !PLATFORM(MAC)

SubresourceLoader::SubresourceLoader(Frame* frame, SubresourceLoaderClient* client, ResourceHandle* handle)
    : ResourceLoader(frame)
    , m_client(client)
    , m_handle(handle)
    , m_loadingMultipartContent(false)
{
}

SubresourceLoader::~SubresourceLoader()
{
}

PassRefPtr<SubresourceLoader> SubresourceLoader::create(Frame* frame, SubresourceLoaderClient* client, const ResourceRequest& request)
{
    // FIXME: This is only temporary until we get the Mac version of SubresourceLoader::create cross-platform.
    RefPtr<SubresourceLoader> subloader(new SubresourceLoader(frame, client, 0));

    subloader->m_handle = ResourceHandle::create(request, subloader.get(), frame->document()->docLoader());

    return subloader.release();
}

#endif

void SubresourceLoader::willSendRequest(ResourceHandle*, ResourceRequest& request, const ResourceResponse& redirectResponse)
{
    if (m_client)
        m_client->willSendRequest(this, request, redirectResponse);
}

void SubresourceLoader::didReceiveResponse(ResourceHandle*, const ResourceResponse& response)
{
    if (m_client)
        m_client->didReceiveResponse(this, response);
}

void SubresourceLoader::didReceiveData(ResourceHandle*, const char* data, int length)
{
    if (m_client)
        m_client->didReceiveData(this, data, length);
}

void SubresourceLoader::didFinishLoading(ResourceHandle*)
{
    if (m_client) {
        // We must protect the resource loader in case the call to receivedAllData causes a deref.
        RefPtr<SubresourceLoader> protect(this);
        
#if PLATFORM(MAC)
        m_client->receivedAllData(this, resourceData());
#else
        m_client->receivedAllData(this, 0);
#endif
        m_client->didFinishLoading(this);
    }
}

void SubresourceLoader::didFailWithError(ResourceHandle*, const ResourceError& error)
{
    if (m_client)
        m_client->didFailWithError(this, error);
}

void SubresourceLoader::receivedAllData(ResourceHandle*, PlatformData data)
{
    if (m_client)
        m_client->receivedAllData(this, data);
}

}
