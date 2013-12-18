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

// This file has been auto-generated by code_generator_v8.pm. DO NOT MODIFY!

#include "config.h"
#include "V8TestInterfaceEventConstructor.h"

#include "RuntimeEnabledFeatures.h"
#include "V8TestInterfaceEmpty.h"
#include "bindings/v8/Dictionary.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/ScriptValue.h"
#include "bindings/v8/SerializedScriptValue.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8HiddenPropertyName.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "bindings/v8/custom/V8Uint8ArrayCustom.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "core/frame/UseCounter.h"
#include "platform/TraceEvent.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestInterfaceEventConstructor* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestInterfaceEventConstructor::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestInterfaceEventConstructor* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestInterfaceEventConstructor::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestInterfaceEventConstructor::domTemplate, V8TestInterfaceEventConstructor::derefObject, 0, 0, 0, V8TestInterfaceEventConstructor::installPerContextEnabledMethods, &V8Event::wrapperTypeInfo, WrapperTypeObjectPrototype };

namespace TestInterfaceEventConstructorV8Internal {

template <typename T> void V8_USE(T) { }

static void readonlyStringAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueString(info, imp->readonlyStringAttribute(), info.GetIsolate());
}

static void readonlyStringAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::readonlyStringAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyStringAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueString(info, imp->initializedByEventConstructorReadonlyStringAttribute(), info.GetIsolate());
}

static void initializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyStringAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyAnyAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValue(info, imp->initializedByEventConstructorReadonlyAnyAttribute().v8Value());
}

static void initializedByEventConstructorReadonlyAnyAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyAnyAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyBooleanAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueBool(info, imp->initializedByEventConstructorReadonlyBooleanAttribute());
}

static void initializedByEventConstructorReadonlyBooleanAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyBooleanAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyLongAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueInt(info, imp->initializedByEventConstructorReadonlyLongAttribute());
}

static void initializedByEventConstructorReadonlyLongAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyLongAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyUint8ArrayAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    RefPtr<Uint8Array> result = imp->initializedByEventConstructorReadonlyUint8ArrayAttribute();
    if (result && DOMDataStore::setReturnValueFromWrapper<V8Uint8Array>(info.GetReturnValue(), result.get()))
        return;
    v8::Handle<v8::Value> wrapper = toV8(result.get(), info.Holder(), info.GetIsolate());
    if (!wrapper.IsEmpty()) {
        V8HiddenPropertyName::setNamedHiddenReference(info.Holder(), "initializedByEventConstructorReadonlyUint8ArrayAttribute", wrapper);
        v8SetReturnValue(info, wrapper);
    }
}

static void initializedByEventConstructorReadonlyUint8ArrayAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyUint8ArrayAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyTestInterfaceEmptyAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    RefPtr<TestInterfaceEmpty> result = imp->initializedByEventConstructorReadonlyTestInterfaceEmptyAttribute();
    if (result && DOMDataStore::setReturnValueFromWrapper<V8TestInterfaceEmpty>(info.GetReturnValue(), result.get()))
        return;
    v8::Handle<v8::Value> wrapper = toV8(result.get(), info.Holder(), info.GetIsolate());
    if (!wrapper.IsEmpty()) {
        V8HiddenPropertyName::setNamedHiddenReference(info.Holder(), "initializedByEventConstructorReadonlyTestInterfaceEmptyAttribute", wrapper);
        v8SetReturnValue(info, wrapper);
    }
}

static void initializedByEventConstructorReadonlyTestInterfaceEmptyAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyTestInterfaceEmptyAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValue(info, v8Array(imp->initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttribute(), info.GetIsolate()));
}

static void initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    bool isNull = false;
    RefPtr<TestInterfaceEmpty> jsValue = imp->initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttribute(isNull);
    if (isNull) {
        v8SetReturnValueNull(info);
        return;
    }
    RefPtr<TestInterfaceEmpty> result = jsValue;
    if (result && DOMDataStore::setReturnValueFromWrapper<V8TestInterfaceEmpty>(info.GetReturnValue(), result.get()))
        return;
    v8::Handle<v8::Value> wrapper = toV8(result.get(), info.Holder(), info.GetIsolate());
    if (!wrapper.IsEmpty()) {
        V8HiddenPropertyName::setNamedHiddenReference(info.Holder(), "initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttribute", wrapper);
        v8SetReturnValue(info, wrapper);
    }
}

static void initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void deprecatedInitializedByEventConstructorReadonlyStringAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueString(info, imp->deprecatedInitializedByEventConstructorReadonlyStringAttribute(), info.GetIsolate());
}

static void deprecatedInitializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    UseCounter::countDeprecation(activeExecutionContext(), UseCounter::initializedByEventConstructorReadonlyStringAttribute);
    TestInterfaceEventConstructorV8Internal::deprecatedInitializedByEventConstructorReadonlyStringAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void implementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueString(info, imp->implementedAsName(), info.GetIsolate());
}

