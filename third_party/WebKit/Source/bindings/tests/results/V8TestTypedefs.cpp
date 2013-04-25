/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

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
#include "V8TestTypedefs.h"

#include "BindingState.h"
#include "ContextFeatures.h"
#include "DOMStringList.h"
#include "ExceptionCode.h"
#include "SVGPropertyTearOff.h"
#include "ScriptController.h"
#include "SerializedScriptValue.h"
#include "V8Binding.h"
#include "V8DOMStringList.h"
#include "V8DOMWrapper.h"
#include "V8SVGPoint.h"
#include "V8SerializedScriptValue.h"
#include "V8TestCallback.h"
#include "V8TestSubObj.h"
#include "core/page/Frame.h"
#include "core/page/RuntimeEnabledFeatures.h"
#include <wtf/UnusedParam.h>

#if ENABLE(BINDING_INTEGRITY)
#if defined(OS_WIN)
#pragma warning(disable: 4483)
extern "C" { extern void (*const __identifier("??_7TestTypedefs@WebCore@@6B@")[])(); }
#else
extern "C" { extern void* _ZTVN7WebCore12TestTypedefsE[]; }
#endif
#endif // ENABLE(BINDING_INTEGRITY)

namespace WebCore {

#if ENABLE(BINDING_INTEGRITY)
// This checks if a DOM object that is about to be wrapped is valid.
// Specifically, it checks that a vtable of the DOM object is equal to
// a vtable of an expected class.
// Due to a dangling pointer, the DOM object you are wrapping might be
// already freed or realloced. If freed, the check will fail because
// a free list pointer should be stored at the head of the DOM object.
// If realloced, the check will fail because the vtable of the DOM object
// differs from the expected vtable (unless the same class of DOM object
// is realloced on the slot).
inline void checkTypeOrDieTrying(TestTypedefs* object)
{
    void* actualVTablePointer = *(reinterpret_cast<void**>(object));
#if defined(OS_WIN)
    void* expectedVTablePointer = reinterpret_cast<void*>(__identifier("??_7TestTypedefs@WebCore@@6B@"));
#else
    void* expectedVTablePointer = &_ZTVN7WebCore12TestTypedefsE[2];
#endif
    if (actualVTablePointer != expectedVTablePointer)
        CRASH();
}
#endif // ENABLE(BINDING_INTEGRITY)

#if defined(OS_WIN)
// In ScriptWrappable, the use of extern function prototypes inside templated static methods has an issue on windows.
// These prototypes do not pick up the surrounding namespace, so drop out of WebCore as a workaround.
} // namespace WebCore
using WebCore::ScriptWrappable;
using WebCore::V8TestTypedefs;
using WebCore::TestTypedefs;
#endif
void initializeScriptWrappableForInterface(TestTypedefs* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestTypedefs::info);
}
#if defined(OS_WIN)
namespace WebCore {
#endif
WrapperTypeInfo V8TestTypedefs::info = { V8TestTypedefs::GetTemplate, V8TestTypedefs::derefObject, 0, 0, 0, V8TestTypedefs::installPerContextPrototypeProperties, 0, WrapperTypeObjectPrototype };

namespace TestTypedefsV8Internal {

template <typename T> void V8_USE(T) { }

static v8::Handle<v8::Value> unsignedLongLongAttrAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    return v8::Number::New(static_cast<double>(imp->unsignedLongLongAttr()));
}

static v8::Handle<v8::Value> unsignedLongLongAttrAttrGetterCallback(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::unsignedLongLongAttrAttrGetter(name, info);
}

static void unsignedLongLongAttrAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(unsigned long long, v, toUInt64(value));
    imp->setUnsignedLongLongAttr(v);
    return;
}

static void unsignedLongLongAttrAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefsV8Internal::unsignedLongLongAttrAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> immutableSerializedScriptValueAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    return imp->immutableSerializedScriptValue() ? imp->immutableSerializedScriptValue()->deserialize() : v8::Handle<v8::Value>(v8Null(info.GetIsolate()));
}

static v8::Handle<v8::Value> immutableSerializedScriptValueAttrGetterCallback(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::immutableSerializedScriptValueAttrGetter(name, info);
}

static void immutableSerializedScriptValueAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(RefPtr<SerializedScriptValue>, v, SerializedScriptValue::create(value, info.GetIsolate()));
    imp->setImmutableSerializedScriptValue(WTF::getPtr(v));
    return;
}

static void immutableSerializedScriptValueAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefsV8Internal::immutableSerializedScriptValueAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> attrWithGetterExceptionAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    ExceptionCode ec = 0;
    int v = imp->attrWithGetterException(ec);
    if (UNLIKELY(ec))
        return setDOMException(ec, info.GetIsolate());
    return v8Integer(v, info.GetIsolate());
}

static v8::Handle<v8::Value> attrWithGetterExceptionAttrGetterCallback(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::attrWithGetterExceptionAttrGetter(name, info);
}

static void attrWithGetterExceptionAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(int, v, toInt32(value));
    imp->setAttrWithGetterException(v);
    return;
}

static void attrWithGetterExceptionAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefsV8Internal::attrWithGetterExceptionAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> attrWithSetterExceptionAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    return v8Integer(imp->attrWithSetterException(), info.GetIsolate());
}

static v8::Handle<v8::Value> attrWithSetterExceptionAttrGetterCallback(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::attrWithSetterExceptionAttrGetter(name, info);
}

static void attrWithSetterExceptionAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_VOID(int, v, toInt32(value));
    ExceptionCode ec = 0;
    imp->setAttrWithSetterException(v, ec);
    if (UNLIKELY(ec))
        setDOMException(ec, info.GetIsolate());
    return;
}

static void attrWithSetterExceptionAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefsV8Internal::attrWithSetterExceptionAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> stringAttrWithGetterExceptionAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    ExceptionCode ec = 0;
    String v = imp->stringAttrWithGetterException(ec);
    if (UNLIKELY(ec))
        return setDOMException(ec, info.GetIsolate());
    return v8String(v, info.GetIsolate(), ReturnUnsafeHandle);
}

static v8::Handle<v8::Value> stringAttrWithGetterExceptionAttrGetterCallback(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::stringAttrWithGetterExceptionAttrGetter(name, info);
}

static void stringAttrWithGetterExceptionAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, v, value);
    imp->setStringAttrWithGetterException(v);
    return;
}

static void stringAttrWithGetterExceptionAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefsV8Internal::stringAttrWithGetterExceptionAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> stringAttrWithSetterExceptionAttrGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    return v8String(imp->stringAttrWithSetterException(), info.GetIsolate(), ReturnUnsafeHandle);
}

static v8::Handle<v8::Value> stringAttrWithSetterExceptionAttrGetterCallback(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::stringAttrWithSetterExceptionAttrGetter(name, info);
}

static void stringAttrWithSetterExceptionAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(info.Holder());
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, v, value);
    ExceptionCode ec = 0;
    imp->setStringAttrWithSetterException(v, ec);
    if (UNLIKELY(ec))
        setDOMException(ec, info.GetIsolate());
    return;
}

static void stringAttrWithSetterExceptionAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    TestTypedefsV8Internal::stringAttrWithSetterExceptionAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> TestTypedefsConstructorGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    v8::Handle<v8::Value> data = info.Data();
    ASSERT(data->IsExternal());
    V8PerContextData* perContextData = V8PerContextData::from(info.Holder()->CreationContext());
    if (!perContextData)
        return v8Undefined();
    return perContextData->constructorForType(WrapperTypeInfo::unwrap(data));
}
static void TestTypedefsReplaceableAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    info.This()->ForceSet(name, value);
}

static void TestTypedefsReplaceableAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
{
    return TestTypedefsV8Internal::TestTypedefsReplaceableAttrSetter(name, value, info);
}

static v8::Handle<v8::Value> funcMethod(const v8::Arguments& args)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    if (args.Length() <= 0) {
        imp->func();
        return v8Undefined();
    }
    if (args.Length() > 0 && !isUndefinedOrNull(args[0]) && !V8long[]::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate())))
        return throwTypeError(0, args.GetIsolate());
    V8TRYCATCH(Vector<int>, x, toNativeArray<int>(args[0]));
    imp->func(x);
    return v8Undefined();
}

static v8::Handle<v8::Value> funcMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::funcMethod(args);
}

static v8::Handle<v8::Value> setShadowMethod(const v8::Arguments& args)
{
    if (args.Length() < 3)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    V8TRYCATCH(float, width, static_cast<float>(args[0]->NumberValue()));
    V8TRYCATCH(float, height, static_cast<float>(args[1]->NumberValue()));
    V8TRYCATCH(float, blur, static_cast<float>(args[2]->NumberValue()));
    if (args.Length() <= 3) {
        imp->setShadow(width, height, blur);
        return v8Undefined();
    }
    V8TRYCATCH_FOR_V8STRINGRESOURCE(V8StringResource<>, color, args[3]);
    if (args.Length() <= 4) {
        imp->setShadow(width, height, blur, color);
        return v8Undefined();
    }
    V8TRYCATCH(float, alpha, static_cast<float>(args[4]->NumberValue()));
    imp->setShadow(width, height, blur, color, alpha);
    return v8Undefined();
}

