/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
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
  
#include "dom_selection.h"

#include "dom_caretposition.h"
#include "dom_docimpl.h"
#include "dom_elementimpl.h"
#include "dom_node.h"
#include "dom_nodeimpl.h"
#include "dom_positioniterator.h"
#include "dom_string.h"
#include "dom_textimpl.h"
#include "dom2_rangeimpl.h"
#include "htmltags.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "qevent.h"
#include "qpainter.h"
#include "qrect.h"
#include "render_object.h"
#include "render_style.h"
#include "render_text.h"

#if APPLE_CHANGES
#include "KWQAssertions.h"
#else
#define ASSERT(assertion) assert(assertion)
#endif

#define EDIT_DEBUG 0

using khtml::findWordBoundary;
using khtml::InlineTextBox;
using khtml::RenderObject;
using khtml::RenderText;

namespace DOM {

static Selection selectionForLine(const Position &position);

Selection::Selection()
{
    init();
}

Selection::Selection(const Position &pos)
    : m_base(pos), m_extent(pos)
{
    init();
    validate();
}

Selection::Selection(const Range &r)
    : m_base(startPosition(r)), m_extent(endPosition(r))
{
    init();
    validate();
}

Selection::Selection(const Position &base, const Position &extent)
    : m_base(base), m_extent(extent)
{
    init();
    validate();
}

Selection::Selection(const CaretPosition &base, const CaretPosition &extent)
    : m_base(base.position()), m_extent(extent.position())
{
    init();
    validate();
}

Selection::Selection(const Selection &o)
    : m_base(o.m_base), m_extent(o.m_extent)
    , m_start(o.m_start), m_end(o.m_end)
    , m_state(o.m_state), m_affinity(o.m_affinity)
    , m_baseIsStart(o.m_baseIsStart)
    , m_needsCaretLayout(o.m_needsCaretLayout)
    , m_modifyBiasSet(o.m_modifyBiasSet)
{
    // Only copy the coordinates over if the other object
    // has had a layout, otherwise keep the current
    // coordinates. This prevents drawing artifacts from
    // remaining when the caret is painted and then moves,
    // and the old rectangle needs to be repainted.
    if (!m_needsCaretLayout) {
        m_caretRect = o.m_caretRect;
    }
}

void Selection::init()
{
    m_state = NONE; 
    m_affinity = DOWNSTREAM;
    m_baseIsStart = true;
    m_needsCaretLayout = true;
    m_modifyBiasSet = false;
}

Selection &Selection::operator=(const Selection &o)
{
    m_base = o.m_base;
    m_extent = o.m_extent;
    m_start = o.m_start;
    m_end = o.m_end;

    m_state = o.m_state;
    m_affinity = o.m_affinity;

    m_baseIsStart = o.m_baseIsStart;
    m_needsCaretLayout = o.m_needsCaretLayout;
    m_modifyBiasSet = o.m_modifyBiasSet;
    
    // Only copy the coordinates over if the other object
    // has had a layout, otherwise keep the current
    // coordinates. This prevents drawing artifacts from
    // remaining when the caret is painted and then moves,
    // and the old rectangle needs to be repainted.
    if (!m_needsCaretLayout) {
        m_caretRect = o.m_caretRect;
    }
    
    return *this;
}

void Selection::setAffinity(EAffinity affinity)
{
    if (affinity == m_affinity)
        return;
        
    m_affinity = affinity;
    setNeedsLayout();
}

void Selection::moveTo(const Range &r)
{
    m_base = startPosition(r);
    m_extent = endPosition(r);
    validate();
}

void Selection::moveTo(const Selection &o)
{
    m_base = o.m_start;
    m_extent = o.m_end;
    validate();
}

void Selection::moveTo(const Position &pos)
{
    m_base = pos;
    m_extent = pos;
    validate();
}

void Selection::moveTo(const Position &base, const Position &extent)
{
    m_base = base;
    m_extent = extent;
    validate();
}

void Selection::setModifyBias(EAlter alter, EDirection direction)
{
    switch (alter) {
        case MOVE:
            m_modifyBiasSet = false;
            break;
        case EXTEND:
            if (!m_modifyBiasSet) {
                m_modifyBiasSet = true;
                switch (direction) {
                    // FIXME: right for bidi?
                    case RIGHT:
                    case FORWARD:
                        m_base = m_start;
                        m_extent = m_end;
                        break;
                    case LEFT:
                    case BACKWARD:
                        m_base = m_end;
                        m_extent = m_start;
                        break;
                }
            }
            break;
    }
}

CaretPosition Selection::modifyExtendingRightForward(ETextGranularity granularity)
{
    CaretPosition pos = m_extent;
    switch (granularity) {
        case CHARACTER:
            pos = pos.next();
            break;
        case WORD:
            pos = nextWordPosition(pos);
            break;
        case PARAGRAPH:
            pos = nextParagraphPosition(pos, xPosForVerticalArrowNavigation(EXTENT));
            break;
        case LINE:
            pos = nextLinePosition(pos, xPosForVerticalArrowNavigation(EXTENT));
            break;
        case LINE_BOUNDARY:
            pos = selectionForLine(m_end).end();
            break;
        case PARAGRAPH_BOUNDARY:
            pos = endOfParagraph(m_end);
            break;
        case DOCUMENT_BOUNDARY: {
            NodeImpl *de = m_start.node()->getDocument()->documentElement();
            pos = Position(de, de ? de->childNodeCount() : 0);
            break;
        }
    }
    return pos;
}

CaretPosition Selection::modifyMovingRightForward(ETextGranularity granularity)
{
    CaretPosition pos;
    switch (granularity) {
        case CHARACTER:
            if (isRange()) 
                pos = m_end;
            else
                pos = CaretPosition(m_extent).next();
            break;
        case WORD:
            pos = nextWordPosition(m_extent);
            break;
        case PARAGRAPH:
            pos = nextParagraphPosition(m_end, xPosForVerticalArrowNavigation(END, isRange()));
            break;
        case LINE:
            pos = nextLinePosition(m_end, xPosForVerticalArrowNavigation(END, isRange()));
            break;
        case LINE_BOUNDARY:
            pos = selectionForLine(m_end).end();
            break;
        case PARAGRAPH_BOUNDARY:
            pos = endOfParagraph(m_end);
            break;
        case DOCUMENT_BOUNDARY: {
            NodeImpl *de = m_start.node()->getDocument()->documentElement();
            pos = Position(de, de ? de->childNodeCount() : 0);
            break;
        }
    }
    return pos;
}

CaretPosition Selection::modifyExtendingLeftBackward(ETextGranularity granularity)
{
    CaretPosition pos = m_extent;
    switch (granularity) {
        case CHARACTER:
            pos = pos.previous();
            break;
        case WORD:
            pos = previousWordPosition(pos);
            break;
        case PARAGRAPH:
            pos = previousParagraphPosition(pos, xPosForVerticalArrowNavigation(EXTENT));
            break;
        case LINE:
            pos = previousLinePosition(pos, xPosForVerticalArrowNavigation(EXTENT));
            break;
        case LINE_BOUNDARY:
            pos = selectionForLine(m_start).start();
            break;
        case PARAGRAPH_BOUNDARY:
            pos = startOfParagraph(m_start);
            break;
        case DOCUMENT_BOUNDARY:
            pos = CaretPosition(m_start.node()->getDocument()->documentElement(), 0);
            break;
    }
    return pos;
}

CaretPosition Selection::modifyMovingLeftBackward(ETextGranularity granularity)
{
    CaretPosition pos;
    switch (granularity) {
        case CHARACTER:
            if (isRange()) 
                pos = m_start;
            else
                pos = CaretPosition(m_extent).previous();
            break;
        case WORD:
            pos = previousWordPosition(m_extent);
            break;
        case PARAGRAPH:
            pos = previousParagraphPosition(m_start, xPosForVerticalArrowNavigation(START, isRange()));
            break;
        case LINE:
            pos = previousLinePosition(m_start, xPosForVerticalArrowNavigation(START, isRange()));
            break;
        case LINE_BOUNDARY:
            pos = selectionForLine(m_start).start();
            break;
        case PARAGRAPH_BOUNDARY:
            pos = startOfParagraph(m_start).deepEquivalent();
            break;
        case DOCUMENT_BOUNDARY:
            pos = CaretPosition(m_start.node()->getDocument()->documentElement(), 0);
            break;
    }
    return pos;
}

bool Selection::modify(EAlter alter, EDirection dir, ETextGranularity granularity)
{
    setModifyBias(alter, dir);

    CaretPosition pos;

    switch (dir) {
        // EDIT FIXME: These need to handle bidi
        case RIGHT:
        case FORWARD:
            if (alter == EXTEND)
                pos = modifyExtendingRightForward(granularity);
            else
                pos = modifyMovingRightForward(granularity);
            break;
        case LEFT:
        case BACKWARD:
            if (alter == EXTEND)
                pos = modifyExtendingLeftBackward(granularity);
            else
                pos = modifyMovingLeftBackward(granularity);
            break;
    }

    if (pos.isNull())
        return false;

    switch (alter) {
        case MOVE:
            moveTo(pos.deepEquivalent());
            break;
        case EXTEND:
            setExtent(pos.deepEquivalent());
            break;
    }

    return true;
}

// FIXME: Maybe baseline would be better?
static bool caretY(const CaretPosition &c, int &y)
{
    Position p = c.deepEquivalent();
    NodeImpl *n = p.node();
    if (!n)
        return false;
    RenderObject *r = p.node()->renderer();
    if (!r)
        return false;
    QRect rect = r->caretRect(p.offset(), false);
    if (rect.isEmpty())
        return false;
    y = rect.y() + rect.height() / 2;
    return true;
}

bool Selection::modify(EAlter alter, int verticalDistance)
{
    if (verticalDistance == 0) {
        return false;
    }

    setModifyBias(alter, verticalDistance > 0 ? FORWARD : BACKWARD);

    CaretPosition pos;

    int xPos = 0; /* initialized only to make compiler happy */
    switch (alter) {
        case MOVE:
            pos = verticalDistance > 0 ? m_end : m_start;
            xPos = xPosForVerticalArrowNavigation(verticalDistance > 0 ? END : START, isRange());
            break;
        case EXTEND:
            pos = m_extent;
            xPos = xPosForVerticalArrowNavigation(EXTENT);
            break;
    }

    int startY;
    if (!caretY(pos, startY))
        return false;
    if (verticalDistance < 0)
        startY = -startY;
    int lastY = startY;

    CaretPosition result;

    CaretPosition next;
    for (CaretPosition p = pos; ; p = next) {
        next = verticalDistance > 0
            ? nextLinePosition(p, xPos)
            : previousLinePosition(p, xPos);
        if (next.isNull() || next == p)
            break;
        int nextY;
        if (!caretY(next, nextY))
            break;
        if (verticalDistance < 0)
            nextY = -nextY;
        if (nextY - startY > verticalDistance)
            break;
        if (nextY >= lastY) {
            lastY = nextY;
            result = next;
        }
    }

    if (result.isNull())
        return false;

    switch (alter) {
        case MOVE:
            moveTo(result.deepEquivalent());
            break;
        case EXTEND:
            setExtent(result.deepEquivalent());
            break;
    }

    return true;
}

bool Selection::expandUsingGranularity(ETextGranularity granularity)
{
    if (isNone())
        return false;
    validate(granularity);
    return true;
}

int Selection::xPosForVerticalArrowNavigation(EPositionType type, bool recalc) const
{
    int x = 0;

    if (isNone())
        return x;

    Position pos;
    switch (type) {
        case START:
            pos = m_start;
            break;
        case END:
            pos = m_end;
            break;
        case BASE:
            pos = m_base;
            break;
        case EXTENT:
            pos = m_extent;
            break;
    }

    KHTMLPart *part = pos.node()->getDocument()->part();
    if (!part)
        return x;
        
    if (recalc || part->xPosForVerticalArrowNavigation() == KHTMLPart::NoXPosForVerticalArrowNavigation) {
        x = pos.node()->renderer()->caretRect(pos.offset(), false).x();
        part->setXPosForVerticalArrowNavigation(x);
    }
    else {
        x = part->xPosForVerticalArrowNavigation();
    }

    return x;
}

void Selection::clear()
{
    m_base.clear();
    m_extent.clear();
    validate();
}

void Selection::setBase(const Position &pos)
{
    m_base = pos;
    validate();
}

void Selection::setExtent(const Position &pos)
{
    m_extent = pos;
    validate();
}

void Selection::setBaseAndExtent(const Position &base, const Position &extent)
{
    m_base = base;
    m_extent = extent;
    validate();
}

void Selection::setNeedsLayout(bool flag)
{
    m_needsCaretLayout = flag;
}

Range Selection::toRange() const
{
    if (isNone())
        return Range();

    // Make sure we have an updated layout since this function is called
    // in the course of running edit commands which modify the DOM.
    // Failing to call this can result in equivalentXXXPosition calls returning
    // incorrect results.
    m_start.node()->getDocument()->updateLayout();

    Position s, e;
    if (isCaret()) {
        // If the selection is a caret, move the range start upstream. This helps us match
        // the conventions of text editors tested, which make style determinations based
        // on the character before the caret, if any. 
        s = m_start.upstream(StayInBlock).equivalentRangeCompliantPosition();
        e = s;
    }
    else {
        // If the selection is a range, select the minimum range that encompasses the selection.
        // Again, this is to match the conventions of text editors tested, which make style 
        // determinations based on the first character of the selection. 
        // For instance, this operation helps to make sure that the "X" selected below is the 
        // only thing selected. The range should not be allowed to "leak" out to the end of the 
        // previous text node, or to the beginning of the next text node, each of which has a 
        // different style.
        // 
        // On a treasure map, <b>X</b> marks the spot.
        //                       ^ selected
        //
        ASSERT(isRange());
        s = m_start.downstream();
        e = m_end.upstream();
        if (RangeImpl::compareBoundaryPoints(s.node(), s.offset(), e.node(), e.offset()) > 0) {
            // Make sure the start is before the end.
            // The end can wind up before the start if collapsed whitespace is the only thing selected.
            Position tmp = s;
            s = e;
            e = tmp;
        }
        s = s.equivalentRangeCompliantPosition();
        e = e.equivalentRangeCompliantPosition();
    }

    return Range(s.node(), s.offset(), e.node(), e.offset());
}

void Selection::layoutCaret()
{
    if (!isCaret() || !m_start.node()->inDocument()) {
        m_caretRect = QRect();
        return;
    }
    
    // EDIT FIXME: Enhance call to pass along selection 
    // upstream/downstream affinity to get the right position.
    m_caretRect = m_start.node()->renderer()->caretRect(m_start.offset(), false);

    m_needsCaretLayout = false;
}

QRect Selection::caretRect() const
{
    if (m_needsCaretLayout) {
        const_cast<Selection *>(this)->layoutCaret();
    }

    return m_caretRect;
}

QRect Selection::caretRepaintRect() const
{
    // FIXME: Add one pixel of slop on each side to make sure we don't leave behind artifacts.
    QRect r = caretRect();
    if (r.isEmpty())
        return QRect();
    return QRect(r.left() - 1, r.top() - 1, r.width() + 2, r.height() + 2);
}

void Selection::needsCaretRepaint()
{
    if (!isCaret())
        return;

    if (!m_start.node()->getDocument())
        return;

    KHTMLView *v = m_start.node()->getDocument()->view();
    if (!v)
        return;

    if (m_needsCaretLayout) {
        // repaint old position and calculate new position
        v->updateContents(caretRepaintRect(), false);
        layoutCaret();
        
        // EDIT FIXME: This is an unfortunate hack.
        // Basically, we can't trust this layout position since we 
        // can't guarantee that the check to see if we are in unrendered 
        // content will work at this point. We may have to wait for
        // a layout and re-render of the document to happen. So, resetting this
        // flag will cause another caret layout to happen the first time
        // that we try to paint the caret after this call. That one will work since
        // it happens after the document has accounted for any editing
        // changes which may have been done.
        // And, we need to leave this layout here so the caret moves right 
        // away after clicking.
        m_needsCaretLayout = true;
    }
    v->updateContents(caretRepaintRect(), false);
}

void Selection::paintCaret(QPainter *p, const QRect &rect)
{
    if (m_state != CARET)
        return;

    if (m_needsCaretLayout)
        layoutCaret();

    if (m_caretRect.isValid())
        p->fillRect(m_caretRect & rect, QBrush());
}

void Selection::validate(ETextGranularity granularity)
{
    // Move the selection to rendered positions, if possible.
    Position originalBase(m_base);
    bool baseAndExtentEqual = m_base == m_extent;
    bool updatedLayout = false;
    if (m_base.isNotNull()) {
        m_base.node()->getDocument()->updateLayout();
        updatedLayout = true;
        m_base = m_base.equivalentDeepPosition().closestRenderedPosition(affinity());
        if (baseAndExtentEqual)
            m_extent = m_base;
    }
    if (m_extent.isNotNull() && !baseAndExtentEqual) {
        if (!updatedLayout)
            m_extent.node()->getDocument()->updateLayout();
        m_extent = m_extent.equivalentDeepPosition().closestRenderedPosition(affinity());
    }

    // Make sure we do not have a dangling start or end
    if (m_base.isNull() && m_extent.isNull()) {
        // Move the position to the enclosingBlockFlowElement of the original base, if possible.
        // This has the effect of flashing the caret somewhere when a rendered position for
        // the base and extent cannot be found.
        if (originalBase.isNotNull()) {
            Position pos(originalBase.node()->enclosingBlockFlowElement(), 0);
            m_base = pos;
            m_extent = pos;
        }
        else {
            // We have no position to work with. See if the BODY element of the page
            // is contentEditable. If it is, put the caret there.
            //NodeImpl *node = document()
            m_start.clear();
            m_end.clear();
        }
        m_baseIsStart = true;
    }
    else if (m_base.isNull()) {
        m_base = m_extent;
        m_baseIsStart = true;
    }
    else if (m_extent.isNull()) {
        m_extent = m_base;
        m_baseIsStart = true;
    }
    else {
        m_baseIsStart = RangeImpl::compareBoundaryPoints(m_base.node(), m_base.offset(), m_extent.node(), m_extent.offset()) <= 0;
    }

    m_start.clear();
    m_end.clear();

    // calculate the correct start and end positions
    switch (granularity) {
        case CHARACTER:
            if (m_baseIsStart) {
                m_start = m_base;
                m_end = m_extent;
            } else {
                m_start = m_extent;
                m_end = m_base;
            }
            break;
        case WORD:
            if (m_baseIsStart) {
                m_start = startOfWord(m_base).deepEquivalent();
                m_end = endOfWord(m_extent).deepEquivalent();
            } else {
                m_start = startOfWord(m_extent).deepEquivalent();
                m_end = endOfWord(m_base).deepEquivalent();
            }
            break;
        case LINE:
        case LINE_BOUNDARY: {
            Selection baseSelection = *this;
            Selection extentSelection = *this;
            Selection baseLine = selectionForLine(m_base);
            if (baseLine.isCaretOrRange()) {
                baseSelection = baseLine;
            }
            Selection extentLine = selectionForLine(m_extent);
            if (extentLine.isCaretOrRange()) {
                extentSelection = extentLine;
            }
            if (m_baseIsStart) {
                m_start = baseSelection.m_start;
                m_end = extentSelection.m_end;
            } else {
                m_start = extentSelection.m_start;
                m_end = baseSelection.m_end;
            }
            break;
        }
        case PARAGRAPH:
            if (m_baseIsStart) {
                m_start = startOfParagraph(m_base).deepEquivalent();
                m_end = endOfParagraph(m_extent, IncludeLineBreak).deepEquivalent();
            } else {
                m_start = startOfParagraph(m_extent).deepEquivalent();
                m_end = endOfParagraph(m_base, IncludeLineBreak).deepEquivalent();
            }
            break;
        case DOCUMENT_BOUNDARY: {
            NodeImpl *de = m_start.node()->getDocument()->documentElement();
            m_start = CaretPosition(de, 0).deepEquivalent();
            m_end = CaretPosition(de, de ? de->childNodeCount() : 0).deepEquivalent();
            break;
        }
        case PARAGRAPH_BOUNDARY:
            if (m_baseIsStart) {
                m_start = startOfParagraph(m_base).deepEquivalent();
                m_end = endOfParagraph(m_extent).deepEquivalent();
            } else {
                m_start = startOfParagraph(m_extent).deepEquivalent();
                m_end = endOfParagraph(m_base).deepEquivalent();
            }
            break;
    }

    // adjust the state
    if (m_start.isNull()) {
        ASSERT(m_end.isNull());
        m_state = NONE;
    }
    else if (m_start == m_end || m_start.upstream(StayInBlock) == m_end.upstream(StayInBlock)) {
        m_state = CARET;
    }
    else {
        m_state = RANGE;
        // "Constrain" the selection to be the smallest equivalent range of nodes.
        // This is a somewhat arbitrary choice, but experience shows that it is
        // useful to make to make the selection "canonical" (if only for
        // purposes of comparing selections). This is an ideal point of the code
        // to do this operation, since all selection changes that result in a RANGE 
        // come through here before anyone uses it.
        m_start = m_start.downstream(StayInBlock);
        m_end = m_end.upstream(StayInBlock);
    }

    m_needsCaretLayout = true;
    
#if EDIT_DEBUG
    debugPosition();
#endif
}

static Position startOfFirstRunAt(RenderObject *renderNode, int y)
{
    for (RenderObject *n = renderNode; n; n = n->nextSibling()) {
        if (n->isText()) {
            RenderText *textRenderer = static_cast<RenderText *>(n);
            for (InlineTextBox* box = textRenderer->firstTextBox(); box; box = box->nextTextBox())
                if (box->m_y == y)
                    return Position(textRenderer->element(), box->m_start);
        }
        
        Position position = startOfFirstRunAt(n->firstChild(), y);
        if (position.isNotNull())
            return position;
    }
    
    return Position();
}

static Position endOfLastRunAt(RenderObject *renderNode, int y)
{
    RenderObject *n = renderNode;
    if (!n)
        return Position();
    if (RenderObject *parent = n->parent())
        n = parent->lastChild();
    
    while (1) {
        Position position = endOfLastRunAt(n->firstChild(), y);
        if (position.isNotNull())
            return position;
        
        if (n->isText()) {
            RenderText *textRenderer = static_cast<RenderText *>(n);
            for (InlineTextBox* box = textRenderer->lastTextBox(); box; box = box->prevTextBox())
                if (box->m_y == y)
                    return Position(textRenderer->element(), box->m_start + box->m_len);
        }
        
        if (n == renderNode)
            return Position();
        
        n = n->previousSibling();
    }
}

static Selection selectionForLine(const Position &position)
{
    NodeImpl *node = position.node();

    if (!node)
        return Selection();

    switch (node->nodeType()) {
        case Node::TEXT_NODE:
        case Node::CDATA_SECTION_NODE:
            break;
        default:
            return Selection();
    }

    RenderText *renderer = static_cast<RenderText *>(node->renderer());

    int pos;
    InlineTextBox *run = renderer->findNextInlineTextBox(position.offset(), pos);
    if (!run)
        return Selection();
        
    int selectionPointY = run->m_y;
    
    // Go up to first non-inline element.
    RenderObject *renderNode = renderer;
    while (renderNode && renderNode->isInline())
        renderNode = renderNode->parent();
    renderNode = renderNode->firstChild();
    
    // Look for all the first child in the block that is on the same line
    // as the selection point.
    Position start = startOfFirstRunAt(renderNode, selectionPointY);
    if (start.isNull())
        return Selection();

    // Look for all the last child in the block that is on the same line
    // as the selection point.
    Position end = endOfLastRunAt(renderNode, selectionPointY);
    if (end.isNull())
        return Selection();
    
    return Selection(start, end);
}

void Selection::debugRenderer(RenderObject *r, bool selected) const
{
    if (r->node()->isElementNode()) {
        ElementImpl *element = static_cast<ElementImpl *>(r->node());
        fprintf(stderr, "%s%s\n", selected ? "==> " : "    ", element->tagName().string().latin1());
    }
    else if (r->isText()) {
        RenderText *textRenderer = static_cast<RenderText *>(r);
        if (textRenderer->stringLength() == 0 || !textRenderer->firstTextBox()) {
            fprintf(stderr, "%s#text (empty)\n", selected ? "==> " : "    ");
            return;
        }
        
        static const int max = 36;
        QString text = DOMString(textRenderer->string()).string();
        int textLength = text.length();
        if (selected) {
            int offset = 0;
            if (r->node() == m_start.node())
                offset = m_start.offset();
            else if (r->node() == m_end.node())
                offset = m_end.offset();
                
            int pos;
            InlineTextBox *box = textRenderer->findNextInlineTextBox(offset, pos);
            text = text.mid(box->m_start, box->m_len);
            
            QString show;
            int mid = max / 2;
            int caret = 0;
            
            // text is shorter than max
            if (textLength < max) {
                show = text;
                caret = pos;
            }
            
            // too few characters to left
            else if (pos - mid < 0) {
                show = text.left(max - 3) + "...";
                caret = pos;
            }
            
            // enough characters on each side
            else if (pos - mid >= 0 && pos + mid <= textLength) {
                show = "..." + text.mid(pos - mid + 3, max - 6) + "...";
                caret = mid;
            }
            
            // too few characters on right
            else {
                show = "..." + text.right(max - 3);
                caret = pos - (textLength - show.length());
            }
            
            show = show.replace("\n", " ");
            show = show.replace("\r", " ");
            fprintf(stderr, "==> #text : \"%s\" at offset %d\n", show.latin1(), pos);
            fprintf(stderr, "           ");
            for (int i = 0; i < caret; i++)
                fprintf(stderr, " ");
            fprintf(stderr, "^\n");
        }
        else {
            if ((int)text.length() > max)
                text = text.left(max - 3) + "...";
            else
                text = text.left(max);
            fprintf(stderr, "    #text : \"%s\"\n", text.latin1());
        }
    }
}

void Selection::debugPosition() const
{
    if (!m_start.node())
        return;

    //static int context = 5;
    
    //RenderObject *r = 0;

    fprintf(stderr, "Selection =================\n");

    if (m_start == m_end) {
        Position pos = m_start;
        Position upstream = pos.upstream();
        Position downstream = pos.downstream();
        fprintf(stderr, "upstream:   %s %p:%d\n", getTagName(upstream.node()->id()).string().latin1(), upstream.node(), upstream.offset());
        fprintf(stderr, "pos:        %s %p:%d\n", getTagName(pos.node()->id()).string().latin1(), pos.node(), pos.offset());
        fprintf(stderr, "downstream: %s %p:%d\n", getTagName(downstream.node()->id()).string().latin1(), downstream.node(), downstream.offset());
    }
    else {
        Position pos = m_start;
        Position upstream = pos.upstream();
        Position downstream = pos.downstream();
        fprintf(stderr, "upstream:   %s %p:%d\n", getTagName(upstream.node()->id()).string().latin1(), upstream.node(), upstream.offset());
        fprintf(stderr, "start:      %s %p:%d\n", getTagName(pos.node()->id()).string().latin1(), pos.node(), pos.offset());
        fprintf(stderr, "downstream: %s %p:%d\n", getTagName(downstream.node()->id()).string().latin1(), downstream.node(), downstream.offset());
        fprintf(stderr, "-----------------------------------\n");
        pos = m_end;
        upstream = pos.upstream();
        downstream = pos.downstream();
        fprintf(stderr, "upstream:   %s %p:%d\n", getTagName(upstream.node()->id()).string().latin1(), upstream.node(), upstream.offset());
        fprintf(stderr, "end:        %s %p:%d\n", getTagName(pos.node()->id()).string().latin1(), pos.node(), pos.offset());
        fprintf(stderr, "downstream: %s %p:%d\n", getTagName(downstream.node()->id()).string().latin1(), downstream.node(), downstream.offset());
        fprintf(stderr, "-----------------------------------\n");
    }
          
#if 0
    int back = 0;
    r = m_start.node()->renderer();
    for (int i = 0; i < context; i++, back++) {
        if (r->previousRenderer())
            r = r->previousRenderer();
        else
            break;
    }
    for (int i = 0; i < back; i++) {
        debugRenderer(r, false);
        r = r->nextRenderer();
    }


    fprintf(stderr, "\n");

    if (m_start.node() == m_end.node())
        debugRenderer(m_start.node()->renderer(), true);
    else
        for (r = m_start.node()->renderer(); r && r != m_end.node()->renderer(); r = r->nextRenderer())
            debugRenderer(r, true);
    
    fprintf(stderr, "\n");
    
    r = m_end.node()->renderer();
    for (int i = 0; i < context; i++) {
        if (r->nextRenderer()) {
            r = r->nextRenderer();
            debugRenderer(r, false);
        }
        else
            break;
    }
#endif

    fprintf(stderr, "================================\n");
}

#ifndef NDEBUG
#define FormatBufferSize 1024
void Selection::formatForDebugger(char *buffer, unsigned length) const
{
    DOMString result;
    DOMString s;
    
    if (isNone()) {
        result = "<none>";
    }
    else {
        char s[FormatBufferSize];
        result += "from ";
        m_start.formatForDebugger(s, FormatBufferSize);
        result += s;
        result += " to ";
        m_end.formatForDebugger(s, FormatBufferSize);
        result += s;
    }
          
    strncpy(buffer, result.string().latin1(), length - 1);
}
#undef FormatBufferSize
#endif

} // namespace DOM
