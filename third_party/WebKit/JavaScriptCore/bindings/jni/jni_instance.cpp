/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
#include <jni_class.h>
#include <jni_instance.h>
#include <jni_runtime.h>
#include <jni_utility.h>
#include <runtime_object.h>

using namespace Bindings;
using namespace KJS;

JavaInstance::JavaInstance (jobject instance) 
{
    _instance = new JObjectWrapper (instance);
};

JavaInstance::~JavaInstance () 
{
    _instance->deref();
}


JavaInstance::JavaInstance (const JavaInstance &other) : Instance() 
{
    _instance = other._instance;
    _instance->ref();
};

Class *JavaInstance::getClass() const 
{
    return JavaClass::classForInstance (_instance->_instance);
}

KJS::Value JavaInstance::stringValue() const
{
    jstring stringValue = (jstring)callJNIObjectMethod (_instance->_instance, "toString", "()Ljava/lang/String;");
    JNIEnv *env = getJNIEnv();
    const char *c = getCharactersFromJStringInEnv (env, stringValue);
    KJS::String v(c);
    releaseCharactersForJStringInEnv (env, stringValue, c);
    return v;
}

KJS::Value JavaInstance::numberValue() const
{
    jdouble doubleValue = callJNIDoubleMethod (_instance->_instance, "doubleValue", "()D");
    KJS::Number v(doubleValue);
    return v;
}

KJS::Value JavaInstance::booleanValue() const
{
    jboolean booleanValue = callJNIBooleanMethod (_instance->_instance, "booleanValue", "()Z");
    KJS::Boolean v(booleanValue);
    return v;
}

Value JavaInstance::invokeMethod (KJS::ExecState *exec, const Method *method, const List &args)
{
    const JavaMethod *jMethod = static_cast<const JavaMethod*>(method);
    int i, count = args.size();
    jvalue *jArgs;
    Value resultValue;
    
    fprintf(stderr,"%s: this=%p, invoking %s with signature %s\n", __PRETTY_FUNCTION__, this, method->name(), jMethod->signature());
    
    if (count > 0) {
        jArgs = (jvalue *)malloc (count * sizeof(jvalue));
    }
    else
        jArgs = 0;
        
    for (i = 0; i < count; i++) {
        JavaParameter *aParameter = static_cast<JavaParameter *>(jMethod->parameterAt(i));
        jArgs[i] = convertValueToJValue (exec, args.at(i), aParameter);
    }
    
    jvalue result;
    switch (jMethod->JNIReturnType()){
        case void_type: {
            callJNIVoidMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Undefined();
        }
        break;
        
        case object_type: {
            result.l = callJNIObjectMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Object(new RuntimeObjectImp(new JavaInstance (result.l)));
        }
        break;
        
        case boolean_type: {
            result.z = callJNIBooleanMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = KJS::Boolean(result.z);
        }
        break;
        
        case byte_type: {
            result.b = callJNIByteMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number(result.b);
        }
        break;
        
        case char_type: {
            result.c = callJNICharMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number(result.c);
        }
        break;
        
        case short_type: {
            result.s = callJNIShortMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number(result.s);
        }
        break;
        
        case int_type: {
            result.i = callJNIIntMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number(result.i);
        }
        break;
        
        case long_type: {
            result.j = callJNILongMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number((long int)result.j);
        }
        break;
        
        case float_type: {
            result.f = callJNIFloatMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number(result.f);
        }
        break;
        
        case double_type: {
            result.d = callJNIDoubleMethodA (_instance->_instance, method->name(), jMethod->signature(), jArgs);
            resultValue = Number(result.d);
        }
        break;

        case invalid_type:
        default: {
            resultValue = Undefined();
        }
        break;
    }
        
    free (jArgs);
    
    return resultValue;
}


JObjectWrapper::JObjectWrapper(jobject instance)
{
    _ref = 1;
    // Cache the JNIEnv used to get the global ref for this java instanace.
    // It'll be used to delete the reference.
    _env = getJNIEnv();
    
    _instance = _env->NewGlobalRef (instance);
    _env->DeleteLocalRef (instance);
    
    if  (_instance == NULL) {
        fprintf (stderr, "%s:  could not get GlobalRef for %p\n", __PRETTY_FUNCTION__, instance);
    }
}
