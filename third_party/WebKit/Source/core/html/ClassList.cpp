/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "core/html/ClassList.h"

#include "core/dom/SpaceSplitString.h"
#include <wtf/text/StringBuilder.h>

namespace WebCore {

using namespace HTMLNames;

ClassList::ClassList(Element* element) : m_element(element) { }

void ClassList::ref()
{
    m_element->ref();
}

void ClassList::deref()
{
    m_element->deref();
}

unsigned ClassList::length() const
{
    return m_element->hasClass() ? classNames().size() : 0;
}

const AtomicString ClassList::item(unsigned index) const
{
    if (index >= length())
        return AtomicString();
    return classNames()[index];
}

bool ClassList::containsInternal(const AtomicString& token) const
{
    return m_element->hasClass() && classNames().contains(token);
}

const SpaceSplitString& ClassList::classNames() const
{
    ASSERT(m_element->hasClass());
    if (m_element->document()->inQuirksMode()) {
        if (!m_classNamesForQuirksMode)
            m_classNamesForQuirksMode = adoptPtr(new SpaceSplitString(value(), false));
        return *m_classNamesForQuirksMode.get();
    }
    return m_element->elementData()->classNames();
}

} // namespace WebCore
