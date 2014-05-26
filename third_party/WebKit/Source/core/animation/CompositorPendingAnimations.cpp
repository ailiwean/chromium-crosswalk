/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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
#include "core/animation/CompositorPendingAnimations.h"

#include "core/animation/Animation.h"
#include "core/animation/AnimationTimeline.h"
#include "core/animation/CompositorAnimations.h"
#include "core/frame/FrameView.h"
#include "core/page/Page.h"
#include "core/rendering/RenderLayer.h"

namespace WebCore {

void CompositorPendingAnimations::add(AnimationPlayer* player)
{
    Page* page = player->timeline()->document()->page();
    bool visible = page && page->visibilityState() == PageVisibilityStateVisible;
    if (!player->hasStartTime() && !visible)
        player->setStartTimeInternal(player->timeline()->currentTimeInternal(), true);

    m_pendingStart.append(player);
}

void CompositorPendingAnimations::cancel(Element& element, int animationId)
{
    m_pendingCancellation.append(std::make_pair(&element, animationId));
}

bool CompositorPendingAnimations::startAndCancelPendingAnimations()
{
    WillBeHeapVector<std::pair<RefPtrWillBeMember<Element>, int> > pendingCancellation;
    pendingCancellation.swap(m_pendingCancellation);
    for (size_t i = 0; i < pendingCancellation.size(); ++i)
        CompositorAnimations::instance()->cancelAnimationOnCompositor(*pendingCancellation[i].first, pendingCancellation[i].second);

    WillBeHeapVector<RefPtrWillBeMember<AnimationPlayer> > pendingStart;
    pendingStart.swap(m_pendingStart);
    bool startedSynchronizedOnCompositor = false;
    for (size_t i = 0; i < pendingStart.size(); ++i) {
        if (!pendingStart[i]->hasActiveAnimationsOnCompositor() && pendingStart[i]->maybeStartAnimationOnCompositor() && !pendingStart[i]->hasStartTime())
            startedSynchronizedOnCompositor = true;
    }

    // If any synchronized animations were started on the compositor, all
    // remaning synchronized animations need to wait for the synchronized
    // start time. Otherwise they may start immediately.
    if (startedSynchronizedOnCompositor) {
        for (size_t i = 0; i < pendingStart.size(); ++i) {
            if (!pendingStart[i]->hasStartTime()) {
                m_waitingForCompositorAnimationStart.append(pendingStart[i]);
            }
        }
    } else {
        for (size_t i = 0; i < pendingStart.size(); ++i) {
            if (!pendingStart[i]->hasStartTime()) {
                pendingStart[i]->setStartTimeInternal(pendingStart[i]->timeline()->currentTimeInternal(), true);
            }
        }
    }
    pendingStart.clear();

    if (startedSynchronizedOnCompositor || m_waitingForCompositorAnimationStart.isEmpty())
        return !m_waitingForCompositorAnimationStart.isEmpty();

    // Check if we're still waiting for any compositor animations to start.
    for (size_t i = 0; i < m_waitingForCompositorAnimationStart.size(); ++i) {
        if (m_waitingForCompositorAnimationStart[i].get()->hasActiveAnimationsOnCompositor())
            return true;
    }

    // If not, go ahead and start any animations that were waiting.
    notifyCompositorAnimationStarted(monotonicallyIncreasingTime());
    return false;
}

void CompositorPendingAnimations::notifyCompositorAnimationStarted(double monotonicAnimationStartTime)
{
    for (size_t i = 0; i < m_waitingForCompositorAnimationStart.size(); ++i) {
        AnimationPlayer* player = m_waitingForCompositorAnimationStart[i].get();
        player->setStartTimeInternal(monotonicAnimationStartTime - player->timeline()->zeroTime(), true);
    }

    m_waitingForCompositorAnimationStart.clear();
}

void CompositorPendingAnimations::trace(Visitor* visitor)
{
    visitor->trace(m_pendingStart);
    visitor->trace(m_waitingForCompositorAnimationStart);
    visitor->trace(m_pendingCancellation);
}

} // namespace
