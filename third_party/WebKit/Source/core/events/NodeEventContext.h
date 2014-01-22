/*
 * Copyright (C) 2014 Google Inc. All Rights Reserved.
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
 *
 */

#ifndef NodeEventContext_h
#define NodeEventContext_h

#include "core/events/TreeScopeEventContext.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

class EventTarget;
class Node;
class NodeList;
class TouchEventContext;

class NodeEventContext {
public:
    // FIXME: Use ContainerNode instead of Node.
    NodeEventContext(PassRefPtr<Node>, PassRefPtr<EventTarget> currentTarget);
    ~NodeEventContext();

    Node* node() const { return m_node.get(); }
    EventTarget* currentTarget() const { return m_currentTarget.get(); }

    TreeScopeEventContext* treeScopeEventContext() const { return m_treeScopeEventContext.get(); }
    void setTreeScopeEventContext(PassRefPtr<TreeScopeEventContext> prpTreeScopeEventContext) { m_treeScopeEventContext = prpTreeScopeEventContext; }

    EventTarget* target() const { return m_treeScopeEventContext->target(); }
    EventTarget* relatedTarget() const { return m_treeScopeEventContext->relatedTarget(); }
    TouchEventContext* touchEventContext() const { return m_treeScopeEventContext->touchEventContext(); }
    PassRefPtr<NodeList> eventPath() const { return m_treeScopeEventContext->eventPath(); }

    bool currentTargetSameAsTarget() const { return m_currentTarget.get() == target(); }
    void handleLocalEvents(Event*) const;

private:
    RefPtr<Node> m_node;
    RefPtr<EventTarget> m_currentTarget;
    RefPtr<TreeScopeEventContext> m_treeScopeEventContext;
};

}

#endif // NodeEventContext_h
