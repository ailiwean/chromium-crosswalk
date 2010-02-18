/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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
#include "JSCustomSQLStatementErrorCallback.h"

#if ENABLE(DATABASE)

#include "Frame.h"
#include "JSCallbackData.h"
#include "JSSQLError.h"
#include "JSSQLTransaction.h"
#include "ScriptController.h"
#include <runtime/JSLock.h>
#include <wtf/MainThread.h>

namespace WebCore {
    
using namespace JSC;
    
JSCustomSQLStatementErrorCallback::JSCustomSQLStatementErrorCallback(JSObject* callback, JSDOMGlobalObject* globalObject)
    : m_data(new JSCallbackData(callback, globalObject))
{
}

JSCustomSQLStatementErrorCallback::~JSCustomSQLStatementErrorCallback()
{
    callOnMainThread(JSCallbackData::deleteData, m_data);
#ifndef NDEBUG
    m_data = 0;
#endif
}

bool JSCustomSQLStatementErrorCallback::handleEvent(SQLTransaction* transaction, SQLError* error)
{
    ASSERT(m_data);
        
    RefPtr<JSCustomSQLStatementErrorCallback> protect(this);
        
    JSC::JSLock lock(SilenceAssertionsOnly);
    ExecState* exec = m_data->globalObject()->globalExec();
    MarkedArgumentBuffer args;
    args.append(toJS(exec, deprecatedGlobalObjectForPrototype(exec), transaction));
    args.append(toJS(exec, deprecatedGlobalObjectForPrototype(exec), error));
    
    bool raisedException = false;
    JSValue result = m_data->invokeCallback(args, &raisedException);
    if (raisedException) {
        // The spec says:
        // "If the error callback returns false, then move on to the next statement..."
        // "Otherwise, the error callback did not return false, or there was no error callback"
        // Therefore an exception and returning true are the same thing - so, return true on an exception
        return true;
    }
    return !result.isFalse();
}

}

#endif // ENABLE(DATABASE)
