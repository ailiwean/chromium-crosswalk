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

#ifndef EditorClientImpl_h
#define EditorClientImpl_h

#include "core/page/EditorClient.h"
#include "wtf/Deque.h"

namespace WebCore {
class Frame;
class HTMLInputElement;
}

namespace blink {
class WebViewImpl;

class EditorClientImpl : public WebCore::EditorClient {
public:
    EditorClientImpl(WebViewImpl*);

    virtual ~EditorClientImpl();

    virtual void respondToChangedContents() OVERRIDE;
    virtual void respondToChangedSelection(WebCore::Frame*) OVERRIDE;
    virtual void didCancelCompositionOnSelectionChange() OVERRIDE;
    virtual void registerUndoStep(PassRefPtr<WebCore::UndoStep>) OVERRIDE;
    virtual void registerRedoStep(PassRefPtr<WebCore::UndoStep>) OVERRIDE;
    virtual void clearUndoRedoOperations() OVERRIDE;
    virtual bool canCopyCut(WebCore::Frame*, bool defaultValue) const OVERRIDE;
    virtual bool canPaste(WebCore::Frame*, bool defaultValue) const OVERRIDE;
    virtual bool canUndo() const OVERRIDE;
    virtual bool canRedo() const OVERRIDE;
    virtual void undo() OVERRIDE;
    virtual void redo() OVERRIDE;
    virtual void handleKeyboardEvent(WebCore::KeyboardEvent*) OVERRIDE;
    virtual void textFieldDidEndEditing(WebCore::Element*) OVERRIDE;
    virtual void textDidChangeInTextField(WebCore::Element*) OVERRIDE;
    virtual bool doTextFieldCommandFromEvent(WebCore::Element*, WebCore::KeyboardEvent*) OVERRIDE;
    virtual void willSetInputMethodState() OVERRIDE;

    const char* interpretKeyEvent(const WebCore::KeyboardEvent*);

private:
    bool handleEditingKeyboardEvent(WebCore::KeyboardEvent*);
    void modifySelection(WebCore::Frame*, WebCore::KeyboardEvent*);

    WebViewImpl* m_webView;
    bool m_inRedo;

    typedef Deque<RefPtr<WebCore::UndoStep> > UndoManagerStack;
    UndoManagerStack m_undoStack;
    UndoManagerStack m_redoStack;
};

} // namespace blink

#endif
