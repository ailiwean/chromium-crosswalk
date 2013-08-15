/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "config.h"
#include "LocalFileSystemClient.h"

#include "WebFileSystemCallbacksImpl.h"
#include "WebFrameImpl.h"
#include "WebViewImpl.h"
#include "core/dom/Document.h"
#include "core/platform/AsyncFileSystemCallbacks.h"
#include "public/platform/Platform.h"
#include "public/platform/WebFileError.h"
#include "public/platform/WebFileSystem.h"
#include "public/platform/WebFileSystemCallbacks.h"
#include "public/platform/WebFileSystemType.h"
#include "public/web/WebPermissionClient.h"
#include "weborigin/SecurityOrigin.h"
#include "wtf/text/WTFString.h"

using namespace WebCore;

namespace WebKit {

PassOwnPtr<FileSystemClient> LocalFileSystemClient::create()
{
    return adoptPtr(static_cast<FileSystemClient*>(new LocalFileSystemClient()));
}

LocalFileSystemClient::~LocalFileSystemClient()
{
}

bool LocalFileSystemClient::allowFileSystem(ScriptExecutionContext* context)
{
    Document* document = toDocument(context);
    WebFrameImpl* webFrame = WebFrameImpl::fromFrame(document->frame());
    WebKit::WebViewImpl* webView = webFrame->viewImpl();

    return !webView->permissionClient() || webView->permissionClient()->allowFileSystem(webFrame);
}

void LocalFileSystemClient::openFileSystem(ScriptExecutionContext* context, WebCore::FileSystemType type, PassOwnPtr<AsyncFileSystemCallbacks> callbacks, long long size, OpenFileSystemMode openMode)
{
    KURL storagePartition = KURL(KURL(), context->securityOrigin()->toString());
    // FIXME: fix this callbacks raw pointer.
    WebKit::Platform::current()->fileSystem()->openFileSystem(storagePartition, static_cast<WebFileSystemType>(type), openMode == CreateFileSystemIfNotPresent, new WebKit::WebFileSystemCallbacksImpl(callbacks));
}

void LocalFileSystemClient::deleteFileSystem(ScriptExecutionContext* context, WebCore::FileSystemType type, PassOwnPtr<AsyncFileSystemCallbacks> callbacks)
{
    KURL storagePartition = KURL(KURL(), context->securityOrigin()->toString());
    // FIXME: fix this callbacks raw pointer.
    WebKit::Platform::current()->fileSystem()->deleteFileSystem(storagePartition, static_cast<WebFileSystemType>(type), new WebFileSystemCallbacksImpl(callbacks));

}

LocalFileSystemClient::LocalFileSystemClient()
{
}

} // namespace WebKit
