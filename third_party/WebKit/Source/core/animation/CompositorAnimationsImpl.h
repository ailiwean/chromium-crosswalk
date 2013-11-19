/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
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

#include "core/animation/AnimationEffect.h"
#include "core/animation/KeyframeAnimationEffect.h"
#include "core/animation/Timing.h"
#include "core/platform/animation/TimingFunction.h"
#include "public/platform/WebAnimation.h"

namespace WebCore {

class CompositorAnimationsKeyframeEffectHelper {
private:
    typedef Vector<std::pair<double, const AnimationEffect::CompositableValue*> > KeyframeValues;

    static PassOwnPtr<Vector<CSSPropertyID> > getProperties(const KeyframeAnimationEffect*);
    static PassOwnPtr<KeyframeValues> getKeyframeValuesForProperty(const KeyframeAnimationEffect*, CSSPropertyID, double scale, bool reverse = false);
    static PassOwnPtr<KeyframeValues> getKeyframeValuesForProperty(const KeyframeAnimationEffect::PropertySpecificKeyframeGroup*, double scale, bool reverse);

    friend class CompositorAnimationsImpl;
};

class CompositorAnimationsImpl {
private:
    struct CompositorTiming {
        bool reverse;
        bool alternate;
        double scaledDuration;
        double scaledTimeOffset;
        int adjustedIterationCount;
    };

    static bool convertTimingForCompositor(const Timing&, CompositorTiming& out);

    static bool isCandidateForCompositor(const Keyframe&);
    static bool isCandidateForCompositor(const KeyframeAnimationEffect&);
    static bool isCandidateForCompositor(const Timing&, const KeyframeAnimationEffect::KeyframeVector&);
    static bool isCandidateForCompositor(const TimingFunction&, const KeyframeAnimationEffect::KeyframeVector*, bool isNestedCall = false);
    static void getAnimationOnCompositor(const Timing&, const KeyframeAnimationEffect&, Vector<OwnPtr<blink::WebAnimation> >& animations, const IntSize& elementSize);

    template<typename PlatformAnimationCurveType, typename PlatformAnimationKeyframeType>
    static void addKeyframesToCurve(PlatformAnimationCurveType&, const CompositorAnimationsKeyframeEffectHelper::KeyframeValues&, const TimingFunction&, const IntSize& elementSize);

    friend class CompositorAnimations;
    friend class AnimationCompositorAnimationsTest;
};

} // WebCore
