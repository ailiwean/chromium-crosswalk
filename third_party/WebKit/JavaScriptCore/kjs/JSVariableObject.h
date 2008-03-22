/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
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

#ifndef JSVariableObject_h
#define JSVariableObject_h

#include "LocalStorage.h"
#include "SymbolTable.h"
#include "object.h"

namespace KJS {

    class JSVariableObject : public JSObject {
    public:
        SymbolTable& symbolTable() const { return *d->symbolTable; }
        LocalStorage& localStorage() const { return d->localStorage; }
        
        void saveLocalStorage(SavedProperties&) const;
        void restoreLocalStorage(const SavedProperties&);

        virtual void initializeVariable(ExecState*, const Identifier&, JSValue*, unsigned attributes) = 0;
        
        virtual bool deleteProperty(ExecState*, const Identifier&);
        virtual void getPropertyNames(ExecState*, PropertyNameArray&);
        
        virtual void mark();

        virtual bool isVariableObject() const;
        virtual bool isDynamicScope() const = 0;

        virtual bool getPropertyAttributes(const Identifier& propertyName, unsigned& attributes) const;

    protected:
        // Subclasses of JSVariableObject can subclass this struct to add data
        // without increasing their own size (since there's a hard limit on the
        // size of a JSCell).
        struct JSVariableObjectData {
            JSVariableObjectData() { }
            JSVariableObjectData(SymbolTable* s)
                : symbolTable(s) // Subclass owns this pointer.
            {
            }

            LocalStorage localStorage; // Storage for variables in the symbol table.
            SymbolTable* symbolTable; // Maps name -> index in localStorage.
        };

        JSVariableObject() { }

        JSVariableObject(JSVariableObjectData* data)
            : d(data) // Subclass owns this pointer.
        {
        }

        JSVariableObject(JSValue* proto, JSVariableObjectData* data)
            : JSObject(proto)
            , d(data) // Subclass owns this pointer.
        {
        }

        bool symbolTableGet(const Identifier&, PropertySlot&);
        bool symbolTablePut(const Identifier&, JSValue*);
        bool symbolTableInitializeVariable(const Identifier&, JSValue*, unsigned attributes);

        JSVariableObjectData* d;
    };

    inline bool JSVariableObject::symbolTableGet(const Identifier& propertyName, PropertySlot& slot)
    {
        size_t index = symbolTable().get(propertyName.ustring().rep());
        if (index != missingSymbolMarker()) {
#ifndef NDEBUG
            // During initialization, the variable object needs to advertise that it has certain
            // properties, even if they're not ready for access yet. This check verifies that
            // no one tries to access such a property.
            
            // In a release build, we optimize this check away and just return an invalid pointer.
            // There's no harm in an invalid pointer, since no one dereferences it.
            if (index >= d->localStorage.size()) {
                slot.setUngettable(this);
                return true;
            }
#endif
            slot.setValueSlot(this, &d->localStorage[index].value);
            return true;
        }
        return false;
    }

    inline bool JSVariableObject::symbolTablePut(const Identifier& propertyName, JSValue* value)
    {
        size_t index = symbolTable().get(propertyName.ustring().rep());
        if (index == missingSymbolMarker())
            return false;
        LocalStorageEntry& entry = d->localStorage[index];
        if (entry.attributes & ReadOnly)
            return true;
        entry.value = value;
        return true;
    }

    inline bool JSVariableObject::symbolTableInitializeVariable(const Identifier& propertyName, JSValue* value, unsigned attributes)
    {
        size_t index = symbolTable().get(propertyName.ustring().rep());
        if (index == missingSymbolMarker())
            return false;
        LocalStorageEntry& entry = d->localStorage[index];
        entry.value = value;
        entry.attributes = attributes;
        return true;
    }

} // namespace KJS

#endif // JSVariableObject_h
