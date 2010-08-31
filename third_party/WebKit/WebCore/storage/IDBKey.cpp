/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "IDBKey.h"

#if ENABLE(INDEXED_DATABASE)

#include "SerializedScriptValue.h"

namespace WebCore {

IDBKey::IDBKey()
    : m_type(NullType)
{
}

IDBKey::IDBKey(int32_t number)
    : m_type(NumberType)
    , m_number(number)
{
}

IDBKey::IDBKey(const String& string)
    : m_type(StringType)
    , m_string(string)
{
}

IDBKey::~IDBKey()
{
}

bool IDBKey::isEqual(IDBKey* other)
{
    if (!other || other->m_type != m_type)
        return false;

    switch (m_type) {
    case StringType:
        return other->m_string == m_string;
    // FIXME: Implement dates.
    case NumberType:
        return other->m_number == m_number;
    case NullType:
        return true;
    }

    ASSERT_NOT_REACHED();
    return false;
}

} // namespace WebCore

#endif
