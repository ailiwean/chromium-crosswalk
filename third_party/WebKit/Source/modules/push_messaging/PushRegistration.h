// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PushRegistration_h
#define PushRegistration_h

#include "bindings/v8/ScriptWrappable.h"
#include "heap/Handle.h"
#include "public/platform/WebPushRegistration.h"
#include "wtf/OwnPtr.h"
#include "wtf/text/WTFString.h"

namespace WebCore {

class NewScriptState;

class PushRegistration FINAL : public RefCountedWillBeGarbageCollectedFinalized<PushRegistration>, public ScriptWrappable {
public:
    // For CallbackPromiseAdapter.
    typedef blink::WebPushRegistration WebType;
    static PassRefPtrWillBeRawPtr<PushRegistration> from(NewScriptState*, WebType* registrationRaw)
    {
        OwnPtr<WebType> registration = adoptPtr(registrationRaw);
        return adoptRefWillBeNoop(new PushRegistration(registration->endpoint, registration->registrationId));
    }

    virtual ~PushRegistration();

    const String& pushEndpoint() const { return m_pushEndpoint; }
    const String& pushRegistrationId() const { return m_pushRegistrationId; }

    void trace(Visitor*) { }

private:
    PushRegistration(const String& pushEndpoint, const String& pushRegistrationId);

    String m_pushEndpoint;
    String m_pushRegistrationId;
};

} // namespace WebCore

#endif // PushRegistration_h
