/*
 * Copyright (C) 2010 ProFUSION embedded systems
 * Copyright (C) 2010 Samsung Electronics
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

#ifndef ContextMenuClientEfl_h
#define ContextMenuClientEfl_h

#include "ContextMenuClient.h"
#include "EWebKit.h"

namespace WebCore {

class ContextMenu;

class ContextMenuClientEfl : public ContextMenuClient {
 public:
    explicit ContextMenuClientEfl(Evas_Object*);

    virtual void contextMenuDestroyed();

    virtual PlatformMenuDescription getCustomMenuFromDefaultItems(ContextMenu*);
    virtual void contextMenuItemSelected(ContextMenuItem*, const ContextMenu*);

    virtual void downloadURL(const KURL&);
    virtual void searchWithGoogle(const Frame*);
    virtual void lookUpInDictionary(Frame*);
    virtual void speak(const String&);
    virtual bool isSpeaking();
    virtual void stopSpeaking();

    PlatformMenuDescription createPlatformDescription(ContextMenu*);
    void freePlatformDescription(PlatformMenuDescription);
    void appendItem(PlatformMenuDescription, ContextMenuItem&);
    void show(PlatformMenuDescription menu);
 private:
    Evas_Object* m_view;
};
}

#endif // ContextMenuClientEfl_h
