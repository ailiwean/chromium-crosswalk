/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
 * (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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
 */

#include "config.h"
#include "Range.h"
#include "RangeException.h"

#include "CString.h"
#include "Document.h"
#include "DocumentFragment.h"
#include "ExceptionCode.h"
#include "HTMLElement.h"
#include "HTMLNames.h"
#include "NodeWithIndex.h"
#include "ProcessingInstruction.h"
#include "RenderBlock.h"
#include "Text.h"
#include "TextIterator.h"
#include "markup.h"
#include "visible_units.h"
#include <stdio.h>

namespace WebCore {

using namespace std;
using namespace HTMLNames;

#ifndef NDEBUG
class RangeCounter {
public:
    static unsigned count;
    ~RangeCounter()
    {
        if (count)
            fprintf(stderr, "LEAK: %u Range\n", count);
    }
};
unsigned RangeCounter::count = 0;
static RangeCounter rangeCounter;
#endif

inline Range::Range(PassRefPtr<Document> ownerDocument)
    : m_ownerDocument(ownerDocument)
    , m_start(m_ownerDocument)
    , m_end(m_ownerDocument)
{
#ifndef NDEBUG
    ++RangeCounter::count;
#endif

    m_ownerDocument->attachRange(this);
}

PassRefPtr<Range> Range::create(PassRefPtr<Document> ownerDocument)
{
    return adoptRef(new Range(ownerDocument));
}

inline Range::Range(PassRefPtr<Document> ownerDocument, PassRefPtr<Node> startContainer, int startOffset, PassRefPtr<Node> endContainer, int endOffset)
    : m_ownerDocument(ownerDocument)
    , m_start(m_ownerDocument)
    , m_end(m_ownerDocument)
{
#ifndef NDEBUG
    ++RangeCounter::count;
#endif

    m_ownerDocument->attachRange(this);

    // Simply setting the containers and offsets directly would not do any of the checking
    // that setStart and setEnd do, so we call those functions.
    ExceptionCode ec = 0;
    setStart(startContainer, startOffset, ec);
    ASSERT(!ec);
    setEnd(endContainer, endOffset, ec);
    ASSERT(!ec);
}

PassRefPtr<Range> Range::create(PassRefPtr<Document> ownerDocument, PassRefPtr<Node> startContainer, int startOffset, PassRefPtr<Node> endContainer, int endOffset)
{
    return adoptRef(new Range(ownerDocument, startContainer, startOffset, endContainer, endOffset));
}

PassRefPtr<Range> Range::create(PassRefPtr<Document> ownerDocument, const Position& start, const Position& end)
{
    return adoptRef(new Range(ownerDocument, start.container.get(), start.posOffset, end.container.get(), end.posOffset));
}

Range::~Range()
{
    if (m_start.container())
        m_ownerDocument->detachRange(this);

#ifndef NDEBUG
    --RangeCounter::count;
#endif
}

Node* Range::startContainer(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return m_start.container();
}

int Range::startOffset(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return m_start.offset();
}

Node* Range::endContainer(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return m_end.container();
}

int Range::endOffset(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return m_end.offset();
}

Node* Range::commonAncestorContainer(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return commonAncestorContainer(m_start.container(), m_end.container());
}

Node* Range::commonAncestorContainer(Node* containerA, Node* containerB)
{
    for (Node* parentA = containerA; parentA; parentA = parentA->parentNode()) {
        for (Node* parentB = containerB; parentB; parentB = parentB->parentNode()) {
            if (parentA == parentB)
                return parentA;
        }
    }
    return 0;
}

bool Range::collapsed(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return m_start == m_end;
}

void Range::setStart(PassRefPtr<Node> refNode, int offset, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    ec = 0;
    Node* childNode = checkNodeWOffset(refNode.get(), offset, ec);
    if (ec)
        return;

    m_start.set(refNode, offset, childNode);

    // check if different root container
    Node* endRootContainer = m_end.container();
    while (endRootContainer->parentNode())
        endRootContainer = endRootContainer->parentNode();
    Node* startRootContainer = m_start.container();
    while (startRootContainer->parentNode())
        startRootContainer = startRootContainer->parentNode();
    if (startRootContainer != endRootContainer)
        collapse(true, ec);
    // check if new start after end
    else if (compareBoundaryPoints(m_start.container(), m_start.offset(), m_end.container(), m_end.offset()) > 0)
        collapse(true, ec);
}

void Range::setEnd(PassRefPtr<Node> refNode, int offset, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    ec = 0;
    Node* childNode = checkNodeWOffset(refNode.get(), offset, ec);
    if (ec)
        return;

    m_end.set(refNode, offset, childNode);

    // check if different root container
    Node* endRootContainer = m_end.container();
    while (endRootContainer->parentNode())
        endRootContainer = endRootContainer->parentNode();
    Node* startRootContainer = m_start.container();
    while (startRootContainer->parentNode())
        startRootContainer = startRootContainer->parentNode();
    if (startRootContainer != endRootContainer)
        collapse(false, ec);
    // check if new end before start
    if (compareBoundaryPoints(m_start.container(), m_start.offset(), m_end.container(), m_end.offset()) > 0)
        collapse(false, ec);
}

void Range::collapse(bool toStart, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (toStart)
        m_end = m_start;
    else
        m_start = m_end;
}

bool Range::isPointInRange(Node* refNode, int offset, ExceptionCode& ec)
{
    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return false;
    }

    if (!m_start.container() && refNode->attached()) {
        ec = INVALID_STATE_ERR;
        return false;
    }

    if (m_start.container() && !refNode->attached()) {
        // Firefox doesn't throw an exception for this case; it returns false.
        return false;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return false;
    }

    ec = 0;
    checkNodeWOffset(refNode, offset, ec);
    if (ec)
        return false;

    return compareBoundaryPoints(refNode, offset, m_start.container(), m_start.offset()) >= 0
        && compareBoundaryPoints(refNode, offset, m_end.container(), m_end.offset()) <= 0;
}

short Range::comparePoint(Node* refNode, int offset, ExceptionCode& ec)
{
    // http://developer.mozilla.org/en/docs/DOM:range.comparePoint
    // This method returns -1, 0 or 1 depending on if the point described by the 
    // refNode node and an offset within the node is before, same as, or after the range respectively.

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return 0;
    }

    if (!m_start.container() && refNode->attached()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    if (m_start.container() && !refNode->attached()) {
        // Firefox doesn't throw an exception for this case; it returns -1.
        return -1;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return 0;
    }

    ec = 0;
    checkNodeWOffset(refNode, offset, ec);
    if (ec)
        return 0;

    // compare to start, and point comes before
    if (compareBoundaryPoints(refNode, offset, m_start.container(), m_start.offset()) < 0)
        return -1;

    // compare to end, and point comes after
    if (compareBoundaryPoints(refNode, offset, m_end.container(), m_end.offset()) > 0)
        return 1;

    // point is in the middle of this range, or on the boundary points
    return 0;
}

Range::CompareResults Range::compareNode(Node* refNode, ExceptionCode& ec)
{
    // http://developer.mozilla.org/en/docs/DOM:range.compareNode
    // This method returns 0, 1, 2, or 3 based on if the node is before, after,
    // before and after(surrounds), or inside the range, respectively

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return NODE_BEFORE;
    }
    
    if (!m_start.container() && refNode->attached()) {
        ec = INVALID_STATE_ERR;
        return NODE_BEFORE;
    }

    if (m_start.container() && !refNode->attached()) {
        // Firefox doesn't throw an exception for this case; it returns 0.
        return NODE_BEFORE;
    }

    if (refNode->document() != m_ownerDocument) {
        // Firefox doesn't throw an exception for this case; it returns 0.
        return NODE_BEFORE;
    }

    Node* parentNode = refNode->parentNode();
    int nodeIndex = refNode->nodeIndex();
    
    if (!parentNode) {
        // if the node is the top document we should return NODE_BEFORE_AND_AFTER
        // but we throw to match firefox behavior
        ec = NOT_FOUND_ERR;
        return NODE_BEFORE;
    }

    if (comparePoint(parentNode, nodeIndex, ec) < 0) { // starts before
        if (comparePoint(parentNode, nodeIndex + 1, ec) > 0) // ends after the range
            return NODE_BEFORE_AND_AFTER;
        return NODE_BEFORE; // ends before or in the range
    } else { // starts at or after the range start
        if (comparePoint(parentNode, nodeIndex + 1, ec) > 0) // ends after the range
            return NODE_AFTER;
        return NODE_INSIDE; // ends inside the range
    }
}


