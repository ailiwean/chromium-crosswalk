// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py. DO NOT MODIFY!

#include "config.h"
#include "V8TestTypedefs.h"

#include "RuntimeEnabledFeatures.h"
#include "V8Bar.h"
#include "V8Foo.h"
#include "V8TestCallbackInterface.h"
#include "V8TestInterfaceEmpty.h"
#include "V8TestSubObj.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8HiddenValue.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "core/frame/DOMWindow.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestTypedefs* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::fromObject(object)->setTypeInfo(&V8TestTypedefs::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

void webCoreInitializeScriptWrappableForInterface(WebCore::TestTypedefs* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestTypedefs::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestTypedefs::domTemplate, V8TestTypedefs::derefObject, 0, 0, 0, V8TestTypedefs::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype, RefCountedObject };

namespace TestTypedefsV8Internal {

template <typename T> void V8_USE(T) { }

static void uLongLongAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    TestTypedefs* impl = V8TestTypedefs::toNative(holder);
    v8SetReturnValue(info, static_cast<double>(impl->uLongLongAttribute()));
}

static void uLongLongAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestTypedefsV8Internal::uLongLongAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void uLongLongAttributeAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    ExceptionState exceptionState(ExceptionState::SetterContext, "uLongLongAttribute", "TestTypedefs", holder, info.GetIsolate());
    TestTypedefs* impl = V8TestTypedefs::toNative(holder);
    TONATIVE_VOID_EXCEPTIONSTATE(unsigned long long, cppValue, toUInt64(v8Value, exceptionState), exceptionState);
    impl->setULongLongAttribute(cppValue);
}

static void uLongLongAttributeAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestTypedefsV8Internal::uLongLongAttributeAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void TestTypedefsConstructorGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Value> data = info.Data();
    ASSERT(data->IsExternal());
    V8PerContextData* perContextData = V8PerContextData::from(info.Holder()->CreationContext());
    if (!perContextData)
        return;
    v8SetReturnValue(info, perContextData->constructorForType(WrapperTypeInfo::unwrap(data)));
}

static void TestTypedefsReplaceableAttributeSetter(v8::Local<v8::String> name, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    if (info.This()->IsObject())
        v8::Handle<v8::Object>::Cast(info.This())->ForceSet(name, v8Value);
}

static void TestTypedefsReplaceableAttributeSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TestTypedefsV8Internal::TestTypedefsReplaceableAttributeSetter(name, v8Value, info);
}

static void voidMethodArrayOfLongsArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    Vector<int> arrayOfLongsArg;
    {
        v8::TryCatch block;
        if (UNLIKELY(info.Length() <= 0)) {
            impl->voidMethodArrayOfLongsArg();
            return;
        }
        TONATIVE_VOID_INTERNAL(arrayOfLongsArg, toNativeArray<int>(info[0], 1, info.GetIsolate()));
    }
    impl->voidMethodArrayOfLongsArg(arrayOfLongsArg);
}

static void voidMethodArrayOfLongsArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::voidMethodArrayOfLongsArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void voidMethodFloatArgStringArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 2)) {
        throwMinimumArityTypeErrorForMethod("voidMethodFloatArgStringArg", "TestTypedefs", 2, info.Length(), info.GetIsolate());
        return;
    }
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    float floatArg;
    V8StringResource<> stringArg;
    {
        v8::TryCatch block;
        TONATIVE_VOID_INTERNAL(floatArg, static_cast<float>(info[0]->NumberValue()));
        TOSTRING_VOID_INTERNAL(stringArg, info[1]);
    }
    impl->voidMethodFloatArgStringArg(floatArg, stringArg);
}

static void voidMethodFloatArgStringArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::voidMethodFloatArgStringArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void voidMethodTestCallbackInterfaceTypeArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwMinimumArityTypeErrorForMethod("voidMethodTestCallbackInterfaceTypeArg", "TestTypedefs", 1, info.Length(), info.GetIsolate());
        return;
    }
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    OwnPtr<TestCallbackInterface> testCallbackInterfaceTypeArg;
    {
        if (info.Length() <= 0 || !info[0]->IsFunction()) {
            throwTypeError(ExceptionMessages::failedToExecute("voidMethodTestCallbackInterfaceTypeArg", "TestTypedefs", "The callback provided as parameter 1 is not a function."), info.GetIsolate());
            return;
        }
        testCallbackInterfaceTypeArg = V8TestCallbackInterface::create(v8::Handle<v8::Function>::Cast(info[0]), ScriptState::current(info.GetIsolate()));
    }
    impl->voidMethodTestCallbackInterfaceTypeArg(testCallbackInterfaceTypeArg.release());
}

