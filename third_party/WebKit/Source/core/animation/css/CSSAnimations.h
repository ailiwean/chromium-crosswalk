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

#ifndef CSSAnimations_h
#define CSSAnimations_h

#include "core/animation/Animation.h"
#include "core/animation/AnimationPlayer.h"
#include "core/animation/InertAnimation.h"
#include "core/animation/Interpolation.h"
#include "core/animation/css/CSSAnimationData.h"
#include "core/css/StylePropertySet.h"
#include "core/dom/Document.h"
#include "core/rendering/style/RenderStyleConstants.h"
#include "wtf/HashMap.h"
#include "wtf/Vector.h"
#include "wtf/text/AtomicString.h"

namespace WebCore {

class Element;
class StylePropertyShorthand;
class StyleResolver;
class StyleRuleKeyframes;

// This class stores the CSS Animations/Transitions information we use during a style recalc.
// This includes updates to animations/transitions as well as the Interpolations to be applied.
class CSSAnimationUpdate FINAL {
public:
    void startAnimation(AtomicString& animationName, const HashSet<RefPtr<InertAnimation> >& animations)
    {
        NewAnimation newAnimation;
        newAnimation.name = animationName;
        newAnimation.animations = animations;
        m_newAnimations.append(newAnimation);
    }
    // Returns whether player has been cancelled and should be filtered during style application.
    bool isCancelledAnimation(const AnimationPlayer* player) const { return m_cancelledAnimationAnimationPlayers.contains(player); }
    void cancelAnimation(const AtomicString& name, const HashSet<RefPtr<AnimationPlayer> >& players)
    {
        m_cancelledAnimationNames.append(name);
        for (HashSet<RefPtr<AnimationPlayer> >::const_iterator iter = players.begin(); iter != players.end(); ++iter)
            m_cancelledAnimationAnimationPlayers.add(iter->get());
    }
    void toggleAnimationPaused(const AtomicString& name)
    {
        m_animationsWithPauseToggled.append(name);
    }

    void startTransition(CSSPropertyID id, const AnimatableValue* from, const AnimatableValue* to, PassRefPtr<InertAnimation> animation)
    {
        NewTransition newTransition;
        newTransition.id = id;
        newTransition.from = from;
        newTransition.to = to;
        newTransition.animation = animation;
        m_newTransitions.set(id, newTransition);
    }
    bool isCancelledTransition(CSSPropertyID id) const { return m_cancelledTransitions.contains(id); }
    void cancelTransition(CSSPropertyID id) { m_cancelledTransitions.add(id); }

    struct NewAnimation {
        AtomicString name;
        HashSet<RefPtr<InertAnimation> > animations;
    };
    const Vector<NewAnimation>& newAnimations() const { return m_newAnimations; }
    const Vector<AtomicString>& cancelledAnimationNames() const { return m_cancelledAnimationNames; }
    const HashSet<const AnimationPlayer*>& cancelledAnimationAnimationPlayers() const { return m_cancelledAnimationAnimationPlayers; }
    const Vector<AtomicString>& animationsWithPauseToggled() const { return m_animationsWithPauseToggled; }

    struct NewTransition {
        ALLOW_ONLY_INLINE_ALLOCATION();
    public:
        void trace(Visitor* visitor)
        {
            visitor->trace(from);
            visitor->trace(to);
        }

        CSSPropertyID id;
        RawPtrWillBeMember<const AnimatableValue> from;
        RawPtrWillBeMember<const AnimatableValue> to;
        RefPtr<InertAnimation> animation;
    };
    typedef WillBePersistentHeapHashMap<CSSPropertyID, NewTransition> NewTransitionMap;
    const NewTransitionMap& newTransitions() const { return m_newTransitions; }
    const HashSet<CSSPropertyID>& cancelledTransitions() const { return m_cancelledTransitions; }

    void adoptActiveInterpolationsForAnimations(HashMap<CSSPropertyID, RefPtr<Interpolation> >& newMap) { newMap.swap(m_activeInterpolationsForAnimations); }
    void adoptActiveInterpolationsForTransitions(HashMap<CSSPropertyID, RefPtr<Interpolation> >& newMap) { newMap.swap(m_activeInterpolationsForTransitions); }
    const HashMap<CSSPropertyID, RefPtr<Interpolation> >& activeInterpolationsForAnimations() const { return m_activeInterpolationsForAnimations; }
    const HashMap<CSSPropertyID, RefPtr<Interpolation> >& activeInterpolationsForTransitions() const { return m_activeInterpolationsForTransitions; }
    HashMap<CSSPropertyID, RefPtr<Interpolation> >& activeInterpolationsForAnimations() { return m_activeInterpolationsForAnimations; }

