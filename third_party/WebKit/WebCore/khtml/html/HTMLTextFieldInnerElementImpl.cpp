/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
 
#include "config.h"
#include "HTMLTextFieldInnerElementImpl.h"

#include "BeforeTextInsertedEventImpl.h"
#include "EventNames.h"
#include "dom2_eventsimpl.h"
#include "html_documentimpl.h"
#include "HTMLInputElementImpl.h"
#include "htmlnames.h"
#include "render_object.h"
#include "RenderTextField.h"

namespace WebCore {

using namespace HTMLNames;
using namespace EventNames;

HTMLTextFieldInnerElementImpl::HTMLTextFieldInnerElementImpl(DocumentImpl* doc, NodeImpl* shadowParent)
    : HTMLDivElementImpl(doc), m_shadowParent(shadowParent)
{
}

void HTMLTextFieldInnerElementImpl::defaultEventHandler(EventImpl* evt)
{
    // FIXME:  In the future, we should add a way to have default event listeners.  Then we would add one to the text field's inner div, and we wouldn't need this subclass.
    if (shadowParentNode() && shadowParentNode()->hasTagName(inputTag) && shadowParentNode()->renderer() && shadowParentNode()->renderer()->isTextField()) {
        if (evt->isBeforeTextInsertedEvent())
            static_cast<HTMLInputElementImpl*>(shadowParentNode())->defaultEventHandler(evt);

        if (evt->type() == khtmlEditableContentChangedEvent) {
            if (shadowParentNode()->renderer()->style()->appearance() == TextFieldAppearance) 
                static_cast<RenderTextField*>(shadowParentNode()->renderer())->subtreeHasChanged();
        }
    }
    HTMLDivElementImpl::defaultEventHandler(evt);
}

}