static void voidMethodTestCallbackInterfaceTypeArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::voidMethodTestCallbackInterfaceTypeArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void uLongLongMethodTestInterfaceEmptyTypeSequenceArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwMinimumArityTypeErrorForMethod("uLongLongMethodTestInterfaceEmptyTypeSequenceArg", "TestTypedefs", 1, info.Length(), info.GetIsolate());
        return;
    }
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    Vector<RefPtr<TestInterfaceEmpty> > testInterfaceEmptyTypeSequenceArg;
    {
        v8::TryCatch block;
        TONATIVE_VOID_INTERNAL(testInterfaceEmptyTypeSequenceArg, (toRefPtrNativeArray<TestInterfaceEmpty, V8TestInterfaceEmpty>(info[0], 1, info.GetIsolate())));
    }
    v8SetReturnValue(info, static_cast<double>(impl->uLongLongMethodTestInterfaceEmptyTypeSequenceArg(testInterfaceEmptyTypeSequenceArg)));
}

static void uLongLongMethodTestInterfaceEmptyTypeSequenceArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::uLongLongMethodTestInterfaceEmptyTypeSequenceArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void fooOrBarMethodMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    bool result0Enabled = false;
    RefPtr<Foo> result0;
    bool result1Enabled = false;
    RefPtr<Bar> result1;
    impl->fooOrBarMethod(result0Enabled, result0, result1Enabled, result1);
    if (result0Enabled) {
        v8SetReturnValue(info, result0.release());
        return;
    }
    if (result1Enabled) {
        v8SetReturnValue(info, result1.release());
        return;
    }
    v8SetReturnValueNull(info);
}

static void fooOrBarMethodMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::fooOrBarMethodMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void arrayOfStringsMethodArrayOfStringsArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwMinimumArityTypeErrorForMethod("arrayOfStringsMethodArrayOfStringsArg", "TestTypedefs", 1, info.Length(), info.GetIsolate());
        return;
    }
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    Vector<String> arrayOfStringsArg;
    {
        v8::TryCatch block;
        TONATIVE_VOID_INTERNAL(arrayOfStringsArg, toNativeArray<String>(info[0], 1, info.GetIsolate()));
    }
    v8SetReturnValue(info, v8Array(impl->arrayOfStringsMethodArrayOfStringsArg(arrayOfStringsArg), info.GetIsolate()));
}

static void arrayOfStringsMethodArrayOfStringsArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::arrayOfStringsMethodArrayOfStringsArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringArrayMethodStringArrayArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwMinimumArityTypeErrorForMethod("stringArrayMethodStringArrayArg", "TestTypedefs", 1, info.Length(), info.GetIsolate());
        return;
    }
    TestTypedefs* impl = V8TestTypedefs::toNative(info.Holder());
    Vector<String> stringArrayArg;
    {
        v8::TryCatch block;
        TONATIVE_VOID_INTERNAL(stringArrayArg, toNativeArray<String>(info[0], 1, info.GetIsolate()));
    }
    v8SetReturnValue(info, v8Array(impl->stringArrayMethodStringArrayArg(stringArrayArg), info.GetIsolate()));
}

static void stringArrayMethodStringArrayArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::stringArrayMethodStringArrayArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate* isolate = info.GetIsolate();
    if (UNLIKELY(info.Length() < 1)) {
        throwMinimumArityTypeErrorForConstructor("TestTypedefs", 1, info.Length(), info.GetIsolate());
        return;
    }
    V8StringResource<> stringArg;
    {
        TOSTRING_VOID_INTERNAL_NOTRYCATCH(stringArg, info[0]);
    }
    RefPtr<TestTypedefs> impl = TestTypedefs::create(stringArg);

    v8::Handle<v8::Object> wrapper = info.Holder();
    V8DOMWrapper::associateObjectWithWrapper<V8TestTypedefs>(impl.release(), &V8TestTypedefs::wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    v8SetReturnValue(info, wrapper);
}

} // namespace TestTypedefsV8Internal

static const V8DOMConfiguration::AttributeConfiguration V8TestTypedefsAttributes[] = {
    {"uLongLongAttribute", TestTypedefsV8Internal::uLongLongAttributeAttributeGetterCallback, TestTypedefsV8Internal::uLongLongAttributeAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"tAttribute", TestTypedefsV8Internal::TestTypedefsConstructorGetter, TestTypedefsV8Internal::TestTypedefsReplaceableAttributeSetterCallback, 0, 0, const_cast<WrapperTypeInfo*>(&V8TestSubObj::wrapperTypeInfo), static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::DontEnum), 0 /* on instance */},
};

