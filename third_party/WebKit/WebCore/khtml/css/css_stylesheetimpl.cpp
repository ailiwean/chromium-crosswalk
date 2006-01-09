/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004 Apple Computer, Inc.
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
 * $Id: css_stylesheetimpl.cpp 11751 2005-12-23 18:44:39Z eseidel $
 */

//#define CSS_STYLESHEET_DEBUG

#include "config.h"
#include "dom/dom_string.h"
#include "dom/dom_exception.h"
#include "dom/css_stylesheet.h"
#include "dom/css_rule.h"

#include "css/css_ruleimpl.h"
#include "css/css_valueimpl.h"
#include "css/cssparser.h"
#include "css/css_stylesheetimpl.h"

#include "xml/dom_nodeimpl.h"
#include "html/html_documentimpl.h"
#include "misc/loader.h"

#include <kdebug.h>

using namespace DOM;
using namespace khtml;
// --------------------------------------------------------------------------------

StyleSheetImpl::StyleSheetImpl(StyleSheetImpl *parentSheet, DOMString href)
    : StyleListImpl(parentSheet)
{
    m_disabled = false;
    m_parentNode = 0;
    m_strHref = href;
}


StyleSheetImpl::StyleSheetImpl(DOM::NodeImpl *parentNode, DOMString href)
    : StyleListImpl()
{
    m_parentNode = parentNode;
    m_disabled = false;
    m_strHref = href;
}

StyleSheetImpl::StyleSheetImpl(StyleBaseImpl *owner, DOMString href)
    : StyleListImpl(owner)
{
    m_disabled = false;
    m_parentNode = 0;
    m_strHref = href;
}

StyleSheetImpl::~StyleSheetImpl()
{
    if (m_media)
        m_media->setParent(0);
}

StyleSheetImpl *StyleSheetImpl::parentStyleSheet() const
{
    return (parent() && parent()->isStyleSheet()) ? static_cast<StyleSheetImpl *>(parent()) : 0;
}

void StyleSheetImpl::setMedia(MediaListImpl *media)
{
    m_media = media;
}

// -----------------------------------------------------------------------


CSSStyleSheetImpl::CSSStyleSheetImpl(CSSStyleSheetImpl *parentSheet, DOMString href)
    : StyleSheetImpl(parentSheet, href)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    m_doc = parentSheet ? parentSheet->doc() : 0;
    m_implicit = false;
    m_namespaces = 0;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(NodeImpl *parentNode, DOMString href, bool _implicit)
    : StyleSheetImpl(parentNode, href)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    m_doc = parentNode->getDocument();
    m_implicit = _implicit; 
    m_namespaces = 0;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(CSSRuleImpl *ownerRule, DOMString href)
    : StyleSheetImpl(ownerRule, href)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    m_doc = 0;
    m_implicit = false;
    m_namespaces = 0;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(DOM::NodeImpl *parentNode, CSSStyleSheetImpl *orig)
    : StyleSheetImpl(parentNode, orig->m_strHref)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    StyleBaseImpl *rule;
    for ( rule = orig->m_lstChildren->first(); rule != 0; rule = orig->m_lstChildren->next() )
    {
        m_lstChildren->append(rule);
        rule->setParent(this);
    }
    m_doc = parentNode->getDocument();
    m_implicit = false;
    m_namespaces = 0;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(CSSRuleImpl *ownerRule, CSSStyleSheetImpl *orig)
    : StyleSheetImpl(ownerRule, orig->m_strHref)
{
    // m_lstChildren is deleted in StyleListImpl
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    StyleBaseImpl *rule;
    for ( rule = orig->m_lstChildren->first(); rule != 0; rule = orig->m_lstChildren->next() )
    {
        m_lstChildren->append(rule);
        rule->setParent(this);
    }
    m_doc  = 0;
    m_implicit = false;
    m_namespaces = 0;
}

CSSRuleImpl *CSSStyleSheetImpl::ownerRule() const
{
    return (parent() && parent()->isRule()) ? static_cast<CSSRuleImpl *>(parent()) : 0;
}

unsigned CSSStyleSheetImpl::insertRule( const DOMString &rule, unsigned index, int &exceptioncode )
{
    exceptioncode = 0;
    if (index > m_lstChildren->count()) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }
    CSSParser p( strictParsing );
    CSSRuleImpl *r = p.parseRule( this, rule );

    if (!r) {
        exceptioncode = CSSException::SYNTAX_ERR + CSSException::_EXCEPTION_OFFSET;
        return 0;
    }
    // ###
    // HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at the specified index e.g. if an
    //@import rule is inserted after a standard rule set or other at-rule.
    m_lstChildren->insert(index, r);
    return index;
}

unsigned CSSStyleSheetImpl::addRule( const DOMString &selector, const DOMString &style, int index, int &exceptioncode )
{
    if (index == -1)
        index = m_lstChildren->count();
    return insertRule(selector + " { " + style + " }", index, exceptioncode);
}

CSSRuleListImpl *CSSStyleSheetImpl::cssRules()
{
    return new CSSRuleListImpl(this);
}

void CSSStyleSheetImpl::deleteRule( unsigned index, int &exceptioncode )
{
    exceptioncode = 0;
    StyleBaseImpl *b = m_lstChildren->take(index);
    if (!b) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return;
    }
    b->deref();
}

