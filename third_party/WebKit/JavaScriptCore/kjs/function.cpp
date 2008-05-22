// -*- c-basic-offset: 2 -*-
/*
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *  Copyright (C) 2007 Maks Orlovich
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "function.h"

#include "ExecState.h"
#include "JSActivation.h"
#include "JSGlobalObject.h"
#include "Machine.h"
#include "Parser.h"
#include "PropertyNameArray.h"
#include "debugger.h"
#include "dtoa.h"
#include "function_object.h"
#include "internal.h"
#include "lexer.h"
#include "nodes.h"
#include "operations.h"
#include "scope_chain_mark.h"
#include <errno.h>
#include <profiler/Profiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wtf/ASCIICType.h>
#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>
#include <wtf/unicode/UTF8.h>

using namespace WTF;
using namespace Unicode;

namespace KJS {

// ----------------------------- FunctionImp ----------------------------------

const ClassInfo FunctionImp::info = { "Function", &InternalFunctionImp::info, 0, 0 };

FunctionImp::FunctionImp(ExecState* exec, const Identifier& name, FunctionBodyNode* b, ScopeChainNode* scopeChain)
  : InternalFunctionImp(exec->lexicalGlobalObject()->functionPrototype(), name)
  , body(b)
  , _scope(scopeChain)
{
}

void FunctionImp::mark()
{
    InternalFunctionImp::mark();
    body->mark();
    _scope.mark();
}

CallType FunctionImp::getCallData(CallData& callData)
{
    callData.js.functionBody = body.get();
    callData.js.scopeChain = _scope.node();
    return CallTypeJS;
}

JSValue* FunctionImp::callAsFunction(ExecState* exec, JSObject* thisObj, const List& args)
{
    JSValue* exception = 0;
    RegisterFileStack* stack = &exec->dynamicGlobalObject()->registerFileStack();
    RegisterFile* current = stack->current();
    if (!current->safeForReentry()) {
        stack->pushFunctionRegisterFile();
        JSValue* result = machine().execute(body.get(), exec, this, thisObj, args, stack, _scope.node(), &exception);
        stack->popFunctionRegisterFile();
        exec->setException(exception);
        return result;
    } else {
        JSValue* result = machine().execute(body.get(), exec, this, thisObj, args, stack, _scope.node(), &exception);
        current->setSafeForReentry(true);
        exec->setException(exception);
        return result;
    }
}

JSValue* FunctionImp::argumentsGetter(ExecState* exec, JSObject*, const Identifier&, const PropertySlot& slot)
{
    FunctionImp* thisObj = static_cast<FunctionImp*>(slot.slotBase());
    ASSERT(exec->machine());
    return exec->machine()->retrieveArguments(exec, thisObj);
}

JSValue* FunctionImp::callerGetter(ExecState* exec, JSObject*, const Identifier&, const PropertySlot& slot)
{
    FunctionImp* thisObj = static_cast<FunctionImp*>(slot.slotBase());
    ASSERT(exec->machine());
    return exec->machine()->retrieveCaller(exec, thisObj);
}

JSValue* FunctionImp::lengthGetter(ExecState*, JSObject*, const Identifier&, const PropertySlot& slot)
{
    FunctionImp* thisObj = static_cast<FunctionImp*>(slot.slotBase());
    return jsNumber(thisObj->body->parameters().size());
}

bool FunctionImp::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    if (propertyName == exec->propertyNames().arguments) {
        slot.setCustom(this, argumentsGetter);
        return true;
    }

    if (propertyName == exec->propertyNames().length) {
        slot.setCustom(this, lengthGetter);
        return true;
    }

    if (propertyName == exec->propertyNames().caller) {
        slot.setCustom(this, callerGetter);
        return true;
    }

    return InternalFunctionImp::getOwnPropertySlot(exec, propertyName, slot);
}

void FunctionImp::put(ExecState* exec, const Identifier& propertyName, JSValue* value)
{
    if (propertyName == exec->propertyNames().arguments || propertyName == exec->propertyNames().length)
        return;
    InternalFunctionImp::put(exec, propertyName, value);
}

bool FunctionImp::deleteProperty(ExecState* exec, const Identifier& propertyName)
{
    if (propertyName == exec->propertyNames().arguments || propertyName == exec->propertyNames().length)
        return false;
    return InternalFunctionImp::deleteProperty(exec, propertyName);
}

/* Returns the parameter name corresponding to the given index. eg:
 * function f1(x, y, z): getParameterName(0) --> x
 *
 * If a name appears more than once, only the last index at which
 * it appears associates with it. eg:
 * function f2(x, x): getParameterName(0) --> null
 */