static const V8DOMConfiguration::MethodConfiguration V8TestTypedefsMethods[] = {
    {"voidMethodArrayOfLongsArg", TestTypedefsV8Internal::voidMethodArrayOfLongsArgMethodCallback, 0, 0},
    {"voidMethodFloatArgStringArg", TestTypedefsV8Internal::voidMethodFloatArgStringArgMethodCallback, 0, 2},
    {"voidMethodTestCallbackInterfaceTypeArg", TestTypedefsV8Internal::voidMethodTestCallbackInterfaceTypeArgMethodCallback, 0, 1},
    {"uLongLongMethodTestInterfaceEmptyTypeSequenceArg", TestTypedefsV8Internal::uLongLongMethodTestInterfaceEmptyTypeSequenceArgMethodCallback, 0, 1},
    {"fooOrBarMethod", TestTypedefsV8Internal::fooOrBarMethodMethodCallback, 0, 0},
    {"arrayOfStringsMethodArrayOfStringsArg", TestTypedefsV8Internal::arrayOfStringsMethodArrayOfStringsArgMethodCallback, 0, 1},
    {"stringArrayMethodStringArrayArg", TestTypedefsV8Internal::stringArrayMethodStringArrayArgMethodCallback, 0, 1},
};

void V8TestTypedefs::constructorCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "DOMConstructor");
    if (!info.IsConstructCall()) {
        throwTypeError(ExceptionMessages::constructorNotCallableAsFunction("TestTypedefs"), info.GetIsolate());
        return;
    }

    if (ConstructorMode::current(info.GetIsolate()) == ConstructorMode::WrapExistingObject) {
        v8SetReturnValue(info, info.Holder());
        return;
    }

    TestTypedefsV8Internal::constructor(info);
}

static void configureV8TestTypedefsTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestTypedefs", v8::Local<v8::FunctionTemplate>(), V8TestTypedefs::internalFieldCount,
        V8TestTypedefsAttributes, WTF_ARRAY_LENGTH(V8TestTypedefsAttributes),
        0, 0,
        V8TestTypedefsMethods, WTF_ARRAY_LENGTH(V8TestTypedefsMethods),
        isolate);
    functionTemplate->SetCallHandler(V8TestTypedefs::constructorCallback);
    functionTemplate->SetLength(1);
    v8::Local<v8::ObjectTemplate> instanceTemplate ALLOW_UNUSED = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> prototypeTemplate ALLOW_UNUSED = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::from(isolate)->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8TestTypedefs::domTemplate(v8::Isolate* isolate)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    v8::Local<v8::FunctionTemplate> result = data->existingDOMTemplate(const_cast<WrapperTypeInfo*>(&wrapperTypeInfo));
    if (!result.IsEmpty())
        return result;

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    result = v8::FunctionTemplate::New(isolate, V8ObjectConstructor::isValidConstructorMode);
    configureV8TestTypedefsTemplate(result, isolate);
    data->setDOMTemplate(const_cast<WrapperTypeInfo*>(&wrapperTypeInfo), result);
    return result;
}

bool V8TestTypedefs::hasInstance(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, v8Value);
}

v8::Handle<v8::Object> V8TestTypedefs::findInstanceInPrototypeChain(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->findInstanceInPrototypeChain(&wrapperTypeInfo, v8Value);
}

TestTypedefs* V8TestTypedefs::toNativeWithTypeCheck(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
    return hasInstance(value, isolate) ? fromInternalPointer(v8::Handle<v8::Object>::Cast(value)->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex)) : 0;
}

v8::Handle<v8::Object> V8TestTypedefs::createWrapper(PassRefPtr<TestTypedefs> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestTypedefs>(impl.get(), isolate));
    if (ScriptWrappable::wrapperCanBeStoredInObject(impl.get())) {
        const WrapperTypeInfo* actualInfo = ScriptWrappable::fromObject(impl.get())->typeInfo();
        // Might be a XXXConstructor::wrapperTypeInfo instead of an XXX::wrapperTypeInfo. These will both have
        // the same object de-ref functions, though, so use that as the basis of the check.
        RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(actualInfo->derefObjectFunction == wrapperTypeInfo.derefObjectFunction);
    }

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &wrapperTypeInfo, toInternalPointer(impl.get()), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;

    installPerContextEnabledProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper<V8TestTypedefs>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestTypedefs::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestTypedefs* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