void CSSStyleSheetImpl::addNamespace(CSSParser* p, const AtomicString& prefix, const AtomicString& uri)
{
    if (uri.isEmpty())
        return;

    m_namespaces = new CSSNamespace(prefix, uri, m_namespaces);
    
    if (prefix.isEmpty())
        // Set the default namespace on the parser so that selectors that omit namespace info will
        // be able to pick it up easily.
        p->defaultNamespace = uri;
}

const AtomicString& CSSStyleSheetImpl::determineNamespace(const AtomicString& prefix)
{
    if (prefix.isEmpty())
        return nullAtom; // No namespace. If an element/attribute has a namespace, we won't match it.
    else if (prefix == starAtom)
        return starAtom; // We'll match any namespace.
    else if (m_namespaces) {
        CSSNamespace* ns = m_namespaces->namespaceForPrefix(prefix);
        if (ns)
            return ns->uri();
    }
    return nullAtom; // Assume we wont match any namespaces.
}

bool CSSStyleSheetImpl::parseString(const DOMString &string, bool strict)
{
    strictParsing = strict;
    CSSParser p( strict );
    p.parseSheet( this, string );
    return true;
}

bool CSSStyleSheetImpl::isLoading()
{
    StyleBaseImpl *rule;
    for ( rule = m_lstChildren->first(); rule != 0; rule = m_lstChildren->next() )
    {
        if (rule->isImportRule())
        {
            CSSImportRuleImpl *import = static_cast<CSSImportRuleImpl *>(rule);
            if (import->isLoading())
            {
                return true;
            }
        }
    }
    return false;
}

void CSSStyleSheetImpl::checkLoaded()
{
    if (isLoading()) return;
    if (parent()) parent()->checkLoaded();
    if (m_parentNode) m_parentNode->sheetLoaded();
}

khtml::DocLoader *CSSStyleSheetImpl::docLoader()
{
    if ( !m_doc ) // doc is 0 for the user- and default-sheet!
        return 0;

    // ### remove? (clients just use sheet->doc()->docLoader())
    return m_doc->docLoader();
}

// ---------------------------------------------------------------------------


StyleSheetListImpl::~StyleSheetListImpl()
{
    for ( QPtrListIterator<StyleSheetImpl> it ( styleSheets ); it.current(); ++it )
        it.current()->deref();
}

void StyleSheetListImpl::add( StyleSheetImpl* s )
{
    if ( !styleSheets.containsRef( s ) ) {
        s->ref();
        styleSheets.append( s );
    }
}

void StyleSheetListImpl::remove( StyleSheetImpl* s )
{
    if ( styleSheets.removeRef( s ) )
        s->deref();
}

unsigned StyleSheetListImpl::length() const
{
    // hack so implicit BODY stylesheets don't get counted here
    unsigned l = 0;
    QPtrListIterator<StyleSheetImpl> it(styleSheets);
    for (; it.current(); ++it) {
        if (!it.current()->isCSSStyleSheet() || !static_cast<CSSStyleSheetImpl*>(it.current())->implicit())
            l++;
    }
    return l;
}

StyleSheetImpl *StyleSheetListImpl::item ( unsigned index )
{
    unsigned l = 0;
    QPtrListIterator<StyleSheetImpl> it(styleSheets);
    for (; it.current(); ++it) {
        if (!it.current()->isCSSStyleSheet() || !static_cast<CSSStyleSheetImpl*>(it.current())->implicit()) {
            if (l == index)
                return it.current();
            l++;
        }
    }
    return 0;
}

// --------------------------------------------------------------------------------------------

MediaListImpl::MediaListImpl( CSSStyleSheetImpl *parentSheet,
                              const DOMString &media )
    : StyleBaseImpl( parentSheet )
{
    setMediaText( media );
}

MediaListImpl::MediaListImpl( CSSRuleImpl *parentRule, const DOMString &media )
    : StyleBaseImpl(parentRule)
{
    setMediaText( media );
}

bool MediaListImpl::contains( const DOMString &medium ) const
{
    return m_lstMedia.count() == 0 || m_lstMedia.contains( medium ) ||
            m_lstMedia.contains( "all" );
}

CSSStyleSheetImpl *MediaListImpl::parentStyleSheet() const
{
    return parent()->isCSSStyleSheet() ? static_cast<CSSStyleSheetImpl *>(parent()) : 0;
}

CSSRuleImpl *MediaListImpl::parentRule() const
{
    return parent()->isRule() ? static_cast<CSSRuleImpl *>(parent()) : 0;
}

void MediaListImpl::deleteMedium( const DOMString &oldMedium )
{
    for ( QValueList<DOMString>::Iterator it = m_lstMedia.begin(); it != m_lstMedia.end(); ++it ) {
        if ((*it) == oldMedium) {
            m_lstMedia.remove( it );
            return;
        }
    }
}

DOM::DOMString MediaListImpl::mediaText() const
{
    DOMString text = "";
    for (QValueList<DOMString>::ConstIterator it = m_lstMedia.begin(); it != m_lstMedia.end(); ++it) {
        if (text.length() > 0)
            text += ", ";
        text += *it;
    }
    return text;
}

void MediaListImpl::setMediaText(const DOM::DOMString &value)
{
    m_lstMedia.clear();
    QStringList list = QStringList::split( ',', value.qstring() );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        DOMString medium = (*it).stripWhiteSpace();
        if (!medium.isEmpty())
            m_lstMedia.append( medium );
    }
}
