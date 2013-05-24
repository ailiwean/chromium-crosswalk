/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#ifndef InjectedScriptHost_h
#define InjectedScriptHost_h

#include "bindings/v8/ScriptState.h"
#include "bindings/v8/ScriptWrappable.h"
#include "core/inspector/InspectorAgent.h"
#include "core/page/ConsoleTypes.h"
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

namespace WebCore {

class Database;
class InjectedScript;
class InspectorAgent;
class InspectorConsoleAgent;
class InspectorDOMAgent;
class InspectorDOMStorageAgent;
class InspectorDatabaseAgent;
class InspectorDebuggerAgent;
class InspectorFrontend;
class InspectorObject;
class InspectorValue;
class Node;
class ScriptDebugServer;
class ScriptObject;
class ScriptValue;
class Storage;

struct EventListenerInfo;

// SECURITY NOTE: Although the InjectedScriptHost is intended for use solely by the inspector,
// a reference to the InjectedScriptHost may be leaked to the page being inspected. Thus, the
// InjectedScriptHost must never implemment methods that have more power over the page than the
// page already has itself (e.g. origin restriction bypasses).

class InjectedScriptHost : public RefCounted<InjectedScriptHost>, public ScriptWrappable {
public:
    static PassRefPtr<InjectedScriptHost> create();
    ~InjectedScriptHost();

    void init(InspectorAgent* inspectorAgent
            , InspectorConsoleAgent* consoleAgent
            , InspectorDatabaseAgent* databaseAgent
            , InspectorDOMStorageAgent* domStorageAgent
            , InspectorDOMAgent* domAgent
            , InspectorDebuggerAgent* debuggerAgent
        )
    {
        m_inspectorAgent = inspectorAgent;
        m_consoleAgent = consoleAgent;
        m_databaseAgent = databaseAgent;
        m_domStorageAgent = domStorageAgent;
        m_domAgent = domAgent;
        m_debuggerAgent = debuggerAgent;
    }

    static Node* scriptValueAsNode(ScriptValue);
    static ScriptValue nodeAsScriptValue(ScriptState*, Node*);

    void disconnect();

    class InspectableObject {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        virtual ScriptValue get(ScriptState*);
        virtual ~InspectableObject() { }
    };
    void addInspectedObject(PassOwnPtr<InspectableObject>);
    void clearInspectedObjects();
    InspectableObject* inspectedObject(unsigned int num);

    void inspectImpl(PassRefPtr<InspectorValue> objectToInspect, PassRefPtr<InspectorValue> hints);
    void getEventListenersImpl(Node*, Vector<EventListenerInfo>& listenersArray);

    void clearConsoleMessages();
    void copyText(const String& text);
    String databaseIdImpl(Database*);
    String storageIdImpl(Storage*);

    ScriptDebugServer& scriptDebugServer();

private:
    InjectedScriptHost();

    InspectorAgent* m_inspectorAgent;
    InspectorConsoleAgent* m_consoleAgent;
    InspectorDatabaseAgent* m_databaseAgent;
    InspectorDOMStorageAgent* m_domStorageAgent;
    InspectorDOMAgent* m_domAgent;
    InspectorDebuggerAgent* m_debuggerAgent;
    Vector<OwnPtr<InspectableObject> > m_inspectedObjects;
    OwnPtr<InspectableObject> m_defaultInspectableObject;
};

} // namespace WebCore

#endif // !defined(InjectedScriptHost_h)
