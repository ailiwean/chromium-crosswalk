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

#include <jni_utility.h>
#include <jni_runtime.h>

using namespace Bindings;

void JavaClass::_commonInit (jobject aClass)
{
    long i;

    JNIEnv *env = getJNIEnv();
    
    // Get the fields
    jarray fields = (jarray)callJNIObjectMethod (aClass, "getFields", "()[Ljava/lang/reflect/Field;");
    long numFields = env->GetArrayLength (fields);    
    _fields = CFDictionaryCreateMutable(NULL, numFields, &kCFTypeDictionaryKeyCallBacks, NULL);
    for (i = 0; i < numFields; i++) {
        jobject aJField = env->GetObjectArrayElement ((jobjectArray)fields, i);
        Field *aField = new JavaField (env, aJField);
        CFStringRef fieldName = CFStringCreateWithCString(NULL, aField->name(), kCFStringEncodingASCII);
        CFDictionaryAddValue ((CFMutableDictionaryRef)_fields, fieldName, aField);
        CFRelease (fieldName);
    }
    
    // Get the methods
    jarray methods = (jarray)callJNIObjectMethod (aClass, "getMethods", "()[Ljava/lang/reflect/Method;");
    long numMethods = env->GetArrayLength (methods);    
    _methods = CFDictionaryCreateMutable(NULL, numMethods, &kCFTypeDictionaryKeyCallBacks, NULL);
    for (i = 0; i < numMethods; i++) {
        jobject aJMethod = env->GetObjectArrayElement ((jobjectArray)methods, i);
        Method *aMethod = new JavaMethod (env, aJMethod);
        CFStringRef methodName = CFStringCreateWithCString(NULL, aMethod->name(), kCFStringEncodingASCII);
        CFDictionaryAddValue ((CFMutableDictionaryRef)_methods, methodName, aMethod);
        CFRelease (methodName);
    }

    // Get the constructors
    jarray constructors = (jarray)callJNIObjectMethod (aClass, "getConstructors", "()[Ljava/lang/reflect/Constructor;");
    _numConstructors = env->GetArrayLength (constructors);    
    _constructors = new JavaConstructor[_numConstructors];
    for (i = 0; i < _numConstructors; i++) {
        jobject aConstructor = env->GetObjectArrayElement ((jobjectArray)constructors, i);
        _constructors[i] = JavaConstructor (env, aConstructor);
    }
}

JavaClass::JavaClass (const char *className)
{
    JNIEnv *env = getJNIEnv();
    
    _name = strdup (className);
    
    // Get the class
    jclass aClass = env->FindClass(_name);
    if (!aClass){   
        fprintf (stderr, "%s:  unable to find class %s\n", __PRETTY_FUNCTION__, _name);
        return;
    }

    _commonInit (aClass);
}

JavaClass::JavaClass (jobject aClass)
{
    _name = 0;
    _commonInit (aClass);
}

static CFMutableDictionaryRef classesByName = 0;

static void _createClassesByNameIfNecessary()
{
    if (classesByName == 0)
        classesByName = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
}

JavaClass *JavaClass::classForName (const char *name)
{
    _createClassesByNameIfNecessary();
    
    CFStringRef stringName = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
    JavaClass *aClass = (JavaClass *)CFDictionaryGetValue(classesByName, stringName);
    if (aClass == NULL) {
        aClass = new JavaClass (name);
        CFDictionaryAddValue (classesByName, stringName, aClass);
    }
    CFRelease (stringName);

    return aClass;
}

void JavaClass::setClassName (const char *n)
{
    free ((void *)_name);
    _name = strdup(n);
}

JavaClass *JavaClass::classForInstance (jobject instance)
{
    _createClassesByNameIfNecessary();
    
    jobject classOfInstance = callJNIObjectMethod(instance, "getClass", "()Ljava/lang/Class;");
    jstring className = (jstring)callJNIObjectMethod(classOfInstance, "getName", "()Ljava/lang/String;");

    const char *classNameC = getCharactersFromJString (className);
    
    CFStringRef stringName = CFStringCreateWithCString(NULL, classNameC, kCFStringEncodingASCII);
    JavaClass *aClass = (JavaClass *)CFDictionaryGetValue(classesByName, stringName);
    if (aClass == NULL) {
        aClass = new JavaClass (classOfInstance);
        aClass->setClassName(classNameC);
        CFDictionaryAddValue (classesByName, stringName, aClass);
    }
    CFRelease (stringName);
    
    releaseCharactersForJString(className, classNameC);

    return aClass;
}

Method *JavaClass::methodNamed(const char *name) const 
{
    CFStringRef methodName = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
    Method *aMethod = (Method *)CFDictionaryGetValue(_methods, methodName);
    CFRelease (methodName);
    return aMethod;
};

Field *JavaClass::fieldNamed(const char *name) const
{
    CFStringRef fieldName = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
    Field *aField = (Field *)CFDictionaryGetValue(_fields, fieldName);
    CFRelease (fieldName);
    return aField;
};

