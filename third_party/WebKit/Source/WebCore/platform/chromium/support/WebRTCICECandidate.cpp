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

#include <public/WebRTCICECandidate.h>

#include "RTCIceCandidateDescriptor.h"
#include <public/WebString.h>

using namespace WebCore;

namespace WebKit {

WebRTCICECandidate::WebRTCICECandidate(RTCIceCandidateDescriptor* iceCandidate)
    : m_private(iceCandidate)
{
}

WebRTCICECandidate::WebRTCICECandidate(PassRefPtr<RTCIceCandidateDescriptor> iceCandidate)
    : m_private(iceCandidate)
{
}

void WebRTCICECandidate::assign(const WebRTCICECandidate& other)
{
    m_private = other.m_private;
}

void WebRTCICECandidate::reset()
{
    m_private.reset();
}

void WebRTCICECandidate::initialize(const WebString& candidate, const WebString& sdpMid, unsigned short sdpMLineIndex)
{
    m_private = RTCIceCandidateDescriptor::create(candidate, sdpMid, sdpMLineIndex);
}

WebRTCICECandidate::operator PassRefPtr<WebCore::RTCIceCandidateDescriptor>() const
{
    return m_private.get();
}

WebString WebRTCICECandidate::candidate() const
{
    ASSERT(!m_private.isNull());
    return m_private->candidate();
}

WebString WebRTCICECandidate::sdpMid() const
{
    ASSERT(!m_private.isNull());
    return m_private->sdpMid();
}

unsigned short WebRTCICECandidate::sdpMLineIndex() const
{
    ASSERT(!m_private.isNull());
    return m_private->sdpMLineIndex();
}

} // namespace WebKit