Identifier FunctionImp::getParameterName(int index)
{
    Vector<Identifier>& parameters = body->parameters();

    if (static_cast<size_t>(index) >= body->parameters().size())
        return CommonIdentifiers::shared()->nullIdentifier;
  
    Identifier name = parameters[index];

    // Are there any subsequent parameters with the same name?
    size_t size = parameters.size();
    for (size_t i = index + 1; i < size; ++i)
        if (parameters[i] == name)
            return CommonIdentifiers::shared()->nullIdentifier;

    return name;
}

// ECMA 13.2.2 [[Construct]]
ConstructType FunctionImp::getConstructData(ConstructData& constructData)
{
    constructData.js.functionBody = body.get();
    constructData.js.scopeChain = _scope.node();
    return ConstructTypeJS;
}

JSObject* FunctionImp::construct(ExecState* exec, const List& args)
{
    JSObject* proto;
    JSValue* p = get(exec, exec->propertyNames().prototype);
    if (p->isObject())
        proto = static_cast<JSObject*>(p);
    else
        proto = exec->lexicalGlobalObject()->objectPrototype();

    JSObject* thisObj = new JSObject(proto);

    JSValue* exception = 0;
    JSValue* result = machine().execute(body.get(), exec, this, thisObj, args, &exec->dynamicGlobalObject()->registerFileStack(), _scope.node(), &exception);
    if (exception) {
        exec->setException(exception);
        return thisObj;
    }

    if (result->isObject())
        return static_cast<JSObject*>(result);
    return thisObj;
}

// ------------------------------ IndexToNameMap ---------------------------------

// We map indexes in the arguments array to their corresponding argument names. 
// Example: function f(x, y, z): arguments[0] = x, so we map 0 to Identifier("x"). 

// Once we have an argument name, we can get and set the argument's value in the 
// activation object.

// We use Identifier::null to indicate that a given argument's value
// isn't stored in the activation object.

IndexToNameMap::IndexToNameMap(FunctionImp* func, const List& args)
{
  _map = new Identifier[args.size()];
  this->size = args.size();
  
  unsigned i = 0;
  List::const_iterator end = args.end();
  for (List::const_iterator it = args.begin(); it != end; ++i, ++it)
    _map[i] = func->getParameterName(i); // null if there is no corresponding parameter
}

IndexToNameMap::~IndexToNameMap()
{
  delete [] _map;
}

bool IndexToNameMap::isMapped(const Identifier& index) const
{
  bool indexIsNumber;
  unsigned indexAsNumber = index.toStrictUInt32(&indexIsNumber);
  
  if (!indexIsNumber)
    return false;
  
  if (indexAsNumber >= size)
    return false;

  if (_map[indexAsNumber].isNull())
    return false;
  
  return true;
}

void IndexToNameMap::unMap(const Identifier& index)
{
  bool indexIsNumber;
  unsigned indexAsNumber = index.toStrictUInt32(&indexIsNumber);

  ASSERT(indexIsNumber && indexAsNumber < size);
  
  _map[indexAsNumber] = CommonIdentifiers::shared()->nullIdentifier;
}

Identifier& IndexToNameMap::operator[](const Identifier& index)
{
  bool indexIsNumber;
  unsigned indexAsNumber = index.toStrictUInt32(&indexIsNumber);

  ASSERT(indexIsNumber && indexAsNumber < size);
  
  return _map[indexAsNumber];
}

