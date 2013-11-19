/*
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"
#include "core/rendering/RenderNamedFlowFragment.h"

#include "core/rendering/FlowThreadController.h"
#include "core/rendering/RenderBoxRegionInfo.h"
#include "core/rendering/RenderFlowThread.h"
#include "core/rendering/RenderNamedFlowThread.h"
#include "core/rendering/RenderView.h"

using namespace std;

namespace WebCore {

RenderNamedFlowFragment::RenderNamedFlowFragment()
    : RenderRegion(0, 0)
{
}

RenderNamedFlowFragment::~RenderNamedFlowFragment()
{
}

RenderNamedFlowFragment* RenderNamedFlowFragment::createAnonymous(Document* document)
{
    RenderNamedFlowFragment* region = new RenderNamedFlowFragment();
    region->setDocumentForAnonymous(document);
    return region;
}

void RenderNamedFlowFragment::setStyleForNamedFlowFragment(const RenderStyle* parentStyle)
{
    RefPtr<RenderStyle> newStyle = RenderStyle::createAnonymousStyleWithDisplay(parentStyle, BLOCK);

    newStyle->setFlowThread(parentStyle->flowThread());
    newStyle->setRegionThread(parentStyle->regionThread());
    newStyle->setRegionFragment(parentStyle->regionFragment());
    newStyle->setShapeInside(parentStyle->shapeInside());
    newStyle->setOverflowX(parentStyle->overflowX());
    newStyle->setOverflowY(parentStyle->overflowY());

    setStyle(newStyle.release());
}

void RenderNamedFlowFragment::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    RenderRegion::styleDidChange(diff, oldStyle);

    if (parent() && parent()->needsLayout())
        setNeedsLayout();
}

// FIXME: flex items as regions with flex-basis: 0 inside a flex container
// with flex-direction: column should not be treated as auto-height regions
bool RenderNamedFlowFragment::shouldHaveAutoLogicalHeight() const
{
    ASSERT(parent());

    RenderStyle* styleToUse = parent()->style();
    bool hasSpecifiedEndpointsForHeight = styleToUse->logicalTop().isSpecified() && styleToUse->logicalBottom().isSpecified();
    bool hasAnchoredEndpointsForHeight = isOutOfFlowPositioned() && hasSpecifiedEndpointsForHeight;
    bool hasAutoHeightStyle = styleToUse->logicalHeight().isAuto()
        || styleToUse->logicalHeight().isFitContent()
        || styleToUse->logicalHeight().isMaxContent()
        || styleToUse->logicalHeight().isMinContent();
    return hasAutoHeightStyle && !hasAnchoredEndpointsForHeight;
}

LayoutUnit RenderNamedFlowFragment::maxPageLogicalHeight() const
{
    ASSERT(m_flowThread);
    ASSERT(hasAutoLogicalHeight() && !m_flowThread->inConstrainedLayoutPhase());
    ASSERT(isAnonymous());
    ASSERT(parent());

    RenderStyle* styleToUse = parent()->style();
    return styleToUse->logicalMaxHeight().isUndefined() ? RenderFlowThread::maxLogicalHeight() : toRenderBlock(parent())->computeReplacedLogicalHeightUsing(styleToUse->logicalMaxHeight());
}

}
