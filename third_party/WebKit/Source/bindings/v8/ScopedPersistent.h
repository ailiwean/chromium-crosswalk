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

#ifndef ScopedPersistent_h
#define ScopedPersistent_h

#include <v8.h>
#include "wtf/Noncopyable.h"

namespace WebCore {

template<typename T>
class ScopedPersistent {
    WTF_MAKE_NONCOPYABLE(ScopedPersistent);
public:
    ScopedPersistent() { }

    explicit ScopedPersistent(v8::Handle<T> handle)
        : m_handle(v8::Isolate::GetCurrent(), handle)
    {
    }

    ~ScopedPersistent()
    {
        clear();
    }

    ALWAYS_INLINE v8::Local<T> newLocal(v8::Isolate* isolate) const
    {
        return v8::Local<T>::New(isolate, m_handle);
    }

    template<typename P>
    void makeWeak(P* parameters, void (*callback)(v8::Isolate*, v8::Persistent<T>*, P*))
    {
        m_handle.MakeWeak(parameters, callback);
    }

    bool isEmpty() const { return m_handle.IsEmpty(); }

    void set(v8::Isolate* isolate, v8::Handle<T> handle)
    {
        m_handle.Reset(isolate, handle);
    }

    // Note: This is clear in the OwnPtr sense, not the v8::Handle sense.
    void clear()
    {
        if (m_handle.IsEmpty())
            return;
        m_handle.Dispose();
        m_handle.Clear();
    }

    bool operator==(const ScopedPersistent<T>& other)
    {
        return m_handle == other.m_handle;
    }

private:
    // FIXME: This function does an unsafe handle access. Remove it.
    friend class V8AbstractEventListener;
    friend class V8PerIsolateData;
    ALWAYS_INLINE v8::Persistent<T>& getUnsafe()
    {
        return m_handle;
    }

    v8::Persistent<T> m_handle;
};

} // namespace WebCore

#endif // ScopedPersistent_h