short Range::compareBoundaryPoints(CompareHow how, const Range* sourceRange, ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    if (!sourceRange) {
        ec = NOT_FOUND_ERR;
        return 0;
    }

    ec = 0;
    Node* thisCont = commonAncestorContainer(ec);
    if (ec)
        return 0;
    Node* sourceCont = sourceRange->commonAncestorContainer(ec);
    if (ec)
        return 0;

    if (thisCont->document() != sourceCont->document()) {
        ec = WRONG_DOCUMENT_ERR;
        return 0;
    }

    Node* thisTop = thisCont;
    Node* sourceTop = sourceCont;
    while (thisTop->parentNode())
        thisTop = thisTop->parentNode();
    while (sourceTop->parentNode())
        sourceTop = sourceTop->parentNode();
    if (thisTop != sourceTop) { // in different DocumentFragments
        ec = WRONG_DOCUMENT_ERR;
        return 0;
    }

    switch (how) {
        case START_TO_START:
            return compareBoundaryPoints(m_start.container(), m_start.offset(),
                sourceRange->m_start.container(), sourceRange->m_start.offset());
        case START_TO_END:
            return compareBoundaryPoints(m_start.container(), m_start.offset(),
                sourceRange->m_end.container(), sourceRange->m_end.offset());
        case END_TO_END:
            return compareBoundaryPoints(m_end.container(), m_end.offset(),
                sourceRange->m_end.container(), sourceRange->m_end.offset());
        case END_TO_START:
            return compareBoundaryPoints(m_end.container(), m_end.offset(),
                sourceRange->m_start.container(), sourceRange->m_start.offset());
    }

    ec = SYNTAX_ERR;
    return 0;
}

short Range::compareBoundaryPoints(Node* containerA, int offsetA, Node* containerB, int offsetB)
{
    ASSERT(containerA && containerB);
    if (!containerA)
        return -1;
    if (!containerB)
        return 1;
    // see DOM2 traversal & range section 2.5

    // case 1: both points have the same container
    if (containerA == containerB) {
        if (offsetA == offsetB)
            return 0;           // A is equal to B
        if (offsetA < offsetB)
            return -1;          // A is before B
        else
            return 1;           // A is after B
    }

    // case 2: node C (container B or an ancestor) is a child node of A
    Node* c = containerB;
    while (c && c->parentNode() != containerA)
        c = c->parentNode();
    if (c) {
        int offsetC = 0;
        Node* n = containerA->firstChild();
        while (n != c && offsetC < offsetA) {
            offsetC++;
            n = n->nextSibling();
        }

        if (offsetA <= offsetC)
            return -1;              // A is before B
        else
            return 1;               // A is after B
    }

    // case 3: node C (container A or an ancestor) is a child node of B
    c = containerA;
    while (c && c->parentNode() != containerB)
        c = c->parentNode();
    if (c) {
        int offsetC = 0;
        Node* n = containerB->firstChild();
        while (n != c && offsetC < offsetB) {
            offsetC++;
            n = n->nextSibling();
        }

        if (offsetC < offsetB)
            return -1;              // A is before B
        else
            return 1;               // A is after B
    }

    // case 4: containers A & B are siblings, or children of siblings
    // ### we need to do a traversal here instead
    Node* commonAncestor = commonAncestorContainer(containerA, containerB);
    if (!commonAncestor)
        return 0;
    Node* childA = containerA;
    while (childA && childA->parentNode() != commonAncestor)
        childA = childA->parentNode();
    if (!childA)
        childA = commonAncestor;
    Node* childB = containerB;
    while (childB && childB->parentNode() != commonAncestor)
        childB = childB->parentNode();
    if (!childB)
        childB = commonAncestor;

    if (childA == childB)
        return 0; // A is equal to B

    Node* n = commonAncestor->firstChild();
    while (n) {
        if (n == childA)
            return -1; // A is before B
        if (n == childB)
            return 1; // A is after B
        n = n->nextSibling();
    }

    // Should never reach this point.
    ASSERT_NOT_REACHED();
    return 0;
}

short Range::compareBoundaryPoints(const Position& a, const Position& b)
{
    return compareBoundaryPoints(a.container.get(), a.posOffset, b.container.get(), b.posOffset);
}

bool Range::boundaryPointsValid() const
{
    return m_start.container() && compareBoundaryPoints(m_start.container(), m_start.offset(), m_end.container(), m_end.offset()) <= 0;
}

