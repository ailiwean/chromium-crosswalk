/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
#include "SinkDocument.h"

#include "DocumentParser.h"

namespace WebCore {

class SinkDocumentParser : public DocumentParser {
public:
    SinkDocumentParser(Document* document) : m_document(document) { }
        
private:
    virtual void write(const SegmentedString&, bool) { ASSERT_NOT_REACHED(); }
    virtual void finish();
    virtual bool isWaitingForScripts() const { return false; }
        
    virtual bool wantsRawData() const { return true; }
    virtual bool writeRawData(const char*, int) { return false; }

    Document* m_document;
};

void SinkDocumentParser::finish()
{
    if (!m_parserStopped) 
        m_document->finishedParsing();    
}
    
SinkDocument::SinkDocument(Frame* frame)
    : HTMLDocument(frame)
{
    setParseMode(Compat);
}
    
DocumentParser* SinkDocument::createTokenizer()
{
    return new SinkDocumentParser(this);
}

} // namespace WebCore
