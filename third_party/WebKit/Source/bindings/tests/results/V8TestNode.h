/*
    This file is part of the Blink open source project.
    This file has been auto-generated by CodeGeneratorV8.pm. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef V8TestNode_h
#define V8TestNode_h

#include "V8Node.h"
#include "bindings/tests/idls/TestNode.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8DOMWrapper.h"
#include "bindings/v8/WrapperTypeInfo.h"

namespace WebCore {

class V8TestNode {
public:
    static bool HasInstance(v8::Handle<v8::Value>, v8::Isolate*, WrapperWorldType);
    static bool HasInstanceInAnyWorld(v8::Handle<v8::Value>, v8::Isolate*);
    static v8::Handle<v8::FunctionTemplate> GetTemplate(v8::Isolate*, WrapperWorldType);
    static TestNode* toNative(v8::Handle<v8::Object> object)
    {
        return fromInternalPointer(object->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex));
    }
    static void derefObject(void*);
    static const WrapperTypeInfo wrapperTypeInfo;
    static EventTarget* toEventTarget(v8::Handle<v8::Object>);
    static void constructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);
    static const int internalFieldCount = v8DefaultWrapperInternalFieldCount + 0;
    static inline void* toInternalPointer(TestNode* impl)
    {
        return V8Node::toInternalPointer(impl);
    }

    static inline TestNode* fromInternalPointer(void* object)
    {
        return static_cast<TestNode*>(V8Node::fromInternalPointer(object));
    }
    static void installPerContextEnabledProperties(v8::Handle<v8::Object>, TestNode*, v8::Isolate*) { }
    static void installPerContextEnabledPrototypeProperties(v8::Handle<v8::Object>, v8::Isolate*) { }

private:
    friend v8::Handle<v8::Object> wrap(TestNode*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
    static v8::Handle<v8::Object> createWrapper(PassRefPtr<TestNode>, v8::Handle<v8::Object> creationContext, v8::Isolate*);
};

template<>
class WrapperTypeTraits<TestNode > {
public:
    static const WrapperTypeInfo* wrapperTypeInfo() { return &V8TestNode::wrapperTypeInfo; }
};

inline v8::Handle<v8::Object> wrap(TestNode* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestNode>(impl, isolate));
    return V8TestNode::createWrapper(impl, creationContext, isolate);
}

inline v8::Handle<v8::Value> toV8(TestNode* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    if (UNLIKELY(!impl))
        return v8NullWithCheck(isolate);
    v8::Handle<v8::Value> wrapper = DOMDataStore::getWrapper<V8TestNode>(impl, isolate);
    if (!wrapper.IsEmpty())
        return wrapper;
    return wrap(impl, creationContext, isolate);
}

template<typename CallbackInfo>
inline void v8SetReturnValue(const CallbackInfo& callbackInfo, TestNode* impl, v8::Handle<v8::Object> creationContext)
{
    if (UNLIKELY(!impl)) {
        v8SetReturnValueNull(callbackInfo);
        return;
    }
    if (DOMDataStore::setReturnValueFromWrapper<V8TestNode>(callbackInfo.GetReturnValue(), impl))
        return;
    v8::Handle<v8::Object> wrapper = wrap(impl, creationContext, callbackInfo.GetIsolate());
    v8SetReturnValue(callbackInfo, wrapper);
}

template<typename CallbackInfo>
inline void v8SetReturnValueForMainWorld(const CallbackInfo& callbackInfo, TestNode* impl, v8::Handle<v8::Object> creationContext)
{
    ASSERT(worldType(callbackInfo.GetIsolate()) == MainWorld);
    if (UNLIKELY(!impl)) {
        v8SetReturnValueNull(callbackInfo);
        return;
    }
    if (DOMDataStore::setReturnValueFromWrapperForMainWorld<V8TestNode>(callbackInfo.GetReturnValue(), impl))
        return;
    v8::Handle<v8::Value> wrapper = wrap(impl, creationContext, callbackInfo.GetIsolate());
    v8SetReturnValue(callbackInfo, wrapper);
}

template<class CallbackInfo, class Wrappable>
inline void v8SetReturnValueFast(const CallbackInfo& callbackInfo, TestNode* impl, Wrappable* wrappable)
{
    if (UNLIKELY(!impl)) {
        v8SetReturnValueNull(callbackInfo);
        return;
    }
    if (DOMDataStore::setReturnValueFromWrapperFast<V8TestNode>(callbackInfo.GetReturnValue(), impl, callbackInfo.Holder(), wrappable))
        return;
    v8::Handle<v8::Object> wrapper = wrap(impl, callbackInfo.Holder(), callbackInfo.GetIsolate());
    v8SetReturnValue(callbackInfo, wrapper);
}

inline v8::Handle<v8::Value> toV8(PassRefPtr<TestNode > impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl.get(), creationContext, isolate);
}

template<class CallbackInfo>
inline void v8SetReturnValue(const CallbackInfo& callbackInfo, PassRefPtr<TestNode > impl, v8::Handle<v8::Object> creationContext)
{
    v8SetReturnValue(callbackInfo, impl.get(), creationContext);
}

template<class CallbackInfo>
inline void v8SetReturnValueForMainWorld(const CallbackInfo& callbackInfo, PassRefPtr<TestNode > impl, v8::Handle<v8::Object> creationContext)
{
    v8SetReturnValueForMainWorld(callbackInfo, impl.get(), creationContext);
}

template<class CallbackInfo, class Wrappable>
inline void v8SetReturnValueFast(const CallbackInfo& callbackInfo, PassRefPtr<TestNode > impl, Wrappable* wrappable)
{
    v8SetReturnValueFast(callbackInfo, impl.get(), wrappable);
}

}

#endif // V8TestNode_h
