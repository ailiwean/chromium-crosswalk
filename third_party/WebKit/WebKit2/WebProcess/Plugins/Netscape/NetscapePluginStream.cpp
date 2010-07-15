/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "NetscapePluginStream.h"

#include "NetscapePlugin.h"
#include <utility>

using namespace WebCore;
using namespace std;

namespace WebKit {

NetscapePluginStream::NetscapePluginStream(PassRefPtr<NetscapePlugin> plugin, uint64_t streamID, bool sendNotification, void* notificationData)
    : m_plugin(plugin)
    , m_streamID(streamID)
    , m_sendNotification(sendNotification)
    , m_notificationData(notificationData)
    , m_npStream()
    , m_transferMode(NP_NORMAL)
    , m_offset(0)
    , m_isStarted(false)
#if !ASSERT_DISABLED
    , m_urlNotifyHasBeenCalled(false)
#endif    
    , m_deliveryDataTimer(RunLoop::main(), this, &NetscapePluginStream::deliverDataToPlugin)
    , m_stopStreamWhenDoneDelivering(false)
{
}

NetscapePluginStream::~NetscapePluginStream()
{
    ASSERT(!m_isStarted);
    ASSERT(!m_sendNotification || m_urlNotifyHasBeenCalled);
}

void NetscapePluginStream::didReceiveResponse(const KURL& responseURL, uint32_t streamLength, uint32_t lastModifiedTime, const String& mimeType, const String& headers)
{
    start(responseURL, streamLength, lastModifiedTime, mimeType, headers);
}

void NetscapePluginStream::sendJavaScriptStream(const String& requestURLString, const String& result)
{
    // starting the stream or delivering the data to it might cause the plug-in stream to go away, so we keep
    // a reference to it here.
    RefPtr<NetscapePluginStream> protect(this);

    CString resultCString = requestURLString.utf8();
    if (resultCString.isNull()) {
        // There was an error evaluating the JavaScript, call NPP_URLNotify if needed and then destroy the stream.
        notifyAndDestroyStream(NPRES_NETWORK_ERR);
        return;
    }

    if (!start(requestURLString, resultCString.length(), 0, "text/plain", ""))
        return;

    deliverData(resultCString.data(), resultCString.length());
    stop(NPRES_DONE);
}

NPError NetscapePluginStream::destroy(NPReason reason)
{
    // It doesn't make sense to call NPN_DestroyStream on a stream that hasn't been started yet.
    if (!m_isStarted)
        return NPERR_GENERIC_ERROR;

    // It isn't really valid for a plug-in to call NPN_DestroyStream with NPRES_DONE.
    // (At least not for browser initiated streams, and we don't support plug-in initiated streams).
    if (reason == NPRES_DONE)
        return NPERR_INVALID_PARAM;

    stop(reason);
    return NPERR_NO_ERROR;
}

static bool isSupportedTransferMode(uint16_t transferMode)
{
    switch (transferMode) {
    case NP_NORMAL:
        return true;
    // FIXME: We don't support streaming to files.
    case NP_ASFILEONLY:
    case NP_ASFILE:
        return false;
    // FIXME: We don't support seekable streams.
    case NP_SEEK:
        return false;
    }

    ASSERT_NOT_REACHED();
    return false;
}
    
bool NetscapePluginStream::start(const WebCore::String& responseURLString, uint32_t streamLength, uint32_t lastModifiedTime, const String& mimeType, const String& headers)
{
    m_responseURL = responseURLString.utf8();
    m_mimeType = mimeType.utf8();
    m_headers = headers.utf8();

    m_npStream.ndata = this;
    m_npStream.url = m_responseURL.data();
    m_npStream.end = streamLength;
    m_npStream.lastmodified = lastModifiedTime;
    m_npStream.notifyData = 0;
    m_npStream.headers = m_headers.length() == 0 ? 0 : m_headers.data();

    NPError error = m_plugin->NPP_NewStream(const_cast<char*>(m_mimeType.data()), &m_npStream, false, &m_transferMode);
    if (error != NPERR_NO_ERROR) {
        // We failed to start the stream, destroy it.
        notifyAndDestroyStream(NPRES_NETWORK_ERR);
        return false;
    }

    // We successfully started the stream.
    m_isStarted = true;

    if (!isSupportedTransferMode(m_transferMode)) {
        // Stop the stream.
        stop(NPRES_NETWORK_ERR);
        return false;
    }

    return true;
}

void NetscapePluginStream::deliverData(const char* bytes, int length)
{
    ASSERT(m_isStarted);

    if (m_transferMode != NP_ASFILEONLY) {
        if (!m_deliveryData)
            m_deliveryData.set(new Vector<char>);

        m_deliveryData->reserveCapacity(m_deliveryData->size() + length);
        m_deliveryData->append(bytes, length);
        
        deliverDataToPlugin();
    }

    // FIXME: Deliver the data to a file as well if needed.
}

void NetscapePluginStream::deliverDataToPlugin()
{
    ASSERT(m_isStarted);

    int32_t numBytesToDeliver = m_deliveryData->size();
    int32_t numBytesDelivered = 0;

    while (numBytesDelivered < numBytesToDeliver) {
        int32_t numBytesPluginCanHandle = m_plugin->NPP_WriteReady(&m_npStream);
        
        // NPP_WriteReady could call NPN_DestroyStream and destroy the stream.
        if (!m_isStarted)
            return;

        if (numBytesPluginCanHandle <= 0) {
            // The plug-in can't handle more data, we'll send the rest later
            m_deliveryDataTimer.startOneShot(0);
            break;
        }

        // Figure out how much data to send to the plug-in.
        int32_t dataLength = min(numBytesPluginCanHandle, numBytesToDeliver - numBytesDelivered);
        char* data = m_deliveryData->data() + numBytesDelivered;

        int32_t numBytesWritten = m_plugin->NPP_Write(&m_npStream, m_offset, dataLength, data);
        if (numBytesWritten < 0) {
            // FIXME: Destroy the stream!
            ASSERT_NOT_REACHED();
        }

        // NPP_Write could call NPN_DestroyStream and destroy the stream.
        if (!m_isStarted)
            return;

        numBytesWritten = min(numBytesWritten, dataLength);
        m_offset += numBytesWritten;
        numBytesDelivered += numBytesWritten;
    }

    // We didn't write anything.
    if (!numBytesDelivered)
        return;

    if (numBytesDelivered < numBytesToDeliver) {
        // Remove the bytes that we actually delivered.
        m_deliveryData->remove(0, numBytesDelivered);
    } else {
        m_deliveryData->clear();

        if (m_stopStreamWhenDoneDelivering)
            stop(NPRES_DONE);
    }
}

void NetscapePluginStream::stop(NPReason reason)
{
    ASSERT(m_isStarted);

    if (reason == NPRES_DONE && m_deliveryData && !m_deliveryData->isEmpty()) {
        // There is still data left that the plug-in hasn't been able to consume yet.
        ASSERT(m_deliveryDataTimer.isActive());
        
        // Set m_stopStreamWhenDoneDelivering to true so that the next time the delivery timer fires
        // and calls deliverDataToPlugin the stream will be closed if all the remaining data was
        // successfully delivered.
        m_stopStreamWhenDoneDelivering = true;
        return;
    }

    m_deliveryData = 0;
    m_deliveryDataTimer.stop();

    // Set m_isStarted to false before calling NPP_DestroyStream in case NPP_DestroyStream calls NPN_DestroyStream.
    m_isStarted = false;

    m_plugin->NPP_DestroyStream(&m_npStream, reason);

    notifyAndDestroyStream(reason);
}

void NetscapePluginStream::notifyAndDestroyStream(NPReason reason)
{
    ASSERT(!m_isStarted);
    ASSERT(!m_deliveryDataTimer.isActive());
    ASSERT(!m_urlNotifyHasBeenCalled);
    
    if (m_sendNotification) {
        m_plugin->NPP_URLNotify(m_responseURL.data(), reason, m_notificationData);
    
#if !ASSERT_DISABLED
        m_urlNotifyHasBeenCalled = true;
#endif    
    }

    if (reason != NPRES_DONE)
        m_plugin->cancelStreamLoad(this);

    m_plugin->removePluginStream(this);
}

} // namespace WebKit
