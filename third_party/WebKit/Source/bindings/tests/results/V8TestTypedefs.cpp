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
#include "V8TestTypedefs.h"

#include "RuntimeEnabledFeatures.h"
#include "V8TestCallbackInterface.h"
#include "V8TestInterfaceEmpty.h"
#include "V8TestSubObj.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestTypedefs* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestTypedefs::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestTypedefs* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestTypedefs::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestTypedefs::domTemplate, V8TestTypedefs::derefObject, 0, 0, 0, V8TestTypedefs::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype, false };

namespace TestTypedefsV8Internal {

template <typename T> void V8_USE(T) { }

static void unsignedLongLongAttrAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    v8SetReturnValue(info, static_cast<double>(imp->unsignedLongLongAttr()));
}

static void unsignedLongLongAttrAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestTypedefsV8Internal::unsignedLongLongAttrAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void unsignedLongLongAttrAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "unsignedLongLongAttr", "TestTypedefs", info.Holder(), info.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_EXCEPTION_VOID(unsigned long long, cppValue, toUInt64(jsValue, exceptionState), exceptionState);
    imp->setUnsignedLongLongAttr(cppValue);
}

static void unsignedLongLongAttrAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestTypedefsV8Internal::unsignedLongLongAttrAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void attrWithGetterExceptionAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    ExceptionState exceptionState(ExceptionState::GetterContext, "attrWithGetterException", "TestTypedefs", info.Holder(), info.GetIsolate());
    int jsValue = imp->attrWithGetterException(exceptionState);
    if (UNLIKELY(exceptionState.throwIfNeeded()))
        return;
    v8SetReturnValueInt(info, jsValue);
}

static void attrWithGetterExceptionAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestTypedefsV8Internal::attrWithGetterExceptionAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void attrWithGetterExceptionAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "attrWithGetterException", "TestTypedefs", info.Holder(), info.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_EXCEPTION_VOID(int, cppValue, toInt32(jsValue, exceptionState), exceptionState);
    imp->setAttrWithGetterException(cppValue);
}

static void attrWithGetterExceptionAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestTypedefsV8Internal::attrWithGetterExceptionAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void attrWithSetterExceptionAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    v8SetReturnValueInt(info, imp->attrWithSetterException());
}

static void attrWithSetterExceptionAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestTypedefsV8Internal::attrWithSetterExceptionAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void attrWithSetterExceptionAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "attrWithSetterException", "TestTypedefs", info.Holder(), info.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_EXCEPTION_VOID(int, cppValue, toInt32(jsValue, exceptionState), exceptionState);
    imp->setAttrWithSetterException(cppValue, exceptionState);
    exceptionState.throwIfNeeded();
}

static void attrWithSetterExceptionAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestTypedefsV8Internal::attrWithSetterExceptionAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringAttrWithGetterExceptionAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    ExceptionState exceptionState(ExceptionState::GetterContext, "stringAttrWithGetterException", "TestTypedefs", info.Holder(), info.GetIsolate());
    String jsValue = imp->stringAttrWithGetterException(exceptionState);
    if (UNLIKELY(exceptionState.throwIfNeeded()))
        return;
    v8SetReturnValueString(info, jsValue, info.GetIsolate());
}

static void stringAttrWithGetterExceptionAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestTypedefsV8Internal::stringAttrWithGetterExceptionAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringAttrWithGetterExceptionAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, cppValue, jsValue);
    imp->setStringAttrWithGetterException(cppValue);
}

static void stringAttrWithGetterExceptionAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestTypedefsV8Internal::stringAttrWithGetterExceptionAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringAttrWithSetterExceptionAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    v8SetReturnValueString(info, imp->stringAttrWithSetterException(), info.GetIsolate());
}

static void stringAttrWithSetterExceptionAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestTypedefsV8Internal::stringAttrWithSetterExceptionAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringAttrWithSetterExceptionAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "stringAttrWithSetterException", "TestTypedefs", info.Holder(), info.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, cppValue, jsValue);
    imp->setStringAttrWithSetterException(cppValue, exceptionState);
    exceptionState.throwIfNeeded();
}

static void stringAttrWithSetterExceptionAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestTypedefsV8Internal::stringAttrWithSetterExceptionAttributeSetter(jsValue, info);
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

static void TestTypedefsReplaceableAttributeSetter(v8::Local<v8::String> name, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    info.This()->ForceSet(name, jsValue);
}

static void TestTypedefsReplaceableAttributeSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TestTypedefsV8Internal::TestTypedefsReplaceableAttributeSetter(name, jsValue, info);
}

