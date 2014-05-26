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

#ifndef CompositorPendingAnimations_h
#define CompositorPendingAnimations_h

#include "platform/heap/Handle.h"
#include "wtf/Vector.h"

namespace WebCore {

class AnimationPlayer;
class Element;

// Manages the start and cancellation of animations on the compositor
// deferred until a compositing update.
// For CSS Animations, used to synchronize the start of main-thread animations
// with compositor animations when both classes of CSS Animations are triggered
// by the same recalc
class CompositorPendingAnimations FINAL {
    DISALLOW_ALLOCATION();
public:
    void add(AnimationPlayer*);
    void cancel(Element&, int animationId);
    // Returns whether we are waiting for an animation to start and should
    // service again on the next frame.
    bool startAndCancelPendingAnimations();
    void notifyCompositorAnimationStarted(double monotonicAnimationStartTime);

    void trace(Visitor*);

private:
    WillBeHeapVector<RefPtrWillBeMember<AnimationPlayer> > m_pendingStart;
    WillBeHeapVector<RefPtrWillBeMember<AnimationPlayer> > m_waitingForCompositorAnimationStart;
    WillBeHeapVector<std::pair<RefPtrWillBeMember<Element>, int> > m_pendingCancellation;
};

} // namespace WebCore

#endif