    bool isEmpty() const
    {
        return m_newAnimations.isEmpty()
            && m_cancelledAnimationNames.isEmpty()
            && m_cancelledAnimationAnimationPlayers.isEmpty()
            && m_animationsWithPauseToggled.isEmpty()
            && m_newTransitions.isEmpty()
            && m_cancelledTransitions.isEmpty()
            && m_activeInterpolationsForAnimations.isEmpty()
            && m_activeInterpolationsForTransitions.isEmpty();
    }
private:
    // Order is significant since it defines the order in which new animations
    // will be started. Note that there may be multiple animations present
    // with the same name, due to the way in which we split up animations with
    // incomplete keyframes.
    Vector<NewAnimation> m_newAnimations;
    Vector<AtomicString> m_cancelledAnimationNames;
    HashSet<const AnimationPlayer*> m_cancelledAnimationAnimationPlayers;
    Vector<AtomicString> m_animationsWithPauseToggled;

    NewTransitionMap m_newTransitions;
    HashSet<CSSPropertyID> m_cancelledTransitions;

    HashMap<CSSPropertyID, RefPtr<Interpolation> > m_activeInterpolationsForAnimations;
    HashMap<CSSPropertyID, RefPtr<Interpolation> > m_activeInterpolationsForTransitions;
};

class CSSAnimations FINAL {
public:
    // FIXME: This method is only used here and in the legacy animations
    // implementation. It should be made private or file-scope when the legacy
    // engine is removed.
    static const StyleRuleKeyframes* matchScopedKeyframesRule(StyleResolver*, const Element*, const StringImpl*);

    static bool isAnimatableProperty(CSSPropertyID);
    static const StylePropertyShorthand& animatableProperties();
    // FIXME: This should take a const ScopedStyleTree instead of a StyleResolver.
    // We should also change the Element* to a const Element*
    static PassOwnPtr<CSSAnimationUpdate> calculateUpdate(Element*, const Element& parentElement, const RenderStyle&, RenderStyle* parentStyle, StyleResolver*);

    void setPendingUpdate(PassOwnPtr<CSSAnimationUpdate> update) { m_pendingUpdate = update; }
    void maybeApplyPendingUpdate(Element*);
    bool isEmpty() const { return m_animations.isEmpty() && m_transitions.isEmpty() && !m_pendingUpdate; }
    void cancel();

private:
    // Note that a single animation name may map to multiple players due to
    // the way in which we split up animations with incomplete keyframes.
    // FIXME: Once the Web Animations model supports groups, we could use a
    // ParGroup to drive multiple animations from a single AnimationPlayer.
    typedef HashMap<AtomicString, HashSet<RefPtr<AnimationPlayer> > > AnimationMap;
    struct RunningTransition {
        ALLOW_ONLY_INLINE_ALLOCATION();
    public:
        void trace(Visitor* visitor)
        {
            visitor->trace(from);
            visitor->trace(to);
        }

        Animation* transition; // The TransitionTimeline keeps the AnimationPlayers alive
        RawPtrWillBeMember<const AnimatableValue> from;
        RawPtrWillBeMember<const AnimatableValue> to;
    };
    typedef WillBePersistentHeapHashMap<CSSPropertyID, RunningTransition> TransitionMap;
    AnimationMap m_animations;
    TransitionMap m_transitions;
    OwnPtr<CSSAnimationUpdate> m_pendingUpdate;

    HashMap<CSSPropertyID, RefPtr<Interpolation> > m_previousActiveInterpolationsForAnimations;

    static void calculateAnimationUpdate(CSSAnimationUpdate*, Element*, const Element& parentElement, const RenderStyle&, RenderStyle* parentStyle, StyleResolver*);
    static void calculateTransitionUpdate(CSSAnimationUpdate*, const Element*, const RenderStyle&);
    static void calculateTransitionUpdateForProperty(CSSPropertyID, const CSSAnimationData*, const RenderStyle& oldStyle, const RenderStyle&, const TransitionMap* activeTransitions, CSSAnimationUpdate*, const Element*);

    static void calculateAnimationActiveInterpolations(CSSAnimationUpdate*, const Element*);
    static void calculateTransitionActiveInterpolations(CSSAnimationUpdate*, const Element*);

    class AnimationEventDelegate FINAL : public TimedItem::EventDelegate {
    public:
        AnimationEventDelegate(Element* target, const AtomicString& name)
            : m_target(target)
            , m_name(name)
        {
        }
        virtual void onEventCondition(const TimedItem*, bool isFirstSample, TimedItem::Phase previousPhase, double previousIteration) OVERRIDE;
    private:
        void maybeDispatch(Document::ListenerType, const AtomicString& eventName, double elapsedTime);
        Element* m_target;
        const AtomicString m_name;
    };

    class TransitionEventDelegate FINAL : public TimedItem::EventDelegate {
    public:
        TransitionEventDelegate(Element* target, CSSPropertyID property)
            : m_target(target)
            , m_property(property)
        {
        }
        virtual void onEventCondition(const TimedItem*, bool isFirstSample, TimedItem::Phase previousPhase, double previousIteration) OVERRIDE;
    private:
        Element* m_target;
        const CSSPropertyID m_property;
    };
};

} // namespace WebCore

#endif