static void funcMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    if (UNLIKELY(info.Length() <= 0)) {
        imp->func();
        return;
    }
    V8TRYCATCH_VOID(Vector<int>, x, toNativeArray<int>(info[0], 1, info.GetIsolate()));
    imp->func(x);
}

static void funcMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::funcMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void setShadowMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 3)) {
        throwTypeError(ExceptionMessages::failedToExecute("setShadow", "TestTypedefs", ExceptionMessages::notEnoughArguments(3, info.Length())), info.GetIsolate());
        return;
    }
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(float, width, static_cast<float>(info[0]->NumberValue()));
    V8TRYCATCH_VOID(float, height, static_cast<float>(info[1]->NumberValue()));
    V8TRYCATCH_VOID(float, blur, static_cast<float>(info[2]->NumberValue()));
    if (UNLIKELY(info.Length() <= 3)) {
        imp->setShadow(width, height, blur);
        return;
    }
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, color, info[3]);
    if (UNLIKELY(info.Length() <= 4)) {
        imp->setShadow(width, height, blur, color);
        return;
    }
    V8TRYCATCH_VOID(float, alpha, static_cast<float>(info[4]->NumberValue()));
    imp->setShadow(width, height, blur, color, alpha);
}

static void setShadowMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::setShadowMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void voidMethodTestCallbackInterfaceArgumentMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwTypeError(ExceptionMessages::failedToExecute("voidMethodTestCallbackInterfaceArgument", "TestTypedefs", ExceptionMessages::notEnoughArguments(1, info.Length())), info.GetIsolate());
        return;
    }
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    if (info.Length() <= 0 || !info[0]->IsFunction()) {
        throwTypeError(ExceptionMessages::failedToExecute("voidMethodTestCallbackInterfaceArgument", "TestTypedefs", "The callback provided as parameter 1 is not a function."), info.GetIsolate());
        return;
    }
    OwnPtr<TestCallbackInterface> testCallbackInterface = V8TestCallbackInterface::create(v8::Handle<v8::Function>::Cast(info[0]), currentExecutionContext(info.GetIsolate()));
    imp->voidMethodTestCallbackInterfaceArgument(testCallbackInterface.release());
}

static void voidMethodTestCallbackInterfaceArgumentMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::voidMethodTestCallbackInterfaceArgumentMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void methodWithSequenceArgMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwTypeError(ExceptionMessages::failedToExecute("methodWithSequenceArg", "TestTypedefs", ExceptionMessages::notEnoughArguments(1, info.Length())), info.GetIsolate());
        return;
    }
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(Vector<RefPtr<TestInterfaceEmpty> >, sequenceArg, (toRefPtrNativeArray<TestInterfaceEmpty, V8TestInterfaceEmpty>(info[0], 1, info.GetIsolate())));
    v8SetReturnValue(info, static_cast<double>(imp->methodWithSequenceArg(sequenceArg)));
}

static void methodWithSequenceArgMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::methodWithSequenceArgMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringArrayFunctionMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "stringArrayFunction", "TestTypedefs", info.Holder(), info.GetIsolate());
    if (UNLIKELY(info.Length() < 1)) {
        exceptionState.throwTypeError(ExceptionMessages::notEnoughArguments(1, info.Length()));
        exceptionState.throwIfNeeded();
        return;
    }
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(Vector<String>, values, toNativeArray<String>(info[0], 1, info.GetIsolate()));
    Vector<String> result = imp->stringArrayFunction(values, exceptionState);
    if (exceptionState.throwIfNeeded())
        return;
    v8SetReturnValue(info, v8Array(result, info.GetIsolate()));
}

static void stringArrayFunctionMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::stringArrayFunctionMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void stringArrayFunction2Method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "stringArrayFunction2", "TestTypedefs", info.Holder(), info.GetIsolate());
    if (UNLIKELY(info.Length() < 1)) {
        exceptionState.throwTypeError(ExceptionMessages::notEnoughArguments(1, info.Length()));
        exceptionState.throwIfNeeded();
        return;
    }
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(Vector<String>, values, toNativeArray<String>(info[0], 1, info.GetIsolate()));
    Vector<String> result = imp->stringArrayFunction2(values, exceptionState);
    if (exceptionState.throwIfNeeded())
        return;
    v8SetReturnValue(info, v8Array(result, info.GetIsolate()));
}

static void stringArrayFunction2MethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::stringArrayFunction2Method(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void methodWithExceptionMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "methodWithException", "TestTypedefs", info.Holder(), info.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    imp->methodWithException(exceptionState);
    if (exceptionState.throwIfNeeded())
        return;
}