void Range::deleteContents(ExceptionCode& ec)
{
    checkDeleteExtract(ec);
    if (ec)
        return;

    processContents(DELETE_CONTENTS, ec);
}

bool Range::intersectsNode(Node* refNode, ExceptionCode& ec)
{
    // http://developer.mozilla.org/en/docs/DOM:range.intersectsNode
    // Returns a bool if the node intersects the range.

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return false;
    }
    
    if (!m_start.container() && refNode->attached()
            || m_start.container() && !refNode->attached()
            || refNode->document() != m_ownerDocument) {
        // Firefox doesn't throw an exception for these cases; it returns false.
        return false;
    }

    Node* parentNode = refNode->parentNode();
    int nodeIndex = refNode->nodeIndex();
    
    if (!parentNode) {
        // if the node is the top document we should return NODE_BEFORE_AND_AFTER
        // but we throw to match firefox behavior
        ec = NOT_FOUND_ERR;
        return false;
    }

    if (comparePoint(parentNode, nodeIndex, ec) < 0 && // starts before start
        comparePoint(parentNode, nodeIndex + 1, ec) < 0) { // ends before start
        return false;
    } else if (comparePoint(parentNode, nodeIndex, ec) > 0 && // starts after end
               comparePoint(parentNode, nodeIndex + 1, ec) > 0) { // ends after end
        return false;
    }
    
    return true; // all other cases
}