static v8::Handle<v8::Value> setShadowMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::setShadowMethod(args);
}

static v8::Handle<v8::Value> methodWithSequenceArgMethod(const v8::Arguments& args)
{
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    V8TRYCATCH(Vector<RefPtr<SerializedScriptValue> >, sequenceArg, (toRefPtrNativeArray<SerializedScriptValue, V8SerializedScriptValue>(args[0], args.GetIsolate())));
    return v8::Number::New(static_cast<double>(imp->methodWithSequenceArg(sequenceArg)));
}

static v8::Handle<v8::Value> methodWithSequenceArgMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::methodWithSequenceArgMethod(args);
}

static v8::Handle<v8::Value> nullableArrayArgMethod(const v8::Arguments& args)
{
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    V8TRYCATCH(Vector<String>, arrayArg, toNativeArray<String>(args[0]));
    imp->nullableArrayArg(arrayArg);
    return v8Undefined();
}

static v8::Handle<v8::Value> nullableArrayArgMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::nullableArrayArgMethod(args);
}

static v8::Handle<v8::Value> funcWithClampMethod(const v8::Arguments& args)
{
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    unsigned long long arg1 = 0;
    V8TRYCATCH(double, arg1NativeValue, args[0]->NumberValue());
    if (!std::isnan(arg1NativeValue))
        arg1 = clampTo<unsigned long long>(arg1NativeValue);
    if (args.Length() <= 1) {
        imp->funcWithClamp(arg1);
        return v8Undefined();
    }
    unsigned long long arg2 = 0;
    V8TRYCATCH(double, arg2NativeValue, args[1]->NumberValue());
    if (!std::isnan(arg2NativeValue))
        arg2 = clampTo<unsigned long long>(arg2NativeValue);
    imp->funcWithClamp(arg1, arg2);
    return v8Undefined();
}

static v8::Handle<v8::Value> funcWithClampMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::funcWithClampMethod(args);
}

static v8::Handle<v8::Value> immutablePointFunctionMethod(const v8::Arguments& args)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    return toV8(WTF::getPtr(SVGPropertyTearOff<FloatPoint>::create(imp->immutablePointFunction())), args.Holder(), args.GetIsolate());
}

static v8::Handle<v8::Value> immutablePointFunctionMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::immutablePointFunctionMethod(args);
}

static v8::Handle<v8::Value> stringArrayFunctionMethod(const v8::Arguments& args)
{
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    ExceptionCode ec = 0;
    {
    V8TRYCATCH(Vector<String>, values, toNativeArray<String>(args[0]));
    Vector<String> result = imp->stringArrayFunction(values, ec);
    if (UNLIKELY(ec))
        goto fail;
    return v8Array(result, args.GetIsolate());
    }
    fail:
    return setDOMException(ec, args.GetIsolate());
}

static v8::Handle<v8::Value> stringArrayFunctionMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::stringArrayFunctionMethod(args);
}

static v8::Handle<v8::Value> stringArrayFunction2Method(const v8::Arguments& args)
{
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    ExceptionCode ec = 0;
    {
    V8TRYCATCH(Vector<String>, values, toNativeArray<String>(args[0]));
    Vector<String> result = imp->stringArrayFunction2(values, ec);
    if (UNLIKELY(ec))
        goto fail;
    return v8Array(result, args.GetIsolate());
    }
    fail:
    return setDOMException(ec, args.GetIsolate());
}

static v8::Handle<v8::Value> stringArrayFunction2MethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::stringArrayFunction2Method(args);
}

static v8::Handle<v8::Value> methodWithExceptionMethod(const v8::Arguments& args)
{
    TestTypedefs* imp = V8TestTypedefs::toNative(args.Holder());
    ExceptionCode ec = 0;
    {
    imp->methodWithException(ec);
    if (UNLIKELY(ec))
        goto fail;
    return v8Undefined();
    }
    fail:
    return setDOMException(ec, args.GetIsolate());
}

static v8::Handle<v8::Value> methodWithExceptionMethodCallback(const v8::Arguments& args)
{
    return TestTypedefsV8Internal::methodWithExceptionMethod(args);
}

