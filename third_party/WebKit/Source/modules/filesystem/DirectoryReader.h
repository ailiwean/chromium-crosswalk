/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DirectoryReader_h
#define DirectoryReader_h

#include "bindings/v8/ScriptWrappable.h"
#include "modules/filesystem/DOMFileSystem.h"
#include "modules/filesystem/DirectoryReaderBase.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefCounted.h"
#include "wtf/text/WTFString.h"

namespace WebCore {

class EntriesCallback;
class EntriesCallbacks;
class ErrorCallback;

class DirectoryReader : public DirectoryReaderBase, public ScriptWrappable {
public:
    static PassRefPtr<DirectoryReader> create(PassRefPtr<DOMFileSystemBase> fileSystem, const String& fullPath)
    {
        return adoptRef(new DirectoryReader(fileSystem, fullPath));
    }

    void readEntries(PassOwnPtr<EntriesCallback>, PassOwnPtr<ErrorCallback> = nullptr);

    DOMFileSystem* filesystem() const { return static_cast<DOMFileSystem*>(m_fileSystem.get()); }

private:
    DirectoryReader(PassRefPtr<DOMFileSystemBase>, const String& fullPath);
};

}

#endif // DirectoryReader_h