static void methodWithExceptionMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    TestTypedefsV8Internal::methodWithExceptionMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 1)) {
        throwTypeError(ExceptionMessages::failedToConstruct("TestTypedefs", ExceptionMessages::notEnoughArguments(1, info.Length())), info.GetIsolate());
        return;
    }
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, hello, info[0]);
    RefPtr<TestTypedefs> impl = TestTypedefs::create(hello);
    v8::Handle<v8::Object> wrapper = info.Holder();

    V8DOMWrapper::associateObjectWithWrapper<V8TestTypedefs>(impl.release(), &V8TestTypedefs::wrapperTypeInfo, wrapper, info.GetIsolate(), WrapperConfiguration::Dependent);
    v8SetReturnValue(info, wrapper);
}

} // namespace TestTypedefsV8Internal

static const V8DOMConfiguration::AttributeConfiguration V8TestTypedefsAttributes[] = {
    {"unsignedLongLongAttr", TestTypedefsV8Internal::unsignedLongLongAttrAttributeGetterCallback, TestTypedefsV8Internal::unsignedLongLongAttrAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"TestSubObj", TestTypedefsV8Internal::TestTypedefsConstructorGetter, TestTypedefsV8Internal::TestTypedefsReplaceableAttributeSetterCallback, 0, 0, const_cast<WrapperTypeInfo*>(&V8TestSubObj::wrapperTypeInfo), static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::DontEnum), 0 /* on instance */},
    {"attrWithGetterException", TestTypedefsV8Internal::attrWithGetterExceptionAttributeGetterCallback, TestTypedefsV8Internal::attrWithGetterExceptionAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"attrWithSetterException", TestTypedefsV8Internal::attrWithSetterExceptionAttributeGetterCallback, TestTypedefsV8Internal::attrWithSetterExceptionAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"stringAttrWithGetterException", TestTypedefsV8Internal::stringAttrWithGetterExceptionAttributeGetterCallback, TestTypedefsV8Internal::stringAttrWithGetterExceptionAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"stringAttrWithSetterException", TestTypedefsV8Internal::stringAttrWithSetterExceptionAttributeGetterCallback, TestTypedefsV8Internal::stringAttrWithSetterExceptionAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

static const V8DOMConfiguration::MethodConfiguration V8TestTypedefsMethods[] = {
    {"func", TestTypedefsV8Internal::funcMethodCallback, 0, 0},
    {"setShadow", TestTypedefsV8Internal::setShadowMethodCallback, 0, 3},
    {"voidMethodTestCallbackInterfaceArgument", TestTypedefsV8Internal::voidMethodTestCallbackInterfaceArgumentMethodCallback, 0, 1},
    {"methodWithSequenceArg", TestTypedefsV8Internal::methodWithSequenceArgMethodCallback, 0, 1},
    {"stringArrayFunction", TestTypedefsV8Internal::stringArrayFunctionMethodCallback, 0, 1},
    {"stringArrayFunction2", TestTypedefsV8Internal::stringArrayFunction2MethodCallback, 0, 1},
    {"methodWithException", TestTypedefsV8Internal::methodWithExceptionMethodCallback, 0, 0},
};

void V8TestTypedefs::constructorCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "DOMConstructor");
    if (!info.IsConstructCall()) {
        throwTypeError(ExceptionMessages::failedToConstruct("TestTypedefs", "Please use the 'new' operator, this DOM object constructor cannot be called as a function."), info.GetIsolate());
        return;
    }

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject) {
        v8SetReturnValue(info, info.Holder());
        return;
    }

    TestTypedefsV8Internal::constructor(info);
}

static void configureV8TestTypedefsTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestTypedefs", v8::Local<v8::FunctionTemplate>(), V8TestTypedefs::internalFieldCount,
        V8TestTypedefsAttributes, WTF_ARRAY_LENGTH(V8TestTypedefsAttributes),
        0, 0,
        V8TestTypedefsMethods, WTF_ARRAY_LENGTH(V8TestTypedefsMethods),
        isolate, currentWorldType);
    functionTemplate->SetCallHandler(V8TestTypedefs::constructorCallback);
    functionTemplate->SetLength(1);
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED prototypeTemplate = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::current()->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8TestTypedefs::domTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&wrapperTypeInfo);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, V8ObjectConstructor::isValidConstructorMode);
    configureV8TestTypedefsTemplate(templ, isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&wrapperTypeInfo, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Escape(templ);
}

bool V8TestTypedefs::hasInstance(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstanceInMainWorld(&wrapperTypeInfo, jsValue)
        || V8PerIsolateData::from(isolate)->hasInstanceInNonMainWorld(&wrapperTypeInfo, jsValue);
}

v8::Handle<v8::Object> V8TestTypedefs::createWrapper(PassRefPtr<TestTypedefs> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestTypedefs>(impl.get(), isolate));
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
