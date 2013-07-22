/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#include "config.h"
#include "core/workers/DedicatedWorkerGlobalScope.h"

#include "bindings/v8/ExceptionState.h"
#include "core/page/DOMWindow.h"
#include "core/workers/DedicatedWorkerThread.h"
#include "core/workers/WorkerClients.h"
#include "core/workers/WorkerObjectProxy.h"
#include "core/workers/WorkerThreadStartupData.h"

namespace WebCore {

PassRefPtr<DedicatedWorkerGlobalScope> DedicatedWorkerGlobalScope::create(DedicatedWorkerThread* thread, PassOwnPtr<WorkerThreadStartupData> startupData, double timeOrigin)
{
    RefPtr<DedicatedWorkerGlobalScope> context = adoptRef(new DedicatedWorkerGlobalScope(startupData->m_scriptURL, startupData->m_userAgent, thread, startupData->m_topOrigin, timeOrigin, startupData->m_workerClients.release()));
    context->applyContentSecurityPolicyFromString(startupData->m_contentSecurityPolicy, startupData->m_contentSecurityPolicyType);
    return context.release();
}

DedicatedWorkerGlobalScope::DedicatedWorkerGlobalScope(const KURL& url, const String& userAgent, DedicatedWorkerThread* thread, PassRefPtr<SecurityOrigin> topOrigin, double timeOrigin, PassOwnPtr<WorkerClients> workerClients)
    : WorkerGlobalScope(url, userAgent, thread, topOrigin, timeOrigin, workerClients)
{
    ScriptWrappable::init(this);
}

DedicatedWorkerGlobalScope::~DedicatedWorkerGlobalScope()
{
}

const AtomicString& DedicatedWorkerGlobalScope::interfaceName() const
{
    return eventNames().interfaceForDedicatedWorkerGlobalScope;
}

void DedicatedWorkerGlobalScope::postMessage(PassRefPtr<SerializedScriptValue> message, const MessagePortArray* ports, ExceptionState& es)
{
    // Disentangle the port in preparation for sending it to the remote context.
    OwnPtr<MessagePortChannelArray> channels = MessagePort::disentanglePorts(ports, es);
    if (es.hadException())
        return;
    thread()->workerObjectProxy().postMessageToWorkerObject(message, channels.release());
}

void DedicatedWorkerGlobalScope::importScripts(const Vector<String>& urls, ExceptionState& es)
{
    Base::importScripts(urls, es);
    thread()->workerObjectProxy().reportPendingActivity(hasPendingActivity());
}

DedicatedWorkerThread* DedicatedWorkerGlobalScope::thread()
{
    return static_cast<DedicatedWorkerThread*>(Base::thread());
}

} // namespace WebCore
