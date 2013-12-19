/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef WTF_VectorTraits_h
#define WTF_VectorTraits_h

#include "wtf/OwnPtr.h"
#include "wtf/RefPtr.h"
#include "wtf/TypeTraits.h"
#include <utility>

using std::pair;

namespace WTF {

    class AtomicString;

    template<bool isPod, typename T>
    struct VectorTraitsBase
    {
        static const bool needsDestruction = !isPod;
        static const bool needsInitialization = !isPod;
        static const bool canInitializeWithMemset = isPod;
        static const bool canMoveWithMemcpy = isPod;
        static const bool canCopyWithMemcpy = isPod;
        static const bool canFillWithMemset = isPod && (sizeof(T) == sizeof(char));
        static const bool canCompareWithMemcmp = isPod;
        static const bool needsTracing = NeedsTracing<T>::value;
        static const bool isWeak = IsWeak<T>::value;
    };

    template<typename T>
    struct VectorTraits : VectorTraitsBase<IsPod<T>::value, T> { };

    struct SimpleClassVectorTraits : VectorTraitsBase<false, int>
    {
        static const bool canInitializeWithMemset = true;
        static const bool canMoveWithMemcpy = true;
        static const bool canCompareWithMemcmp = true;
    };

    // we know OwnPtr and RefPtr are simple enough that initializing to 0 and moving with memcpy
    // (and then not destructing the original) will totally work
    template<typename P>
    struct VectorTraits<RefPtr<P> > : SimpleClassVectorTraits { };

    template<typename P>
    struct VectorTraits<OwnPtr<P> > : SimpleClassVectorTraits { };

    template<>
    struct VectorTraits<AtomicString> : SimpleClassVectorTraits { };

    template<typename First, typename Second>
    struct VectorTraits<pair<First, Second> >
    {
        typedef VectorTraits<First> FirstTraits;
        typedef VectorTraits<Second> SecondTraits;

        static const bool needsDestruction = FirstTraits::needsDestruction || SecondTraits::needsDestruction;
        static const bool needsInitialization = FirstTraits::needsInitialization || SecondTraits::needsInitialization;
        static const bool canInitializeWithMemset = FirstTraits::canInitializeWithMemset && SecondTraits::canInitializeWithMemset;
        static const bool canMoveWithMemcpy = FirstTraits::canMoveWithMemcpy && SecondTraits::canMoveWithMemcpy;
        static const bool canCopyWithMemcpy = FirstTraits::canCopyWithMemcpy && SecondTraits::canCopyWithMemcpy;
        static const bool canFillWithMemset = false;
        static const bool canCompareWithMemcmp = FirstTraits::canCompareWithMemcmp && SecondTraits::canCompareWithMemcmp;
        static const bool needsTracing = FirstTraits::needsTracing || SecondTraits::needsTracing;
        static const bool isWeak = FirstTraits::isWeak || SecondTraits::isWeak;
    };

} // namespace WTF

using WTF::VectorTraits;
using WTF::SimpleClassVectorTraits;

#endif // WTF_VectorTraits_h