PassRefPtr<DocumentFragment> Range::processContents(ActionType action, ExceptionCode& ec)
{
    // FIXME: To work properly with mutation events, we will have to take into account
    // situations where the tree is being transformed while we work on it - ugh!

    RefPtr<DocumentFragment> fragment;
    if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS)
        fragment = new DocumentFragment(m_ownerDocument.get());
    
    ec = 0;
    if (collapsed(ec))
        return fragment.release();
    if (ec)
        return 0;

    Node* commonRoot = commonAncestorContainer(ec);
    if (ec)
        return 0;
    ASSERT(commonRoot);

    // what is the highest node that partially selects the start of the range?
    Node* partialStart = 0;
    if (m_start.container() != commonRoot) {
        partialStart = m_start.container();
        while (partialStart->parentNode() != commonRoot)
            partialStart = partialStart->parentNode();
    }

    // what is the highest node that partially selects the end of the range?
    Node* partialEnd = 0;
    if (m_end.container() != commonRoot) {
        partialEnd = m_end.container();
        while (partialEnd->parentNode() != commonRoot)
            partialEnd = partialEnd->parentNode();
    }

    // Simple case: the start and end containers are the same. We just grab
    // everything >= start offset and < end offset
    if (m_start.container() == m_end.container()) {
        Node::NodeType startNodeType = m_start.container()->nodeType();
        if (startNodeType == Node::TEXT_NODE || startNodeType == Node::CDATA_SECTION_NODE || startNodeType == Node::COMMENT_NODE) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<CharacterData> c = static_pointer_cast<CharacterData>(m_start.container()->cloneNode(true));
                c->deleteData(m_end.offset(), c->length() - m_end.offset(), ec);
                c->deleteData(0, m_start.offset(), ec);
                fragment->appendChild(c.release(), ec);
            }
            if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS)
                static_cast<CharacterData*>(m_start.container())->deleteData(m_start.offset(), m_end.offset() - m_start.offset(), ec);
        } else if (startNodeType == Node::PROCESSING_INSTRUCTION_NODE) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<ProcessingInstruction> c = static_pointer_cast<ProcessingInstruction>(m_start.container()->cloneNode(true));
                c->setData(c->data().substring(m_start.offset(), m_end.offset() - m_start.offset()), ec);
                fragment->appendChild(c.release(), ec);
            }
            if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS) {
                ProcessingInstruction* pi = static_cast<ProcessingInstruction*>(m_start.container());
                String data(pi->data());
                data.remove(m_start.offset(), m_end.offset() - m_start.offset());
                pi->setData(data, ec);
            }
        } else {
            Node* n = m_start.container()->firstChild();
            int i;
            for (i = 0; n && i < m_start.offset(); i++) // skip until start offset
                n = n->nextSibling();
            int endOffset = m_end.offset();
            while (n && i < endOffset) { // delete until end offset
                Node* next = n->nextSibling();
                if (action == EXTRACT_CONTENTS)
                    fragment->appendChild(n, ec); // will remove n from its parent
                else if (action == CLONE_CONTENTS)
                    fragment->appendChild(n->cloneNode(true), ec);
                else
                    m_start.container()->removeChild(n, ec);
                n = next;
                i++;
            }
        }
        return fragment.release();
    }

    // Complex case: Start and end containers are different.
    // There are three possiblities here:
    // 1. Start container == commonRoot (End container must be a descendant)
    // 2. End container == commonRoot (Start container must be a descendant)
    // 3. Neither is commonRoot, they are both descendants
    //
    // In case 3, we grab everything after the start (up until a direct child
    // of commonRoot) into leftContents, and everything before the end (up until
    // a direct child of commonRoot) into rightContents. Then we process all
    // commonRoot children between leftContents and rightContents
    //
    // In case 1 or 2, we skip either processing of leftContents or rightContents,
    // in which case the last lot of nodes either goes from the first or last
    // child of commonRoot.
    //
    // These are deleted, cloned, or extracted (i.e. both) depending on action.

    RefPtr<Node> leftContents;
    if (m_start.container() != commonRoot) {
        // process the left-hand side of the range, up until the last ancestor of
        // start container before commonRoot
        Node::NodeType startNodeType = m_start.container()->nodeType();
        if (startNodeType == Node::TEXT_NODE || startNodeType == Node::CDATA_SECTION_NODE || startNodeType == Node::COMMENT_NODE) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<CharacterData> c = static_pointer_cast<CharacterData>(m_start.container()->cloneNode(true));
                c->deleteData(0, m_start.offset(), ec);
                leftContents = c.release();
            }
            if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS)
                static_cast<CharacterData*>(m_start.container())->deleteData(
                    m_start.offset(), static_cast<CharacterData*>(m_start.container())->length() - m_start.offset(), ec);
        } else if (startNodeType == Node::PROCESSING_INSTRUCTION_NODE) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<ProcessingInstruction> c = static_pointer_cast<ProcessingInstruction>(m_start.container()->cloneNode(true));
                c->setData(c->data().substring(m_start.offset()), ec);
                leftContents = c.release();
            }
            if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS) {
                ProcessingInstruction* pi = static_cast<ProcessingInstruction*>(m_start.container());
                String data(pi->data());
                pi->setData(data.left(m_start.offset()), ec);
            }
        } else {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS)
                leftContents = m_start.container()->cloneNode(false);
            Node* n = m_start.container()->firstChild();
            for (int i = 0; n && i < m_start.offset(); i++) // skip until start offset
                n = n->nextSibling();
            while (n) { // process until end
                Node* next = n->nextSibling();
                if (action == EXTRACT_CONTENTS)
                    leftContents->appendChild(n, ec); // will remove n from start container
                else if (action == CLONE_CONTENTS)
                    leftContents->appendChild(n->cloneNode(true), ec);
                else
                    m_start.container()->removeChild(n, ec);
                n = next;
            }
        }

        Node* leftParent = m_start.container()->parentNode();
        Node* n = m_start.container()->nextSibling();
        for (; leftParent != commonRoot; leftParent = leftParent->parentNode()) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<Node> leftContentsParent = leftParent->cloneNode(false);
                leftContentsParent->appendChild(leftContents,ec);
                leftContents = leftContentsParent;
            }

            Node* next;
            for (; n; n = next) {
                next = n->nextSibling();
                if (action == EXTRACT_CONTENTS)
                    leftContents->appendChild(n,ec); // will remove n from leftParent
                else if (action == CLONE_CONTENTS)
                    leftContents->appendChild(n->cloneNode(true),ec);
                else
                    leftParent->removeChild(n,ec);
            }
            n = leftParent->nextSibling();
        }
    }

    RefPtr<Node> rightContents;
    if (m_end.container() != commonRoot) {
        // delete the right-hand side of the range, up until the last ancestor of
        // end container before commonRoot
        Node::NodeType endNodeType = m_end.container()->nodeType();
        if (endNodeType == Node::TEXT_NODE || endNodeType == Node::CDATA_SECTION_NODE || endNodeType == Node::COMMENT_NODE) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<CharacterData> c = static_pointer_cast<CharacterData>(m_end.container()->cloneNode(true));
                c->deleteData(m_end.offset(), static_cast<CharacterData*>(m_end.container())->length() - m_end.offset(), ec);
                rightContents = c;
            }
            if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS)
                static_cast<CharacterData*>(m_end.container())->deleteData(0, m_end.offset(), ec);
        } else if (endNodeType == Node::PROCESSING_INSTRUCTION_NODE) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<ProcessingInstruction> c = static_pointer_cast<ProcessingInstruction>(m_end.container()->cloneNode(true));
                c->setData(c->data().left(m_end.offset()), ec);
                rightContents = c.release();
            }
            if (action == EXTRACT_CONTENTS || action == DELETE_CONTENTS) {
                ProcessingInstruction* pi = static_cast<ProcessingInstruction*>(m_end.container());
                pi->setData(pi->data().substring(m_end.offset()), ec);
            }
        } else {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS)
                rightContents = m_end.container()->cloneNode(false);
            Node* n = m_end.container()->firstChild();
            if (n && m_end.offset()) {
                for (int i = 0; i + 1 < m_end.offset(); i++) { // skip to end.offset()
                    Node* next = n->nextSibling();
                    if (!next)
                        break;
                    n = next;
                }
                Node* prev;
                for (; n; n = prev) {
                    prev = n->previousSibling();
                    if (action == EXTRACT_CONTENTS)
                        rightContents->insertBefore(n, rightContents->firstChild(), ec); // will remove n from its parent
                    else if (action == CLONE_CONTENTS)
                        rightContents->insertBefore(n->cloneNode(true), rightContents->firstChild(), ec);
                    else
                        m_end.container()->removeChild(n, ec);
                }
            }
        }

        Node* rightParent = m_end.container()->parentNode();
        Node* n = m_end.container()->previousSibling();
        for (; rightParent != commonRoot; rightParent = rightParent->parentNode()) {
            if (action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) {
                RefPtr<Node> rightContentsParent = rightParent->cloneNode(false);
                rightContentsParent->appendChild(rightContents,ec);
                rightContents = rightContentsParent;
            }
            Node* prev;
            for (; n; n = prev) {
                prev = n->previousSibling();
                if (action == EXTRACT_CONTENTS)
                    rightContents->insertBefore(n, rightContents->firstChild(), ec); // will remove n from its parent
                else if (action == CLONE_CONTENTS)
                    rightContents->insertBefore(n->cloneNode(true), rightContents->firstChild(), ec);
                else
                    rightParent->removeChild(n, ec);
            }
            n = rightParent->previousSibling();
        }
    }

    // delete all children of commonRoot between the start and end container

    Node* processStart; // child of commonRoot
    if (m_start.container() == commonRoot) {
        processStart = m_start.container()->firstChild();
        for (int i = 0; i < m_start.offset(); i++)
            processStart = processStart->nextSibling();
    } else {
        processStart = m_start.container();
        while (processStart->parentNode() != commonRoot)
            processStart = processStart->parentNode();
        processStart = processStart->nextSibling();
    }
    Node* processEnd; // child of commonRoot
    if (m_end.container() == commonRoot) {
        processEnd = m_end.container()->firstChild();
        for (int i = 0; i < m_end.offset(); i++)
            processEnd = processEnd->nextSibling();
    } else {
        processEnd = m_end.container();
        while (processEnd->parentNode() != commonRoot)
            processEnd = processEnd->parentNode();
    }

    // Now add leftContents, stuff in between, and rightContents to the fragment
    // (or just delete the stuff in between)

    if ((action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) && leftContents)
        fragment->appendChild(leftContents, ec);

    Node* next;
    Node* n;
    if (processStart) {
        for (n = processStart; n && n != processEnd; n = next) {
            next = n->nextSibling();
            if (action == EXTRACT_CONTENTS)
                fragment->appendChild(n, ec); // will remove from commonRoot
            else if (action == CLONE_CONTENTS)
                fragment->appendChild(n->cloneNode(true), ec);
            else
                commonRoot->removeChild(n, ec);
        }
    }

    if ((action == EXTRACT_CONTENTS || action == CLONE_CONTENTS) && rightContents)
        fragment->appendChild(rightContents, ec);

    return fragment.release();
}

PassRefPtr<DocumentFragment> Range::extractContents(ExceptionCode& ec)
{
    checkDeleteExtract(ec);
    if (ec)
        return 0;

    return processContents(EXTRACT_CONTENTS, ec);
}

PassRefPtr<DocumentFragment> Range::cloneContents(ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return processContents(CLONE_CONTENTS, ec);
}