// ------------------------------ Arguments ---------------------------------

const ClassInfo Arguments::info = { "Arguments", 0, 0, 0 };

// ECMA 10.1.8
Arguments::Arguments(ExecState* exec, FunctionImp* func, const List& args, JSActivation* act)
    : JSObject(exec->lexicalGlobalObject()->objectPrototype())
    , _activationObject(act)
    , indexToNameMap(func, args)
{
    putDirect(exec->propertyNames().callee, func, DontEnum);
    putDirect(exec->propertyNames().length, args.size(), DontEnum);
  
    int i = 0;
    List::const_iterator end = args.end();
    for (List::const_iterator it = args.begin(); it != end; ++it, ++i) {
        Identifier name = Identifier::from(i);
        if (!indexToNameMap.isMapped(name))
            putDirect(name, *it, DontEnum);
    }
}

void Arguments::mark() 
{
  JSObject::mark();
  if (_activationObject && !_activationObject->marked())
    _activationObject->mark();
}

JSValue* Arguments::mappedIndexGetter(ExecState* exec, JSObject*, const Identifier& propertyName, const PropertySlot& slot)
{
  Arguments* thisObj = static_cast<Arguments*>(slot.slotBase());
  return thisObj->_activationObject->get(exec, thisObj->indexToNameMap[propertyName]);
}

bool Arguments::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
  if (indexToNameMap.isMapped(propertyName)) {
    slot.setCustom(this, mappedIndexGetter);
    return true;
  }

  return JSObject::getOwnPropertySlot(exec, propertyName, slot);
}

void Arguments::put(ExecState* exec, const Identifier& propertyName, JSValue* value)
{
    if (indexToNameMap.isMapped(propertyName))
        _activationObject->put(exec, indexToNameMap[propertyName], value);
    else
        JSObject::put(exec, propertyName, value);
}

bool Arguments::deleteProperty(ExecState* exec, const Identifier& propertyName) 
{
  if (indexToNameMap.isMapped(propertyName)) {
    indexToNameMap.unMap(propertyName);
    return true;
  } else {
    return JSObject::deleteProperty(exec, propertyName);
  }
}

// ------------------------------ Global Functions -----------------------------------

static JSValue* encode(ExecState* exec, const List& args, const char* do_not_escape)
{
  UString r = "", s, str = args[0]->toString(exec);
  CString cstr = str.UTF8String(true);
  if (!cstr.c_str())
    return throwError(exec, URIError, "String contained an illegal UTF-16 sequence.");
  const char* p = cstr.c_str();
  for (size_t k = 0; k < cstr.size(); k++, p++) {
    char c = *p;
    if (c && strchr(do_not_escape, c)) {
      r.append(c);
    } else {
      char tmp[4];
      sprintf(tmp, "%%%02X", (unsigned char)c);
      r += tmp;
    }
  }
  return jsString(r);
}

