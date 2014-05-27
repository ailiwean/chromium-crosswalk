/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL GOOGLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MediaStreamTrackSourcesRequestImpl_h
#define MediaStreamTrackSourcesRequestImpl_h

#include "modules/mediastream/SourceInfo.h"
#include "platform/Timer.h"
#include "platform/mediastream/MediaStreamTrackSourcesRequest.h"
#include "public/platform/WebVector.h"
#include "wtf/PassRefPtr.h"
#include "wtf/text/WTFString.h"

namespace blink {
class WebSourceInfo;
}

namespace WebCore {

class MediaStreamTrackSourcesCallback;

class MediaStreamTrackSourcesRequestImpl FINAL : public MediaStreamTrackSourcesRequest {
public:
    static PassRefPtrWillBeRawPtr<MediaStreamTrackSourcesRequestImpl> create(const String&, PassOwnPtr<MediaStreamTrackSourcesCallback>);
    ~MediaStreamTrackSourcesRequestImpl();

    virtual String origin() { return m_origin; }
    virtual void requestSucceeded(const blink::WebVector<blink::WebSourceInfo>&);

    virtual void trace(Visitor*) OVERRIDE;

private:
    MediaStreamTrackSourcesRequestImpl(const String&, PassOwnPtr<MediaStreamTrackSourcesCallback>);

    void scheduledEventTimerFired(Timer<MediaStreamTrackSourcesRequestImpl>*);

    OwnPtr<MediaStreamTrackSourcesCallback> m_callback;
    String m_origin;
    Timer<MediaStreamTrackSourcesRequestImpl> m_scheduledEventTimer;
    SourceInfoVector m_sourceInfos;
    RefPtrWillBeMember<MediaStreamTrackSourcesRequest> m_protect;
};

} // namespace WebCore

#endif // MediaStreamTrackSourcesRequestImpl_h