void Range::insertNode(PassRefPtr<Node> prpNewNode, ExceptionCode& ec)
{
    RefPtr<Node> newNode = prpNewNode;

    ec = 0;

    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!newNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if an ancestor container of either boundary-point of
    // the Range is read-only.
    if (containedByReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // WRONG_DOCUMENT_ERR: Raised if newParent and the container of the start of the Range were
    // not created from the same document.
    if (newNode->document() != m_start.container()->document()) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    // HIERARCHY_REQUEST_ERR: Raised if the container of the start of the Range is of a type that
    // does not allow children of the type of newNode or if newNode is an ancestor of the container.

    // an extra one here - if a text node is going to split, it must have a parent to insert into
    bool startIsText = m_start.container()->isTextNode();
    if (startIsText && !m_start.container()->parentNode()) {
        ec = HIERARCHY_REQUEST_ERR;
        return;
    }

    // In the case where the container is a text node, we check against the container's parent, because
    // text nodes get split up upon insertion.
    Node* checkAgainst;
    if (startIsText)
        checkAgainst = m_start.container()->parentNode();
    else
        checkAgainst = m_start.container();

    Node::NodeType newNodeType = newNode->nodeType();
    int numNewChildren;
    if (newNodeType == Node::DOCUMENT_FRAGMENT_NODE) {
        // check each child node, not the DocumentFragment itself
        numNewChildren = 0;
        for (Node* c = newNode->firstChild(); c; c = c->nextSibling()) {
            if (!checkAgainst->childTypeAllowed(c->nodeType())) {
                ec = HIERARCHY_REQUEST_ERR;
                return;
            }
            ++numNewChildren;
        }
    } else {
        numNewChildren = 1;
        if (!checkAgainst->childTypeAllowed(newNodeType)) {
            ec = HIERARCHY_REQUEST_ERR;
            return;
        }
    }

    for (Node* n = m_start.container(); n; n = n->parentNode()) {
        if (n == newNode) {
            ec = HIERARCHY_REQUEST_ERR;
            return;
        }
    }

    // INVALID_NODE_TYPE_ERR: Raised if newNode is an Attr, Entity, Notation, or Document node.
    if (newNodeType == Node::ATTRIBUTE_NODE || newNodeType == Node::ENTITY_NODE
            || newNodeType == Node::NOTATION_NODE || newNodeType == Node::DOCUMENT_NODE) {
        ec = RangeException::INVALID_NODE_TYPE_ERR;
        return;
    }

    bool collapsed = m_start == m_end;
    if (startIsText) {
        RefPtr<Text> newText = static_cast<Text*>(m_start.container())->splitText(m_start.offset(), ec);
        if (ec)
            return;
        m_start.container()->parentNode()->insertBefore(newNode.release(), newText.get(), ec);
        if (ec)
            return;

        // This special case doesn't seem to match the DOM specification, but it's currently required
        // to pass Acid3. We might later decide to remove this.
        if (collapsed)
            m_end.setToChild(newText.get());
    } else {
        RefPtr<Node> lastChild;
        if (collapsed)
            lastChild = (newNodeType == Node::DOCUMENT_FRAGMENT_NODE) ? newNode->lastChild() : newNode;

        int startOffset = m_start.offset();
        m_start.container()->insertBefore(newNode.release(), m_start.container()->childNode(startOffset), ec);
        if (ec)
            return;

        // This special case doesn't seem to match the DOM specification, but it's currently required
        // to pass Acid3. We might later decide to remove this.
        if (collapsed)
            m_end.set(m_start.container(), startOffset + numNewChildren, lastChild.get());
    }
}

String Range::toString(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return String();
    }

    Vector<UChar> result;

    Node* pastLast = pastLastNode();
    for (Node* n = firstNode(); n != pastLast; n = n->traverseNextNode()) {
        if (n->nodeType() == Node::TEXT_NODE || n->nodeType() == Node::CDATA_SECTION_NODE) {
            String data = static_cast<CharacterData*>(n)->data();
            int length = data.length();
            int start = (n == m_start.container()) ? min(max(0, m_start.offset()), length) : 0;
            int end = (n == m_end.container()) ? min(max(start, m_end.offset()), length) : length;
            result.append(data.characters() + start, end - start);
        }
    }

    return String::adopt(result);
}

String Range::toHTML() const
{
    return createMarkup(this);
}

String Range::text() const
{
    if (!m_start.container())
        return String();

    // We need to update layout, since plainText uses line boxes in the render tree.
    // FIXME: As with innerText, we'd like this to work even if there are no render objects.
    m_start.container()->document()->updateLayout();

    return plainText(this);
}

PassRefPtr<DocumentFragment> Range::createContextualFragment(const String& markup, ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    Node* element = m_start.container()->isElementNode() ? m_start.container() : m_start.container()->parentNode();
    if (!element || !element->isHTMLElement()) {
        ec = NOT_SUPPORTED_ERR;
        return 0;
    }

    RefPtr<DocumentFragment> fragment = static_cast<HTMLElement*>(element)->createContextualFragment(markup);
    if (!fragment) {
        ec = NOT_SUPPORTED_ERR;
        return 0;
    }

    return fragment.release();
}


void Range::detach(ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    m_ownerDocument->detachRange(this);

    m_start.clear();
    m_end.clear();
}

Node* Range::checkNodeWOffset(Node* n, int offset, ExceptionCode& ec) const
{
    switch (n->nodeType()) {
        case Node::DOCUMENT_TYPE_NODE:
        case Node::ENTITY_NODE:
        case Node::NOTATION_NODE:
            ec = RangeException::INVALID_NODE_TYPE_ERR;
            return 0;
        case Node::CDATA_SECTION_NODE:
        case Node::COMMENT_NODE:
        case Node::TEXT_NODE:
            if (static_cast<unsigned>(offset) > static_cast<CharacterData*>(n)->length())
                ec = INDEX_SIZE_ERR;
            return 0;
        case Node::PROCESSING_INSTRUCTION_NODE:
            if (static_cast<unsigned>(offset) > static_cast<ProcessingInstruction*>(n)->data().length())
                ec = INDEX_SIZE_ERR;
            return 0;
        case Node::ATTRIBUTE_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
        case Node::DOCUMENT_NODE:
        case Node::ELEMENT_NODE:
        case Node::ENTITY_REFERENCE_NODE:
        case Node::XPATH_NAMESPACE_NODE: {
            if (!offset)
                return 0;
            Node* childBefore = n->childNode(offset - 1);
            if (!childBefore)
                ec = INDEX_SIZE_ERR;
            return childBefore;
        }
    }
    ASSERT_NOT_REACHED();
    return 0;
}

