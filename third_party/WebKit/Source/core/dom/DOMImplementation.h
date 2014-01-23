/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2008 Apple Inc. All rights reserved.
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
 *
 */

#ifndef DOMImplementation_h
#define DOMImplementation_h

#include "core/dom/Document.h"
#include "wtf/Forward.h"
#include "wtf/PassRefPtr.h"

namespace WebCore {

class CSSStyleSheet;
class Document;
class DocumentInit;
class DocumentType;
class ExceptionState;
class Frame;
class HTMLDocument;
class KURL;
class XMLDocument;

class DOMImplementation : public ScriptWrappable {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static PassOwnPtr<DOMImplementation> create(Document& document) { return adoptPtr(new DOMImplementation(document)); }

    void ref() { m_document.ref(); }
    void deref() { m_document.deref(); }
    Document* document() { return &m_document; }

    // DOM methods & attributes for DOMImplementation
    static bool hasFeature(const String& feature, const String& version);
    bool hasFeatureForBindings(const String& feature, const String& version);
    PassRefPtr<DocumentType> createDocumentType(const AtomicString& qualifiedName, const String& publicId, const String& systemId, ExceptionState&);
    PassRefPtr<XMLDocument> createDocument(const AtomicString& namespaceURI, const AtomicString& qualifiedName, DocumentType*, ExceptionState&);

    DOMImplementation* getInterface(const String& feature);

    // From the DOMImplementationCSS interface
    static PassRefPtr<CSSStyleSheet> createCSSStyleSheet(const String& title, const String& media);

    // From the HTMLDOMImplementation interface
    PassRefPtr<HTMLDocument> createHTMLDocument(const String& title);

    // Other methods (not part of DOM)
    static PassRefPtr<Document> createDocument(const String& mimeType, Frame*, const KURL&, bool inViewSourceMode);
    static PassRefPtr<Document> createDocument(const String& mimeType, const DocumentInit&, bool inViewSourceMode);

    static bool isXMLMIMEType(const String&);
    static bool isTextMIMEType(const String&);
    static bool isJSONMIMEType(const String&);

private:
    explicit DOMImplementation(Document&);

    Document& m_document;
};

} // namespace WebCore

#endif