static JSValue* decode(ExecState* exec, const List& args, const char* do_not_unescape, bool strict)
{
  UString s = "", str = args[0]->toString(exec);
  int k = 0, len = str.size();
  const UChar* d = str.data();
  UChar u = 0;
  while (k < len) {
    const UChar* p = d + k;
    UChar c = *p;
    if (c == '%') {
      int charLen = 0;
      if (k <= len - 3 && isASCIIHexDigit(p[1]) && isASCIIHexDigit(p[2])) {
        const char b0 = Lexer::convertHex(p[1], p[2]);
        const int sequenceLen = UTF8SequenceLength(b0);
        if (sequenceLen != 0 && k <= len - sequenceLen * 3) {
          charLen = sequenceLen * 3;
          char sequence[5];
          sequence[0] = b0;
          for (int i = 1; i < sequenceLen; ++i) {
            const UChar* q = p + i * 3;
            if (q[0] == '%' && isASCIIHexDigit(q[1]) && isASCIIHexDigit(q[2]))
              sequence[i] = Lexer::convertHex(q[1], q[2]);
            else {
              charLen = 0;
              break;
            }
          }
          if (charLen != 0) {
            sequence[sequenceLen] = 0;
            const int character = decodeUTF8Sequence(sequence);
            if (character < 0 || character >= 0x110000) {
              charLen = 0;
            } else if (character >= 0x10000) {
              // Convert to surrogate pair.
              s.append(static_cast<UChar>(0xD800 | ((character - 0x10000) >> 10)));
              u = static_cast<UChar>(0xDC00 | ((character - 0x10000) & 0x3FF));
            } else {
              u = static_cast<UChar>(character);
            }
          }
        }
      }
      if (charLen == 0) {
        if (strict)
          return throwError(exec, URIError);
        // The only case where we don't use "strict" mode is the "unescape" function.
        // For that, it's good to support the wonky "%u" syntax for compatibility with WinIE.
        if (k <= len - 6 && p[1] == 'u'
            && isASCIIHexDigit(p[2]) && isASCIIHexDigit(p[3])
            && isASCIIHexDigit(p[4]) && isASCIIHexDigit(p[5])) {
          charLen = 6;
          u = Lexer::convertUnicode(p[2], p[3], p[4], p[5]);
        }
      }
      if (charLen && (u == 0 || u >= 128 || !strchr(do_not_unescape, u))) {
        c = u;
        k += charLen - 1;
      }
    }
    k++;
    s.append(c);
  }
  return jsString(s);
}

static bool isStrWhiteSpace(unsigned short c)
{
    switch (c) {
        case 0x0009:
        case 0x000A:
        case 0x000B:
        case 0x000C:
        case 0x000D:
        case 0x0020:
        case 0x00A0:
        case 0x2028:
        case 0x2029:
            return true;
        default:
            return c > 0xff && isSeparatorSpace(c);
    }
}

static int parseDigit(unsigned short c, int radix)
{
    int digit = -1;

    if (c >= '0' && c <= '9') {
        digit = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        digit = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        digit = c - 'a' + 10;
    }

    if (digit >= radix)
        return -1;
    return digit;
}

double parseIntOverflow(const char* s, int length, int radix)
{
    double number = 0.0;
    double radixMultiplier = 1.0;

    for (const char* p = s + length - 1; p >= s; p--) {
        if (radixMultiplier == Inf) {
            if (*p != '0') {
                number = Inf;
                break;
            }
        } else {
            int digit = parseDigit(*p, radix);
            number += digit * radixMultiplier;
        }

        radixMultiplier *= radix;
    }

    return number;
}

static double parseInt(const UString& s, int radix)
{
    int length = s.size();
    int p = 0;

    while (p < length && isStrWhiteSpace(s[p])) {
        ++p;
    }

    double sign = 1;
    if (p < length) {
        if (s[p] == '+') {
            ++p;
        } else if (s[p] == '-') {
            sign = -1;
            ++p;
        }
    }

    if ((radix == 0 || radix == 16) && length - p >= 2 && s[p] == '0' && (s[p + 1] == 'x' || s[p + 1] == 'X')) {
        radix = 16;
        p += 2;
    } else if (radix == 0) {
        if (p < length && s[p] == '0')
            radix = 8;
        else
            radix = 10;
    }

    if (radix < 2 || radix > 36)
        return NaN;

    int firstDigitPosition = p;
    bool sawDigit = false;
    double number = 0;
    while (p < length) {
        int digit = parseDigit(s[p], radix);
        if (digit == -1)
            break;
        sawDigit = true;
        number *= radix;
        number += digit;
        ++p;
    }

    if (number >= mantissaOverflowLowerBound) {
        if (radix == 10)
            number = strtod(s.substr(firstDigitPosition, p - firstDigitPosition).ascii(), 0);
        else if (radix == 2 || radix == 4 || radix == 8 || radix == 16 || radix == 32)
            number = parseIntOverflow(s.substr(firstDigitPosition, p - firstDigitPosition).ascii(), p - firstDigitPosition, radix);
    }

    if (!sawDigit)
        return NaN;

    return sign * number;
}

