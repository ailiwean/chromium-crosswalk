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

#ifndef Animation_h
#define Animation_h

#include "core/animation/AnimationEffect.h"
#include "core/animation/TimedItem.h"
#include "wtf/RefPtr.h"

namespace WebCore {

class Element;

class Animation FINAL : public TimedItem {

public:
    static PassRefPtr<Animation> create(PassRefPtr<Element>, PassRefPtr<AnimationEffect>, const Timing&);
    virtual ~Animation();

    const AnimationEffect::CompositableValueMap* compositableValues() const
    {
        ASSERT(m_compositableValues);
        return m_compositableValues.get();
    }

protected:
    virtual void applyEffects(bool previouslyActiveOrInEffect);
    virtual void clearEffects();
    virtual void updateChildrenAndEffects(bool) const OVERRIDE FINAL;

private:
    Animation(PassRefPtr<Element>, PassRefPtr<AnimationEffect>, const Timing&);

    RefPtr<Element> m_target;
    RefPtr<AnimationEffect> m_effect;
    bool m_isInTargetActiveAnimationsList;
    OwnPtr<AnimationEffect::CompositableValueMap> m_compositableValues;
};

} // namespace

#endif
