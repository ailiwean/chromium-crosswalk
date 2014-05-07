// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/html/HTMLRTElement.h"

#include "HTMLNames.h"
#include "core/rendering/RenderRubyText.h"

namespace WebCore {

using namespace HTMLNames;

HTMLRTElement::HTMLRTElement(Document& document)
    : HTMLElement(rtTag, document)
{
}

PassRefPtrWillBeRawPtr<HTMLRTElement> HTMLRTElement::create(Document& document)
{
    return adoptRefWillBeRefCountedGarbageCollected(new HTMLRTElement(document));
}

RenderObject* HTMLRTElement::createRenderer(RenderStyle* style)
{
    if (style->display() == BLOCK)
        return new RenderRubyText(this);
    return RenderObject::createObject(this, style);
}

}