static void implementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestInterfaceEventConstructorV8Internal::implementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestInterfaceEventConstructor* imp = V8TestInterfaceEventConstructor::toNative(info.Holder());
    v8SetReturnValueString(info, imp->deprecatedImplementedAsName(), info.GetIsolate());
}

static void deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    UseCounter::countDeprecation(activeExecutionContext(), UseCounter::initializedByEventConstructorReadonlyStringAttribute);
    TestInterfaceEventConstructorV8Internal::deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ConstructionContext, "TestInterfaceEventConstructor", info.Holder(), info.GetIsolate());
    if (info.Length() < 1) {
        exceptionState.throwTypeError("An event name must be provided.");
        exceptionState.throwIfNeeded();
        return;
    }

    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, type, info[0]);
    v8::Local<v8::Value> initializedByEventConstructorReadonlyAnyAttribute;
    TestInterfaceEventConstructorInit eventInit;
    if (info.Length() >= 2) {
        V8TRYCATCH_VOID(Dictionary, options, Dictionary(info[1], info.GetIsolate()));
        if (!initializeTestInterfaceEventConstructor(eventInit, options, exceptionState)) {
            exceptionState.throwIfNeeded();
            return;
        }
        options.get("initializedByEventConstructorReadonlyAnyAttribute", initializedByEventConstructorReadonlyAnyAttribute);
        if (!initializedByEventConstructorReadonlyAnyAttribute.IsEmpty())
            info.Holder()->SetHiddenValue(V8HiddenPropertyName::initializedByEventConstructorReadonlyAnyAttribute(info.GetIsolate()), initializedByEventConstructorReadonlyAnyAttribute);
    }
    RefPtr<TestInterfaceEventConstructor> event = TestInterfaceEventConstructor::create(type, eventInit, exceptionState);
    if (exceptionState.throwIfNeeded())
        return;
    if (isolatedWorldForIsolate(info.GetIsolate())) {
        if (!initializedByEventConstructorReadonlyAnyAttribute.IsEmpty())
            event->setSerializedInitializedByEventConstructorReadonlyAnyAttribute(SerializedScriptValue::createAndSwallowExceptions(initializedByEventConstructorReadonlyAnyAttribute, info.GetIsolate()));
    }

    v8::Handle<v8::Object> wrapper = info.Holder();
    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceEventConstructor>(event.release(), &V8TestInterfaceEventConstructor::wrapperTypeInfo, wrapper, info.GetIsolate(), WrapperConfiguration::Dependent);
    v8SetReturnValue(info, wrapper);
}

} // namespace TestInterfaceEventConstructorV8Internal

