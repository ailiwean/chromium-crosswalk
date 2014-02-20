/*
 *  Copyright (C) 2003, 2006 Apple Computer, Inc.
 *  Copyright (C) 2006 Samuel Weinig (sam@webkit.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef XMLSerializer_h
#define XMLSerializer_h

#include "bindings/v8/ScriptWrappable.h"
#include "heap/Handle.h"
#include "wtf/Forward.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefCounted.h"

namespace WebCore {

class ExceptionState;
class Node;

class XMLSerializer : public RefCountedWillBeGarbageCollectedFinalized<XMLSerializer>, public ScriptWrappable {
    DECLARE_GC_INFO;
public:
    static PassRefPtrWillBeRawPtr<XMLSerializer> create()
    {
        return adoptRefWillBeNoop(new XMLSerializer);
    }

    String serializeToString(Node*, ExceptionState&);

    void trace(Visitor*) { }

private:
    XMLSerializer()
    {
        ScriptWrappable::init(this);
    }
};

} // namespace WebCore

#endif // XMLSerializer_h
