/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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
#include "modules/mediasource/WebKitSourceBufferList.h"

#include "core/events/Event.h"
#include "core/events/GenericEventQueue.h"
#include "modules/mediasource/WebKitSourceBuffer.h"

namespace WebCore {

WebKitSourceBufferList::WebKitSourceBufferList(ScriptExecutionContext* context, GenericEventQueue* asyncEventQueue)
    : m_scriptExecutionContext(context)
    , m_asyncEventQueue(asyncEventQueue)
{
    ScriptWrappable::init(this);
}

unsigned long WebKitSourceBufferList::length() const
{
    return m_list.size();
}

WebKitSourceBuffer* WebKitSourceBufferList::item(unsigned index) const
{
    if (index >= m_list.size())
        return 0;
    return m_list[index].get();
}

void WebKitSourceBufferList::add(PassRefPtr<WebKitSourceBuffer> buffer)
{
    m_list.append(buffer);
    createAndFireEvent(EventNames::webkitaddsourcebuffer);
}

bool WebKitSourceBufferList::remove(WebKitSourceBuffer* buffer)
{
    size_t index = m_list.find(buffer);
    if (index == kNotFound)
        return false;

    buffer->removedFromMediaSource();
    m_list.remove(index);
    createAndFireEvent(EventNames::webkitremovesourcebuffer);
    return true;
}

void WebKitSourceBufferList::clear()
{
    for (size_t i = 0; i < m_list.size(); ++i)
        m_list[i]->removedFromMediaSource();
    m_list.clear();
    createAndFireEvent(EventNames::webkitremovesourcebuffer);
}

void WebKitSourceBufferList::createAndFireEvent(const AtomicString& eventName)
{
    ASSERT(m_asyncEventQueue);

    RefPtr<Event> event = Event::create(eventName);
    event->setTarget(this);

    m_asyncEventQueue->enqueueEvent(event.release());
}

const AtomicString& WebKitSourceBufferList::interfaceName() const
{
    return eventNames().interfaceForWebKitSourceBufferList;
}

ScriptExecutionContext* WebKitSourceBufferList::scriptExecutionContext() const
{
    return m_scriptExecutionContext;
}

EventTargetData* WebKitSourceBufferList::eventTargetData()
{
    return &m_eventTargetData;
}

EventTargetData* WebKitSourceBufferList::ensureEventTargetData()
{
    return &m_eventTargetData;
}

} // namespace WebCore