static v8::Handle<v8::Value> constructor(const v8::Arguments& args)
{
    if (args.Length() < 2)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    V8TRYCATCH_FOR_V8STRINGRESOURCE(V8StringResource<>, hello, args[0]);
    if (args.Length() <= 1 || !args[1]->IsFunction())
        return throwTypeError(0, args.GetIsolate());
    RefPtr<TestCallback> testCallback = V8TestCallback::create(args[1], getScriptExecutionContext());

    RefPtr<TestTypedefs> impl = TestTypedefs::create(hello, testCallback);
    v8::Handle<v8::Object> wrapper = args.Holder();

    V8DOMWrapper::associateObjectWithWrapper(impl.release(), &V8TestTypedefs::info, wrapper, args.GetIsolate(), WrapperConfiguration::Dependent);
    return wrapper;
}

} // namespace TestTypedefsV8Internal

static const V8DOMConfiguration::BatchedAttribute V8TestTypedefsAttrs[] = {
    // Attribute 'unsignedLongLongAttr' (Type: 'attribute' ExtAttr: '')
    {"unsignedLongLongAttr", TestTypedefsV8Internal::unsignedLongLongAttrAttrGetterCallback, TestTypedefsV8Internal::unsignedLongLongAttrAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'immutableSerializedScriptValue' (Type: 'attribute' ExtAttr: 'Immutable')
    {"immutableSerializedScriptValue", TestTypedefsV8Internal::immutableSerializedScriptValueAttrGetterCallback, TestTypedefsV8Internal::immutableSerializedScriptValueAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'TestSubObj' (Type: 'readonly attribute' ExtAttr: '')
    {"TestSubObj", TestTypedefsV8Internal::TestTypedefsConstructorGetter, 0, 0, 0, &V8TestSubObj::info, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'attrWithGetterException' (Type: 'attribute' ExtAttr: 'GetterRaisesException')
    {"attrWithGetterException", TestTypedefsV8Internal::attrWithGetterExceptionAttrGetterCallback, TestTypedefsV8Internal::attrWithGetterExceptionAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'attrWithSetterException' (Type: 'attribute' ExtAttr: 'SetterRaisesException')
    {"attrWithSetterException", TestTypedefsV8Internal::attrWithSetterExceptionAttrGetterCallback, TestTypedefsV8Internal::attrWithSetterExceptionAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'stringAttrWithGetterException' (Type: 'attribute' ExtAttr: 'GetterRaisesException')
    {"stringAttrWithGetterException", TestTypedefsV8Internal::stringAttrWithGetterExceptionAttrGetterCallback, TestTypedefsV8Internal::stringAttrWithGetterExceptionAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    // Attribute 'stringAttrWithSetterException' (Type: 'attribute' ExtAttr: 'SetterRaisesException')
    {"stringAttrWithSetterException", TestTypedefsV8Internal::stringAttrWithSetterExceptionAttrGetterCallback, TestTypedefsV8Internal::stringAttrWithSetterExceptionAttrSetterCallback, 0, 0, 0 /* no data */, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

static const V8DOMConfiguration::BatchedMethod V8TestTypedefsMethods[] = {
    {"func", TestTypedefsV8Internal::funcMethodCallback, 0, 0},
    {"setShadow", TestTypedefsV8Internal::setShadowMethodCallback, 0, 3},
    {"funcWithClamp", TestTypedefsV8Internal::funcWithClampMethodCallback, 0, 1},
    {"immutablePointFunction", TestTypedefsV8Internal::immutablePointFunctionMethodCallback, 0, 0},
    {"methodWithException", TestTypedefsV8Internal::methodWithExceptionMethodCallback, 0, 0},
};

v8::Handle<v8::Value> V8TestTypedefs::constructorCallback(const v8::Arguments& args)
{
    if (!args.IsConstructCall())
        return throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    return TestTypedefsV8Internal::constructor(args);
}

static v8::Persistent<v8::FunctionTemplate> ConfigureV8TestTypedefsTemplate(v8::Persistent<v8::FunctionTemplate> desc, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    desc->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::configureTemplate(desc, "TestTypedefs", v8::Persistent<v8::FunctionTemplate>(), V8TestTypedefs::internalFieldCount,
        V8TestTypedefsAttrs, WTF_ARRAY_LENGTH(V8TestTypedefsAttrs),
        V8TestTypedefsMethods, WTF_ARRAY_LENGTH(V8TestTypedefsMethods), isolate, currentWorldType);
    UNUSED_PARAM(defaultSignature); // In some cases, it will not be used.
    desc->SetCallHandler(V8TestTypedefs::constructorCallback);
    desc->SetLength(2);
    v8::Local<v8::ObjectTemplate> instance = desc->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> proto = desc->PrototypeTemplate();
    UNUSED_PARAM(instance); // In some cases, it will not be used.
    UNUSED_PARAM(proto); // In some cases, it will not be used.

    // Custom Signature 'methodWithSequenceArg'
    const int methodWithSequenceArgArgc = 1;
    v8::Handle<v8::FunctionTemplate> methodWithSequenceArgArgv[methodWithSequenceArgArgc] = { V8PerIsolateData::from(isolate)->rawTemplate(&V8sequence<SerializedScriptValue>::info, currentWorldType) };
    v8::Handle<v8::Signature> methodWithSequenceArgSignature = v8::Signature::New(desc, methodWithSequenceArgArgc, methodWithSequenceArgArgv);
    proto->Set(v8::String::NewSymbol("methodWithSequenceArg"), v8::FunctionTemplate::New(TestTypedefsV8Internal::methodWithSequenceArgMethodCallback, v8Undefined(), methodWithSequenceArgSignature));

    // Custom Signature 'nullableArrayArg'
    const int nullableArrayArgArgc = 1;
    v8::Handle<v8::FunctionTemplate> nullableArrayArgArgv[nullableArrayArgArgc] = { V8PerIsolateData::from(isolate)->rawTemplate(&V8DOMString[]::info, currentWorldType) };
    v8::Handle<v8::Signature> nullableArrayArgSignature = v8::Signature::New(desc, nullableArrayArgArgc, nullableArrayArgArgv);
    proto->Set(v8::String::NewSymbol("nullableArrayArg"), v8::FunctionTemplate::New(TestTypedefsV8Internal::nullableArrayArgMethodCallback, v8Undefined(), nullableArrayArgSignature));

    // Custom Signature 'stringArrayFunction'
    const int stringArrayFunctionArgc = 1;
    v8::Handle<v8::FunctionTemplate> stringArrayFunctionArgv[stringArrayFunctionArgc] = { V8PerIsolateData::from(isolate)->rawTemplate(&V8DOMString[]::info, currentWorldType) };
    v8::Handle<v8::Signature> stringArrayFunctionSignature = v8::Signature::New(desc, stringArrayFunctionArgc, stringArrayFunctionArgv);
    proto->Set(v8::String::NewSymbol("stringArrayFunction"), v8::FunctionTemplate::New(TestTypedefsV8Internal::stringArrayFunctionMethodCallback, v8Undefined(), stringArrayFunctionSignature));

    // Custom Signature 'stringArrayFunction2'
    const int stringArrayFunction2Argc = 1;
    v8::Handle<v8::FunctionTemplate> stringArrayFunction2Argv[stringArrayFunction2Argc] = { V8PerIsolateData::from(isolate)->rawTemplate(&V8DOMString[]::info, currentWorldType) };
    v8::Handle<v8::Signature> stringArrayFunction2Signature = v8::Signature::New(desc, stringArrayFunction2Argc, stringArrayFunction2Argv);
    proto->Set(v8::String::NewSymbol("stringArrayFunction2"), v8::FunctionTemplate::New(TestTypedefsV8Internal::stringArrayFunction2MethodCallback, v8Undefined(), stringArrayFunction2Signature));

    // Custom toString template
    desc->Set(v8::String::NewSymbol("toString"), V8PerIsolateData::current()->toStringTemplate());
    return desc;
}

v8::Persistent<v8::FunctionTemplate> V8TestTypedefs::GetTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&info);
    if (result != data->templateMap(currentWorldType).end())
        return result->value;

    v8::HandleScope handleScope;
    v8::Persistent<v8::FunctionTemplate> templ =
        ConfigureV8TestTypedefsTemplate(data->rawTemplate(&info, currentWorldType), isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&info, templ);
    return templ;
}

bool V8TestTypedefs::HasInstance(v8::Handle<v8::Value> value, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&info, value, currentWorldType);
}

bool V8TestTypedefs::HasInstanceInAnyWorld(v8::Handle<v8::Value> value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&info, value, MainWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&info, value, IsolatedWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&info, value, WorkerWorld);
}


v8::Handle<v8::Object> V8TestTypedefs::createWrapper(PassRefPtr<TestTypedefs> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl.get());
    ASSERT(DOMDataStore::getWrapper(impl.get(), isolate).IsEmpty());

#if ENABLE(BINDING_INTEGRITY)
    checkTypeOrDieTrying(impl.get());
#endif

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &info, impl.get(), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;

    installPerContextProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper(impl, &info, wrapper, isolate, hasDependentLifetime ? WrapperConfiguration::Dependent : WrapperConfiguration::Independent);
    return wrapper;
}
void V8TestTypedefs::derefObject(void* object)
{
    static_cast<TestTypedefs*>(object)->deref();
}

} // namespace WebCore
