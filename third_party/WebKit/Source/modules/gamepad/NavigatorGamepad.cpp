/*
 * Copyright (C) 2011, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "config.h"
#include "modules/gamepad/NavigatorGamepad.h"

#include "core/frame/Navigator.h"
#include "modules/gamepad/GamepadList.h"
#include "public/platform/Platform.h"
#include "wtf/PassOwnPtr.h"

namespace WebCore {

static void sampleGamepads(GamepadList* into)
{
    blink::WebGamepads gamepads;

    blink::Platform::current()->sampleGamepads(gamepads);

    for (unsigned i = 0; i < blink::WebGamepads::itemsLengthCap; ++i) {
        blink::WebGamepad& webGamepad = gamepads.items[i];
        if (i < gamepads.length && webGamepad.connected) {
            RefPtr<Gamepad> gamepad = into->item(i);
            if (!gamepad)
                gamepad = Gamepad::create();
            gamepad->id(webGamepad.id);
            gamepad->index(i);
            gamepad->timestamp(webGamepad.timestamp);
            gamepad->axes(webGamepad.axesLength, webGamepad.axes);
            gamepad->buttons(webGamepad.buttonsLength, webGamepad.buttons);
            into->set(i, gamepad);
        } else {
            into->set(i, 0);
        }
    }
}

NavigatorGamepad::NavigatorGamepad()
{
}

NavigatorGamepad::~NavigatorGamepad()
{
}

const char* NavigatorGamepad::supplementName()
{
    return "NavigatorGamepad";
}

NavigatorGamepad* NavigatorGamepad::from(Navigator* navigator)
{
    NavigatorGamepad* supplement = static_cast<NavigatorGamepad*>(Supplement<Navigator>::from(navigator, supplementName()));
    if (!supplement) {
        supplement = new NavigatorGamepad();
        provideTo(navigator, supplementName(), adoptPtr(supplement));
    }
    return supplement;
}

GamepadList* NavigatorGamepad::webkitGetGamepads(Navigator* navigator)
{
    return NavigatorGamepad::from(navigator)->gamepads();
}

GamepadList* NavigatorGamepad::gamepads()
{
    if (!m_gamepads)
        m_gamepads = GamepadList::create();
    sampleGamepads(m_gamepads.get());
    return m_gamepads.get();
}

} // namespace WebCore