static double parseFloat(const UString& s)
{
    // Check for 0x prefix here, because toDouble allows it, but we must treat it as 0.
    // Need to skip any whitespace and then one + or - sign.
    int length = s.size();
    int p = 0;
    while (p < length && isStrWhiteSpace(s[p])) {
        ++p;
    }
    if (p < length && (s[p] == '+' || s[p] == '-')) {
        ++p;
    }
    if (length - p >= 2 && s[p] == '0' && (s[p + 1] == 'x' || s[p + 1] == 'X')) {
        return 0;
    }

    return s.toDouble( true /*tolerant*/, false /* NaN for empty string */ );
}

JSValue* globalFuncEval(ExecState* exec, PrototypeReflexiveFunction* function, JSObject* thisObj, const List& args)
{
    JSGlobalObject* globalObject = thisObj->toGlobalObject(exec);

    if (!globalObject || globalObject->evalFunction() != function)
        return throwError(exec, EvalError, "The \"this\" value passed to eval must be the global object from which eval originated");

    JSValue* x = args[0];
    if (!x->isString())
        return x;
    
    UString s = x->toString(exec);
    
    int sourceId;
    int errLine;
    UString errMsg;

    RefPtr<EvalNode> evalNode = parser().parse<EvalNode>(exec, UString(), 0, UStringSourceProvider::create(s), &sourceId, &errLine, &errMsg);
    
    if (!evalNode)
        return throwError(exec, SyntaxError, errMsg, errLine, sourceId, NULL);

    JSValue* exception = 0;
    JSValue* value = machine().execute(evalNode.get(), exec, thisObj, &exec->dynamicGlobalObject()->registerFileStack(), globalObject->globalScopeChain().node(), &exception);

    if (exception) {
        exec->setException(exception);
        return value;
    }
    
    return value ? value : jsUndefined();
}

JSValue* globalFuncParseInt(ExecState* exec, JSObject*, const List& args)
{
    return jsNumber(parseInt(args[0]->toString(exec), args[1]->toInt32(exec)));
}

JSValue* globalFuncParseFloat(ExecState* exec, JSObject*, const List& args)
{
    return jsNumber(parseFloat(args[0]->toString(exec)));
}

JSValue* globalFuncIsNaN(ExecState* exec, JSObject*, const List& args)
{
    return jsBoolean(isnan(args[0]->toNumber(exec)));
}

JSValue* globalFuncIsFinite(ExecState* exec, JSObject*, const List& args)
{
    double n = args[0]->toNumber(exec);
    return jsBoolean(!isnan(n) && !isinf(n));
}

JSValue* globalFuncDecodeURI(ExecState* exec, JSObject*, const List& args)
{
    static const char do_not_unescape_when_decoding_URI[] =
        "#$&+,/:;=?@";

    return decode(exec, args, do_not_unescape_when_decoding_URI, true);
}

JSValue* globalFuncDecodeURIComponent(ExecState* exec, JSObject*, const List& args)
{
    return decode(exec, args, "", true);
}