void Range::checkNodeBA(Node* n, ExceptionCode& ec) const
{
    // INVALID_NODE_TYPE_ERR: Raised if the root container of refNode is not an
    // Attr, Document or DocumentFragment node or part of a shadow DOM tree
    // or if refNode is a Document, DocumentFragment, Attr, Entity, or Notation node.

    switch (n->nodeType()) {
        case Node::ATTRIBUTE_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
        case Node::DOCUMENT_NODE:
        case Node::ENTITY_NODE:
        case Node::NOTATION_NODE:
            ec = RangeException::INVALID_NODE_TYPE_ERR;
            return;
        case Node::CDATA_SECTION_NODE:
        case Node::COMMENT_NODE:
        case Node::DOCUMENT_TYPE_NODE:
        case Node::ELEMENT_NODE:
        case Node::ENTITY_REFERENCE_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::TEXT_NODE:
        case Node::XPATH_NAMESPACE_NODE:
            break;
    }

    Node* root = n;
    while (Node* parent = root->parentNode())
        root = parent;

    switch (root->nodeType()) {
        case Node::ATTRIBUTE_NODE:
        case Node::DOCUMENT_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
            break;
        case Node::CDATA_SECTION_NODE:
        case Node::COMMENT_NODE:
        case Node::DOCUMENT_TYPE_NODE:
        case Node::ELEMENT_NODE:
        case Node::ENTITY_NODE:
        case Node::ENTITY_REFERENCE_NODE:
        case Node::NOTATION_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::TEXT_NODE:
        case Node::XPATH_NAMESPACE_NODE:
            if (root->isShadowNode())
                break;
            ec = RangeException::INVALID_NODE_TYPE_ERR;
            return;
    }
}

PassRefPtr<Range> Range::cloneRange(ExceptionCode& ec) const
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return 0;
    }

    return Range::create(m_ownerDocument, m_start.container(), m_start.offset(), m_end.container(), m_end.offset());
}

void Range::setStartAfter(Node* refNode, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    ec = 0;
    checkNodeBA(refNode, ec);
    if (ec)
        return;

    setStart(refNode->parentNode(), refNode->nodeIndex() + 1, ec);
}

void Range::setEndBefore(Node* refNode, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    ec = 0;
    checkNodeBA(refNode, ec);
    if (ec)
        return;

    setEnd(refNode->parentNode(), refNode->nodeIndex(), ec);
}

void Range::setEndAfter(Node* refNode, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    ec = 0;
    checkNodeBA(refNode, ec);
    if (ec)
        return;

    setEnd(refNode->parentNode(), refNode->nodeIndex() + 1, ec);

}

void Range::selectNode(Node* refNode, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    // INVALID_NODE_TYPE_ERR: Raised if an ancestor of refNode is an Entity, Notation or
    // DocumentType node or if refNode is a Document, DocumentFragment, Attr, Entity, or Notation
    // node.
    for (Node* anc = refNode->parentNode(); anc; anc = anc->parentNode()) {
        switch (anc->nodeType()) {
            case Node::ATTRIBUTE_NODE:
            case Node::CDATA_SECTION_NODE:
            case Node::COMMENT_NODE:
            case Node::DOCUMENT_FRAGMENT_NODE:
            case Node::DOCUMENT_NODE:
            case Node::ELEMENT_NODE:
            case Node::ENTITY_REFERENCE_NODE:
            case Node::PROCESSING_INSTRUCTION_NODE:
            case Node::TEXT_NODE:
            case Node::XPATH_NAMESPACE_NODE:
                break;
            case Node::DOCUMENT_TYPE_NODE:
            case Node::ENTITY_NODE:
            case Node::NOTATION_NODE:
                ec = RangeException::INVALID_NODE_TYPE_ERR;
                return;
        }
    }

    switch (refNode->nodeType()) {
        case Node::CDATA_SECTION_NODE:
        case Node::COMMENT_NODE:
        case Node::DOCUMENT_TYPE_NODE:
        case Node::ELEMENT_NODE:
        case Node::ENTITY_REFERENCE_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::TEXT_NODE:
        case Node::XPATH_NAMESPACE_NODE:
            break;
        case Node::ATTRIBUTE_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
        case Node::DOCUMENT_NODE:
        case Node::ENTITY_NODE:
        case Node::NOTATION_NODE:
            ec = RangeException::INVALID_NODE_TYPE_ERR;
            return;
    }

    ec = 0;
    setStartBefore(refNode, ec);
    if (ec)
        return;
    setEndAfter(refNode, ec);
}

void Range::selectNodeContents(Node* refNode, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    // INVALID_NODE_TYPE_ERR: Raised if refNode or an ancestor of refNode is an Entity, Notation
    // or DocumentType node.
    for (Node* n = refNode; n; n = n->parentNode()) {
        switch (n->nodeType()) {
            case Node::ATTRIBUTE_NODE:
            case Node::CDATA_SECTION_NODE:
            case Node::COMMENT_NODE:
            case Node::DOCUMENT_FRAGMENT_NODE:
            case Node::DOCUMENT_NODE:
            case Node::ELEMENT_NODE:
            case Node::ENTITY_REFERENCE_NODE:
            case Node::PROCESSING_INSTRUCTION_NODE:
            case Node::TEXT_NODE:
            case Node::XPATH_NAMESPACE_NODE:
                break;
            case Node::DOCUMENT_TYPE_NODE:
            case Node::ENTITY_NODE:
            case Node::NOTATION_NODE:
                ec = RangeException::INVALID_NODE_TYPE_ERR;
                return;
        }
    }

    m_start.setToStart(refNode);
    m_end.setToEnd(refNode);
}

