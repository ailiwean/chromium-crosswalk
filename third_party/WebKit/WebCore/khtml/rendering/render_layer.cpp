/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2002 (hyatt@apple.com)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "render_layer.h"
#include <kdebug.h>
#include <assert.h>
#include "khtmlview.h"
#include "render_object.h"

using namespace DOM;
using namespace khtml;

RenderLayer::RenderLayer(RenderObject* object)
: m_object( object ),
m_parent( 0 ),
m_previous( 0 ),
m_next( 0 ),
m_first( 0 ),
m_last( 0 ),
m_height( 0 ),
m_y( 0 ),
m_x( 0 ),
m_width( 0 )
{
}

RenderLayer::~RenderLayer()
{
    // Child layers will be deleted by their corresponding render objects, so
    // our destructor doesn't have to do anything.
}

void RenderLayer::addChild(RenderLayer *child, RenderLayer *beforeChild)
{
    if (!beforeChild)
        setLastChild(child);
        
    if(beforeChild == firstChild()) // Handles the null case too.
        setFirstChild(child);

    RenderLayer* prev = beforeChild ? beforeChild->previousSibling() : 0;
    child->setNextSibling(beforeChild);
    if (beforeChild) beforeChild->setPreviousSibling(child);
    if(prev) prev->setNextSibling(child);
    child->setPreviousSibling(prev);
    child->setParent(this);
}

RenderLayer* RenderLayer::removeChild(RenderLayer* oldChild)
{
    // remove the child
    if (oldChild->previousSibling())
        oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
        oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
        m_first = oldChild->nextSibling();
    if (m_last == oldChild)
        m_last = oldChild->previousSibling();

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);
    
    return oldChild;
}

void 
RenderLayer::convertToLayerCoords(RenderLayer* ancestorLayer, int& x, int& y)
{
    x = xPos();
    y = yPos();
    if (ancestorLayer == this) return;
    
    for (RenderLayer* current = parent(); current && current != ancestorLayer;
         current = current->parent()) {
        x += current->xPos();
        y += current->yPos();
    }
}

RenderLayer::RenderZTreeNode*
RenderLayer::constructZTree(const QRect& damageRect, 
                            RenderLayer* rootLayer, 
                            RenderLayer* paintingLayer)
{
    // This variable stores the result we will hand back.
    RenderLayer::RenderZTreeNode* returnNode = 0;
    
    // If a layer isn't visible, then none of its child layers are visible either.
    // Don't build this branch of the z-tree, since these layers should not be painted.
    if (renderer()->style()->visibility() != VISIBLE)
        return 0;
    
    // Compute this layer's absolute position, so that we can compare it with our
    // damage rect and avoid repainting the layer if it falls outside that rect.
    int x, y;
    convertToLayerCoords(rootLayer, x, y);
    QRect layerBounds(x, y, width(), height());
    if (!layerBounds.intersects(damageRect))
        return 0;
    
    // Compute our coordinates relative to the layer being painted.
    convertToLayerCoords(paintingLayer, x, y);

    returnNode = new RenderLayer::RenderZTreeNode(this);
    
    // Walk our list of child layers looking only for those layers that have a 
    // non-negative z-index (a z-index >= 0).
    for (RenderLayer* child = firstChild(); child; child = child->nextSibling()) {
        if (child->zIndex() < 0)
            continue; // Ignore negative z-indices in this first pass.

        RenderZTreeNode* childNode = child->constructZTree(damageRect, rootLayer, paintingLayer);
        if (childNode) {
            // Put the new node into the tree at the front of the parent's list.
            childNode->next = returnNode->child;
            returnNode->child = childNode;
        }
    }

    // Now add a leaf node for ourselves.
    RenderLayerElement* layerElt = new RenderLayerElement(this, layerBounds, x, y);
    if (returnNode->child) {
        RenderZTreeNode* leaf = new RenderZTreeNode(layerElt);
        leaf->next = returnNode->child;
        returnNode->child = leaf;
    }
    else
        returnNode->layerElement = layerElt;

    // Now look for children that have a negative z-index.
    for (RenderLayer* child = firstChild(); child; child = child->nextSibling()) {
        if (child->zIndex() >= 0)
            continue; // Ignore non-negative z-indices in this second pass.

        RenderZTreeNode* childNode = child->constructZTree(damageRect, rootLayer, paintingLayer);
        if (childNode) {
            // Deal with the case where all our children views had negative z-indices.
            // Demote our leaf node and make a new interior node that can hold these
            // children.
            if (returnNode->layerElement) {
                RenderZTreeNode* leaf = returnNode;
                returnNode = new RenderLayer::RenderZTreeNode(this);
                returnNode->child = leaf;
            }
            
            // Put the new node into the tree at the front of the parent's list.
            childNode->next = returnNode->child;
            returnNode->child = childNode;
        }
    }
    
    return returnNode;
}

void
RenderLayer::constructLayerList(RenderZTreeNode* ztree, QPtrVector<RenderLayer::RenderLayerElement>* result)
{
    // This merge buffer is just a temporary used during computation as we do merge sorting.
    QPtrVector<RenderLayer::RenderLayerElement> mergeBuffer;
    ztree->constructLayerList(&mergeBuffer, result);
}