JSValue* globalFuncEncodeURI(ExecState* exec, JSObject*, const List& args)
{
    static const char do_not_escape_when_encoding_URI[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "!#$&'()*+,-./:;=?@_~";

    return encode(exec, args, do_not_escape_when_encoding_URI);
}

JSValue* globalFuncEncodeURIComponent(ExecState* exec, JSObject*, const List& args)
{
    static const char do_not_escape_when_encoding_URI_component[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "!'()*-._~";

    return encode(exec, args, do_not_escape_when_encoding_URI_component);
}

JSValue* globalFuncEscape(ExecState* exec, JSObject*, const List& args)
{
    static const char do_not_escape[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "*+-./@_";

    UString r = "", s, str = args[0]->toString(exec);
    const UChar* c = str.data();
    for (int k = 0; k < str.size(); k++, c++) {
        int u = c[0];
        if (u > 255) {
            char tmp[7];
            sprintf(tmp, "%%u%04X", u);
            s = UString(tmp);
        } else if (u != 0 && strchr(do_not_escape, (char)u))
            s = UString(c, 1);
        else {
            char tmp[4];
            sprintf(tmp, "%%%02X", u);
            s = UString(tmp);
        }
        r += s;
    }

    return jsString(r);
}

JSValue* globalFuncUnescape(ExecState* exec, JSObject*, const List& args)
{
    UString s = "", str = args[0]->toString(exec);
    int k = 0, len = str.size();
    while (k < len) {
        const UChar* c = str.data() + k;
        UChar u;
        if (c[0] == '%' && k <= len - 6 && c[1] == 'u') {
            if (Lexer::isHexDigit(c[2]) && Lexer::isHexDigit(c[3]) && Lexer::isHexDigit(c[4]) && Lexer::isHexDigit(c[5])) {
                u = Lexer::convertUnicode(c[2], c[3], c[4], c[5]);
                c = &u;
                k += 5;
            }
        } else if (c[0] == '%' && k <= len - 3 && Lexer::isHexDigit(c[1]) && Lexer::isHexDigit(c[2])) {
            u = UChar(Lexer::convertHex(c[1], c[2]));
            c = &u;
            k += 2;
        }
        k++;
        s += UString(c, 1);
    }

    return jsString(s);
}

#ifndef NDEBUG
JSValue* globalFuncKJSPrint(ExecState* exec, JSObject*, const List& args)
{
    CStringBuffer string;
    args[0]->toString(exec).getCString(string);
    puts(string.data());
    return jsUndefined();
}
#endif

// ------------------------------ PrototypeFunction -------------------------------

PrototypeFunction::PrototypeFunction(ExecState* exec, int len, const Identifier& name, JSMemberFunction function)
    : InternalFunctionImp(exec->lexicalGlobalObject()->functionPrototype(), name)
    , m_function(function)
{
    ASSERT_ARG(function, function);
    putDirect(exec->propertyNames().length, jsNumber(len), DontDelete | ReadOnly | DontEnum);
}

PrototypeFunction::PrototypeFunction(ExecState* exec, FunctionPrototype* functionPrototype, int len, const Identifier& name, JSMemberFunction function)
    : InternalFunctionImp(functionPrototype, name)
    , m_function(function)
{
    ASSERT_ARG(function, function);
    putDirect(exec->propertyNames().length, jsNumber(len), DontDelete | ReadOnly | DontEnum);
}

JSValue* PrototypeFunction::callAsFunction(ExecState* exec, JSObject* thisObj, const List& args)
{
    return m_function(exec, thisObj, args);
}

// ------------------------------ PrototypeReflexiveFunction -------------------------------

PrototypeReflexiveFunction::PrototypeReflexiveFunction(ExecState* exec, FunctionPrototype* functionPrototype, int len, const Identifier& name, JSMemberFunction function, JSGlobalObject* cachedGlobalObject)
    : InternalFunctionImp(functionPrototype, name)
    , m_function(function)
    , m_cachedGlobalObject(cachedGlobalObject)
{
    ASSERT_ARG(function, function);
    ASSERT_ARG(cachedGlobalObject, cachedGlobalObject);
    putDirect(exec->propertyNames().length, jsNumber(len), DontDelete | ReadOnly | DontEnum);
}

JSValue* PrototypeReflexiveFunction::callAsFunction(ExecState* exec, JSObject* thisObj, const List& args)
{
    return m_function(exec, this, thisObj, args);
}

void PrototypeReflexiveFunction::mark()
{
    InternalFunctionImp::mark();
    if (!m_cachedGlobalObject->marked())
        m_cachedGlobalObject->mark();
}

} // namespace KJS
