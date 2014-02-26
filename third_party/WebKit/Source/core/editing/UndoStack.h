/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#ifndef UndoStack_h
#define UndoStack_h

#include "wtf/Deque.h"
#include "wtf/Forward.h"

namespace WebCore {

class LocalFrame;
class UndoStep;

class UndoStack {
public:
    static PassOwnPtr<UndoStack> create();

    ~UndoStack();

    void registerUndoStep(PassRefPtr<UndoStep>);
    void registerRedoStep(PassRefPtr<UndoStep>);
    void didUnloadFrame(const LocalFrame&);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

private:
    UndoStack();

    bool m_inRedo;

    typedef Deque<RefPtr<UndoStep> > UndoStepStack;
    void filterOutUndoSteps(UndoStepStack&, const LocalFrame&);
    UndoStepStack m_undoStack;
    UndoStepStack m_redoStack;
};

} // namespace WebCore

#endif
