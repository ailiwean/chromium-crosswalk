/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Profiler_h
#define Profiler_h

#include "Profile.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace KJS {

    class CallIdentifier;
    class ExecState;
    class JSObject;
    class ProfileGenerator;
    class UString;

    class Profiler {
    public:
        static Profiler** enabledProfilerReference()
        {
            return &s_sharedEnabledProfilerReference;
        }

        static Profiler* profiler(); 
        static CallIdentifier createCallIdentifier(ExecState*, JSObject*, const UString& sourceURL, int lineNumber);

        void startProfiling(ExecState*, const UString& title);
        PassRefPtr<Profile> stopProfiling(ExecState*, const UString& title);

        void willExecute(ExecState*, JSObject* calledFunction);
        void willExecute(ExecState*, const UString& sourceURL, int startingLineNumber);
        void didExecute(ExecState*, JSObject* calledFunction);
        void didExecute(ExecState*, const UString& sourceURL, int startingLineNumber);

        const Vector<RefPtr<ProfileGenerator> >& currentProfiles() { return m_currentProfiles; };

    private:
        Vector<RefPtr<ProfileGenerator> > m_currentProfiles;
        static Profiler* s_sharedProfiler;
        static Profiler* s_sharedEnabledProfilerReference;
    };

} // namespace KJS

#endif // Profiler_h
