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

#include "config.h"
#if ENABLE(Condition1) || ENABLE(Condition2)
#include "V8TestSerializedScriptValueInterface.h"

#include "RuntimeEnabledFeatures.h"
#include "SerializedScriptValue.h"
#include "bindings/v8/ScriptController.h"
#include "bindings/v8/SerializedScriptValue.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8DOMWrapper.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "core/page/Frame.h"
#include "core/platform/chromium/TraceEvent.h"
#include "wtf/UnusedParam.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestSerializedScriptValueInterface* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestSerializedScriptValueInterface::info);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestSerializedScriptValueInterface* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
WrapperTypeInfo V8TestSerializedScriptValueInterface::info = { V8TestSerializedScriptValueInterface::GetTemplate, V8TestSerializedScriptValueInterface::derefObject, 0, 0, 0, V8TestSerializedScriptValueInterface::installPerContextPrototypeProperties, 0, WrapperTypeObjectPrototype };

namespace TestSerializedScriptValueInterfaceV8Internal {

template <typename T> void V8_USE(T) { }

static void valueAttrGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    v8SetReturnValue(info, imp->value() ? imp->value()->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
    return;
}

static void valueAttrGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::valueAttrGetter(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void valueAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    V8TRYCATCH_VOID(RefPtr<SerializedScriptValue>, v, SerializedScriptValue::create(value, info.GetIsolate()));
    imp->setValue(WTF::getPtr(v));
    return;
}

static void valueAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestSerializedScriptValueInterfaceV8Internal::valueAttrSetter(name, value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void readonlyValueAttrGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    v8SetReturnValue(info, imp->readonlyValue() ? imp->readonlyValue()->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
    return;
}

static void readonlyValueAttrGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::readonlyValueAttrGetter(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void cachedValueAttrGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::String> propertyName = v8::String::NewSymbol("cachedValue");
    v8::Handle<v8::Value> value = info.Holder()->GetHiddenValue(propertyName);
    if (!value.IsEmpty()) {
        v8SetReturnValue(info, value);
        return;
    }
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    RefPtr<SerializedScriptValue> serialized = imp->cachedValue();
    value = serialized ? serialized->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate()));
    info.Holder()->SetHiddenValue(propertyName, value);
    v8SetReturnValue(info, value);
    return;
}

static void cachedValueAttrGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::cachedValueAttrGetter(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void cachedValueAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    V8TRYCATCH_VOID(RefPtr<SerializedScriptValue>, v, SerializedScriptValue::create(value, info.GetIsolate()));
    imp->setCachedValue(WTF::getPtr(v));
    info.Holder()->DeleteHiddenValue(v8::String::NewSymbol("cachedValue")); // Invalidate the cached value.
    return;
}

static void cachedValueAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestSerializedScriptValueInterfaceV8Internal::cachedValueAttrSetter(name, value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void cachedReadonlyValueAttrGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::String> propertyName = v8::String::NewSymbol("cachedReadonlyValue");
    v8::Handle<v8::Value> value = info.Holder()->GetHiddenValue(propertyName);
    if (!value.IsEmpty()) {
        v8SetReturnValue(info, value);
        return;
    }
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    RefPtr<SerializedScriptValue> serialized = imp->cachedReadonlyValue();
    value = serialized ? serialized->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate()));
    info.Holder()->SetHiddenValue(propertyName, value);
    v8SetReturnValue(info, value);
    return;
}

static void cachedReadonlyValueAttrGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::cachedReadonlyValueAttrGetter(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

} // namespace TestSerializedScriptValueInterfaceV8Internal

static const V8DOMConfiguration::BatchedAttribute V8TestSerializedScriptValueInterfaceAttrs[] = {
    // Attribute 'value'
    {"value", TestSerializedScriptValueInterfaceV8Internal::valueAttrGetterCallback, TestSerializedScriptValueInterfaceV8Internal::valueAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'readonlyValue'
    {"readonlyValue", TestSerializedScriptValueInterfaceV8Internal::readonlyValueAttrGetterCallback, 0, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'cachedValue'
    {"cachedValue", TestSerializedScriptValueInterfaceV8Internal::cachedValueAttrGetterCallback, TestSerializedScriptValueInterfaceV8Internal::cachedValueAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'cachedReadonlyValue'
    {"cachedReadonlyValue", TestSerializedScriptValueInterfaceV8Internal::cachedReadonlyValueAttrGetterCallback, 0, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

static v8::Handle<v8::FunctionTemplate> ConfigureV8TestSerializedScriptValueInterfaceTemplate(v8::Handle<v8::FunctionTemplate> desc, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    desc->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::configureTemplate(desc, "TestSerializedScriptValueInterface", v8::Local<v8::FunctionTemplate>(), V8TestSerializedScriptValueInterface::internalFieldCount,
        V8TestSerializedScriptValueInterfaceAttrs, WTF_ARRAY_LENGTH(V8TestSerializedScriptValueInterfaceAttrs),
        0, 0, isolate, currentWorldType);
    UNUSED_PARAM(defaultSignature); // In some cases, it will not be used.

    // Custom toString template
    desc->Set(v8::String::NewSymbol("toString"), V8PerIsolateData::current()->toStringTemplate());
    return desc;
}

v8::Handle<v8::FunctionTemplate> V8TestSerializedScriptValueInterface::GetTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&info);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::HandleScope handleScope(isolate);
    v8::Handle<v8::FunctionTemplate> templ =
        ConfigureV8TestSerializedScriptValueInterfaceTemplate(data->rawTemplate(&info, currentWorldType), isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&info, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Close(templ);
}

bool V8TestSerializedScriptValueInterface::HasInstance(v8::Handle<v8::Value> value, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&info, value, currentWorldType);
}

bool V8TestSerializedScriptValueInterface::HasInstanceInAnyWorld(v8::Handle<v8::Value> value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&info, value, MainWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&info, value, IsolatedWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&info, value, WorkerWorld);
}


v8::Handle<v8::Object> V8TestSerializedScriptValueInterface::createWrapper(PassRefPtr<TestSerializedScriptValueInterface> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl.get());
    ASSERT(DOMDataStore::getWrapper<V8TestSerializedScriptValueInterface>(impl.get(), isolate).IsEmpty());

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &info, toInternalPointer(impl.get()), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;
    installPerContextProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper<V8TestSerializedScriptValueInterface>(impl, &info, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}
void V8TestSerializedScriptValueInterface::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

} // namespace WebCore

#endif // ENABLE(Condition1) || ENABLE(Condition2)