static const V8DOMConfiguration::AttributeConfiguration V8TestInterfaceEventConstructorAttributes[] = {
    {"readonlyStringAttribute", TestInterfaceEventConstructorV8Internal::readonlyStringAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyStringAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyAnyAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyAnyAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyBooleanAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyBooleanAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyLongAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyLongAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyUint8ArrayAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyUint8ArrayAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyTestInterfaceEmptyAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyTestInterfaceEmptyAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttribute", TestInterfaceEventConstructorV8Internal::initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"deprecatedInitializedByEventConstructorReadonlyStringAttribute", TestInterfaceEventConstructorV8Internal::deprecatedInitializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"implementedAsInitializedByEventConstructorReadonlyStringAttribute", TestInterfaceEventConstructorV8Internal::implementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttribute", TestInterfaceEventConstructorV8Internal::deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttributeAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

bool initializeTestInterfaceEventConstructor(TestInterfaceEventConstructorInit& eventInit, const Dictionary& options, ExceptionState& exceptionState, const String& forEventName)
{
    Dictionary::ConversionContext conversionContext(forEventName.isEmpty() ? String("TestInterfaceEventConstructor") : forEventName, "", exceptionState);
    if (!initializeEvent(eventInit, options, exceptionState, forEventName.isEmpty() ? String("TestInterfaceEventConstructor") : forEventName))
        return false;

    if (!options.convert(conversionContext.setConversionType("DOMString", false), "initializedByEventConstructorReadonlyStringAttribute", eventInit.initializedByEventConstructorReadonlyStringAttribute))
        return false;
    if (!options.convert(conversionContext.setConversionType("boolean", false), "initializedByEventConstructorReadonlyBooleanAttribute", eventInit.initializedByEventConstructorReadonlyBooleanAttribute))
        return false;
    if (!options.convert(conversionContext.setConversionType("long", false), "initializedByEventConstructorReadonlyLongAttribute", eventInit.initializedByEventConstructorReadonlyLongAttribute))
        return false;
    if (!options.convert(conversionContext.setConversionType("Uint8Array", false), "initializedByEventConstructorReadonlyUint8ArrayAttribute", eventInit.initializedByEventConstructorReadonlyUint8ArrayAttribute))
        return false;
    if (!options.convert(conversionContext.setConversionType("TestInterfaceEmpty", false), "initializedByEventConstructorReadonlyTestInterfaceEmptyAttribute", eventInit.initializedByEventConstructorReadonlyTestInterfaceEmptyAttribute))
        return false;
    if (!options.convert(conversionContext.setConversionType("TestInterfaceEmpty[]", false), "initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttribute", eventInit.initializedByEventConstructorReadonlyTestInterfaceEmptyArrayAttribute))
        return false;
    if (!options.convert(conversionContext.setConversionType("TestInterfaceEmpty", true), "initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttribute", eventInit.initializedByEventConstructorReadonlyNullableTestInterfaceEmptyAttribute))
        return false;
    if (options.convert(conversionContext.setConversionType("DOMString", false), "deprecatedInitializedByEventConstructorReadonlyStringAttribute", eventInit.deprecatedInitializedByEventConstructorReadonlyStringAttribute)) {
        if (options.hasProperty("deprecatedInitializedByEventConstructorReadonlyStringAttribute"))
            UseCounter::countDeprecation(activeExecutionContext(), UseCounter::initializedByEventConstructorReadonlyStringAttribute);
    } else {
        return false;
    }
    if (!options.convert(conversionContext.setConversionType("DOMString", false), "implementedAsInitializedByEventConstructorReadonlyStringAttribute", eventInit.implementedAsName))
        return false;
    if (options.convert(conversionContext.setConversionType("DOMString", false), "deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttribute", eventInit.deprecatedImplementedAsName)) {
        if (options.hasProperty("deprecatedImplementedAsInitializedByEventConstructorReadonlyStringAttribute"))
            UseCounter::countDeprecation(activeExecutionContext(), UseCounter::initializedByEventConstructorReadonlyStringAttribute);
    } else {
        return false;
    }
    return true;
}

void V8TestInterfaceEventConstructor::constructorCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "DOMConstructor");
    if (!info.IsConstructCall()) {
        throwTypeError(ExceptionMessages::failedToConstruct("TestInterfaceEventConstructor", "Please use the 'new' operator, this DOM object constructor cannot be called as a function."), info.GetIsolate());
        return;
    }

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject) {
        v8SetReturnValue(info, info.Holder());
        return;
    }

    TestInterfaceEventConstructorV8Internal::constructor(info);
}

static v8::Handle<v8::FunctionTemplate> ConfigureV8TestInterfaceEventConstructorTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestInterfaceEventConstructor", V8Event::domTemplate(isolate, currentWorldType), V8TestInterfaceEventConstructor::internalFieldCount,
        V8TestInterfaceEventConstructorAttributes, WTF_ARRAY_LENGTH(V8TestInterfaceEventConstructorAttributes),
        0, 0,
        0, 0,
        isolate, currentWorldType);
    functionTemplate->SetCallHandler(V8TestInterfaceEventConstructor::constructorCallback);
    functionTemplate->SetLength(1);
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED prototypeTemplate = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8::String::NewFromUtf8(isolate, "toString", v8::String::kInternalizedString), V8PerIsolateData::current()->toStringTemplate());
    return functionTemplate;
}

v8::Handle<v8::FunctionTemplate> V8TestInterfaceEventConstructor::domTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&wrapperTypeInfo);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> templ =
        ConfigureV8TestInterfaceEventConstructorTemplate(data->rawDOMTemplate(&wrapperTypeInfo, currentWorldType), isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&wrapperTypeInfo, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Escape(templ);
}

bool V8TestInterfaceEventConstructor::hasInstance(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, currentWorldType);
}

bool V8TestInterfaceEventConstructor::hasInstanceInAnyWorld(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, MainWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, IsolatedWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, WorkerWorld);
}

v8::Handle<v8::Object> V8TestInterfaceEventConstructor::createWrapper(PassRefPtr<TestInterfaceEventConstructor> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestInterfaceEventConstructor>(impl.get(), isolate));
    if (ScriptWrappable::wrapperCanBeStoredInObject(impl.get())) {
        const WrapperTypeInfo* actualInfo = ScriptWrappable::getTypeInfoFromObject(impl.get());
        // Might be a XXXConstructor::wrapperTypeInfo instead of an XXX::wrapperTypeInfo. These will both have
        // the same object de-ref functions, though, so use that as the basis of the check.
        RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(actualInfo->derefObjectFunction == wrapperTypeInfo.derefObjectFunction);
    }

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &wrapperTypeInfo, toInternalPointer(impl.get()), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;

    installPerContextEnabledProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceEventConstructor>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestInterfaceEventConstructor::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestInterfaceEventConstructor* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