void Range::surroundContents(PassRefPtr<Node> passNewParent, ExceptionCode& ec)
{
    RefPtr<Node> newParent = passNewParent;

    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!newParent) {
        ec = NOT_FOUND_ERR;
        return;
    }

    // INVALID_NODE_TYPE_ERR: Raised if node is an Attr, Entity, DocumentType, Notation,
    // Document, or DocumentFragment node.
    switch (newParent->nodeType()) {
        case Node::ATTRIBUTE_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
        case Node::DOCUMENT_NODE:
        case Node::DOCUMENT_TYPE_NODE:
        case Node::ENTITY_NODE:
        case Node::NOTATION_NODE:
            ec = RangeException::INVALID_NODE_TYPE_ERR;
            return;
        case Node::CDATA_SECTION_NODE:
        case Node::COMMENT_NODE:
        case Node::ELEMENT_NODE:
        case Node::ENTITY_REFERENCE_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::TEXT_NODE:
        case Node::XPATH_NAMESPACE_NODE:
            break;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if an ancestor container of either boundary-point of
    // the Range is read-only.
    if (containedByReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // WRONG_DOCUMENT_ERR: Raised if newParent and the container of the start of the Range were
    // not created from the same document.
    if (newParent->document() != m_start.container()->document()) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    // Raise a HIERARCHY_REQUEST_ERR if m_start.container() doesn't accept children like newParent.
    Node* parentOfNewParent = m_start.container();

    // If m_start.container() is a character data node, it will be split and it will be its parent that will 
    // need to accept newParent (or in the case of a comment, it logically "would" be inserted into the parent,
    // although this will fail below for another reason).
    if (parentOfNewParent->isCharacterDataNode())
        parentOfNewParent = parentOfNewParent->parentNode();
    if (!parentOfNewParent->childTypeAllowed(newParent->nodeType())) {
        ec = HIERARCHY_REQUEST_ERR;
        return;
    }
    
    if (m_start.container() == newParent || m_start.container()->isDescendantOf(newParent.get())) {
        ec = HIERARCHY_REQUEST_ERR;
        return;
    }

    // FIXME: Do we need a check if the node would end up with a child node of a type not
    // allowed by the type of node?

    // BAD_BOUNDARYPOINTS_ERR: Raised if the Range partially selects a non-Text node.
    if (m_start.container()->nodeType() != Node::TEXT_NODE) {
        if (m_start.offset() > 0 && m_start.offset() < maxStartOffset()) {
            ec = RangeException::BAD_BOUNDARYPOINTS_ERR;
            return;
        }
    }
    if (m_end.container()->nodeType() != Node::TEXT_NODE) {
        if (m_end.offset() > 0 && m_end.offset() < maxEndOffset()) {
            ec = RangeException::BAD_BOUNDARYPOINTS_ERR;
            return;
        }
    }    

    ec = 0;
    while (Node* n = newParent->firstChild()) {
        newParent->removeChild(n, ec);
        if (ec)
            return;
    }
    RefPtr<DocumentFragment> fragment = extractContents(ec);
    if (ec)
        return;
    insertNode(newParent, ec);
    if (ec)
        return;
    newParent->appendChild(fragment.release(), ec);
    if (ec)
        return;
    selectNode(newParent.get(), ec);
}

void Range::setStartBefore(Node* refNode, ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    if (!refNode) {
        ec = NOT_FOUND_ERR;
        return;
    }

    if (refNode->document() != m_ownerDocument) {
        ec = WRONG_DOCUMENT_ERR;
        return;
    }

    ec = 0;
    checkNodeBA(refNode, ec);
    if (ec)
        return;

    setStart(refNode->parentNode(), refNode->nodeIndex(), ec);
}

void Range::checkDeleteExtract(ExceptionCode& ec)
{
    if (!m_start.container()) {
        ec = INVALID_STATE_ERR;
        return;
    }

    ec = 0;
    if (!commonAncestorContainer(ec) || ec)
        return;
        
    Node* pastLast = pastLastNode();
    for (Node* n = firstNode(); n != pastLast; n = n->traverseNextNode()) {
        if (n->isReadOnlyNode()) {
            ec = NO_MODIFICATION_ALLOWED_ERR;
            return;
        }
        if (n->nodeType() == Node::DOCUMENT_TYPE_NODE) {
            ec = HIERARCHY_REQUEST_ERR;
            return;
        }
    }

    if (containedByReadOnly()) {
        ec = NO_MODIFICATION_ALLOWED_ERR;
        return;
    }
}

bool Range::containedByReadOnly() const
{
    for (Node* n = m_start.container(); n; n = n->parentNode()) {
        if (n->isReadOnlyNode())
            return true;
    }
    for (Node* n = m_end.container(); n; n = n->parentNode()) {
        if (n->isReadOnlyNode())
            return true;
    }
    return false;
}

Node* Range::firstNode() const
{
    if (!m_start.container())
        return 0;
    if (m_start.container()->offsetInCharacters())
        return m_start.container();
    if (Node* child = m_start.container()->childNode(m_start.offset()))
        return child;
    if (!m_start.offset())
        return m_start.container();
    return m_start.container()->traverseNextSibling();
}

Position Range::editingStartPosition() const
{
    // This function is used by range style computations to avoid bugs like:
    // <rdar://problem/4017641> REGRESSION (Mail): you can only bold/unbold a selection starting from end of line once
    // It is important to skip certain irrelevant content at the start of the selection, so we do not wind up 
    // with a spurious "mixed" style.
    
    VisiblePosition visiblePosition(m_start.container(), m_start.offset(), VP_DEFAULT_AFFINITY);
    if (visiblePosition.isNull())
        return Position();

    ExceptionCode ec = 0;
    // if the selection is a caret, just return the position, since the style
    // behind us is relevant
    if (collapsed(ec))
        return visiblePosition.deepEquivalent();

    // if the selection starts just before a paragraph break, skip over it
    if (isEndOfParagraph(visiblePosition))
        return visiblePosition.next().deepEquivalent().downstream();

    // otherwise, make sure to be at the start of the first selected node,
    // instead of possibly at the end of the last node before the selection
    return visiblePosition.deepEquivalent().downstream();
}

Node* Range::shadowTreeRootNode() const
{
    return startContainer() ? startContainer()->shadowTreeRootNode() : 0;
}

Node* Range::pastLastNode() const
{
    if (!m_start.container() || !m_end.container())
        return 0;
    if (m_end.container()->offsetInCharacters())
        return m_end.container()->traverseNextSibling();
    if (Node* child = m_end.container()->childNode(m_end.offset()))
        return child;
    return m_end.container()->traverseNextSibling();
}

IntRect Range::boundingBox()
{
    IntRect result;
    Vector<IntRect> rects;
    addLineBoxRects(rects);
    const size_t n = rects.size();
    for (size_t i = 0; i < n; ++i)
        result.unite(rects[i]);
    return result;
}

void Range::addLineBoxRects(Vector<IntRect>& rects, bool useSelectionHeight)
{
    if (!m_start.container() || !m_end.container())
        return;

    RenderObject* start = m_start.container()->renderer();
    RenderObject* end = m_end.container()->renderer();
    if (!start || !end)
        return;

    RenderObject* stop = end->nextInPreOrderAfterChildren();
    for (RenderObject* r = start; r && r != stop; r = r->nextInPreOrder()) {
        // only ask leaf render objects for their line box rects
        if (!r->firstChild()) {
            int startOffset = r == start ? m_start.offset() : 0;
            int endOffset = r == end ? m_end.offset() : INT_MAX;
            r->addLineBoxRects(rects, startOffset, endOffset, useSelectionHeight);
        }
    }
}

#ifndef NDEBUG
#define FormatBufferSize 1024
void Range::formatForDebugger(char* buffer, unsigned length) const
{
    String result;
    String s;
    
    if (!m_start.container() || !m_end.container())
        result = "<empty>";
    else {
        char s[FormatBufferSize];
        result += "from offset ";
        result += String::number(m_start.offset());
        result += " of ";
        m_start.container()->formatForDebugger(s, FormatBufferSize);
        result += s;
        result += " to offset ";
        result += String::number(m_end.offset());
        result += " of ";
        m_end.container()->formatForDebugger(s, FormatBufferSize);
        result += s;
    }
          
    strncpy(buffer, result.utf8().data(), length - 1);
}
#undef FormatBufferSize
#endif

bool operator==(const Range& a, const Range& b)
{
    if (&a == &b)
        return true;
    // Not strictly legal C++, but in practice this can happen, and this check works
    // fine with GCC to detect such cases and return false rather than crashing.
    if (!&a || !&b)
        return false;
    return a.startPosition() == b.startPosition() && a.endPosition() == b.endPosition();
}

PassRefPtr<Range> rangeOfContents(Node* node)
{
    ASSERT(node);
    RefPtr<Range> range = Range::create(node->document());
    int exception = 0;
    range->selectNodeContents(node, exception);
    return range.release();
}

int Range::maxStartOffset() const
{
    if (!m_start.container())
        return 0;
    if (!m_start.container()->offsetInCharacters())
        return m_start.container()->childNodeCount();
    return m_start.container()->maxCharacterOffset();
}

int Range::maxEndOffset() const
{
    if (!m_end.container())
        return 0;
    if (!m_end.container()->offsetInCharacters())
        return m_end.container()->childNodeCount();
    return m_end.container()->maxCharacterOffset();
}

static inline void boundaryNodeChildrenChanged(RangeBoundaryPoint& boundary, ContainerNode* container)
{
    if (!boundary.childBefore())
        return;
    if (boundary.container() != container)
        return;
    boundary.invalidateOffset();
}

void Range::nodeChildrenChanged(ContainerNode* container)
{
    ASSERT(container);
    ASSERT(container->document() == m_ownerDocument);
    boundaryNodeChildrenChanged(m_start, container);
    boundaryNodeChildrenChanged(m_end, container);
}

static inline void boundaryNodeWillBeRemoved(RangeBoundaryPoint& boundary, Node* nodeToBeRemoved)
{
    if (boundary.childBefore() == nodeToBeRemoved) {
        boundary.childBeforeWillBeRemoved();
        return;
    }

    for (Node* n = boundary.container(); n; n = n->parentNode()) {
        if (n == nodeToBeRemoved) {
            boundary.setToChild(nodeToBeRemoved);
            return;
        }
    }
}

void Range::nodeWillBeRemoved(Node* node)
{
    ASSERT(node);
    ASSERT(node->document() == m_ownerDocument);
    ASSERT(node != m_ownerDocument);
    ASSERT(node->parentNode());
    boundaryNodeWillBeRemoved(m_start, node);
    boundaryNodeWillBeRemoved(m_end, node);
}

static inline void boundaryTextInserted(RangeBoundaryPoint& boundary, Node* text, unsigned offset, unsigned length)
{
    if (boundary.container() != text)
        return;
    unsigned boundaryOffset = boundary.offset();
    if (offset >= boundaryOffset)
        return;
    boundary.setOffset(boundaryOffset + length);
}

void Range::textInserted(Node* text, unsigned offset, unsigned length)
{
    ASSERT(text);
    ASSERT(text->document() == m_ownerDocument);
    boundaryTextInserted(m_start, text, offset, length);
    boundaryTextInserted(m_end, text, offset, length);
}

static inline void boundaryTextRemoved(RangeBoundaryPoint& boundary, Node* text, unsigned offset, unsigned length)
{
    if (boundary.container() != text)
        return;
    unsigned boundaryOffset = boundary.offset();
    if (offset >= boundaryOffset)
        return;
    if (offset + length >= boundaryOffset)
        boundary.setOffset(offset);
    else
        boundary.setOffset(boundaryOffset - length);
}

void Range::textRemoved(Node* text, unsigned offset, unsigned length)
{
    ASSERT(text);
    ASSERT(text->document() == m_ownerDocument);
    boundaryTextRemoved(m_start, text, offset, length);
    boundaryTextRemoved(m_end, text, offset, length);
}

static inline void boundaryTextNodesMerged(RangeBoundaryPoint& boundary, NodeWithIndex& oldNode, unsigned offset)
{
    if (boundary.container() == oldNode.node())
        boundary.set(oldNode.node()->previousSibling(), boundary.offset() + offset, 0);
    else if (boundary.container() == oldNode.node()->parentNode() && boundary.offset() == oldNode.index())
        boundary.set(oldNode.node()->previousSibling(), offset, 0);
}

void Range::textNodesMerged(NodeWithIndex& oldNode, unsigned offset)
{
    ASSERT(oldNode.node());
    ASSERT(oldNode.node()->document() == m_ownerDocument);
    ASSERT(oldNode.node()->parentNode());
    ASSERT(oldNode.node()->isTextNode());
    ASSERT(oldNode.node()->previousSibling());
    ASSERT(oldNode.node()->previousSibling()->isTextNode());
    boundaryTextNodesMerged(m_start, oldNode, offset);
    boundaryTextNodesMerged(m_end, oldNode, offset);
}

static inline void boundaryTextNodesSplit(RangeBoundaryPoint& boundary, Text* oldNode)
{
    if (boundary.container() != oldNode)
        return;
    unsigned boundaryOffset = boundary.offset();
    if (boundaryOffset <= oldNode->length())
        return;
    boundary.set(oldNode->nextSibling(), boundaryOffset - oldNode->length(), 0);
}

void Range::textNodeSplit(Text* oldNode)
{
    ASSERT(oldNode);
    ASSERT(oldNode->document() == m_ownerDocument);
    ASSERT(oldNode->parentNode());
    ASSERT(oldNode->isTextNode());
    ASSERT(oldNode->nextSibling());
    ASSERT(oldNode->nextSibling()->isTextNode());
    boundaryTextNodesSplit(m_start, oldNode);
    boundaryTextNodesSplit(m_end, oldNode);
}

}
