/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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

#include "config.h"
#include "IDBLevelDBCoding.h"

#if ENABLE(INDEXED_DATABASE)
#if ENABLE(LEVELDB)

#include "IDBKey.h"
#include <gtest/gtest.h>
#include <wtf/Vector.h>

using namespace WebCore;
using namespace IDBLevelDBCoding;

namespace {

TEST(IDBLevelDBCodingTest, EncodeByte)
{
    Vector<char> expected;
    expected.append(0);
    unsigned char c;

    c = 0;
    expected[0] = c;
    EXPECT_EQ(expected, encodeByte(c));

    c = 1;
    expected[0] = c;
    EXPECT_EQ(expected, encodeByte(c));

    c = 255;
    expected[0] = c;
    EXPECT_EQ(expected, encodeByte(c));
}

TEST(IDBLevelDBCodingTest, MaxIDBKey)
{
    Vector<char> maxKey = maxIDBKey();

    Vector<char> minKey = minIDBKey();
    Vector<char> stringKey = encodeIDBKey(*IDBKey::createString("Hello world"));
    Vector<char> numberKey = encodeIDBKey(*IDBKey::createNumber(3.14));
    Vector<char> dateKey = encodeIDBKey(*IDBKey::createDate(1000000));

    EXPECT_GT(compareEncodedIDBKeys(maxKey, minKey), 0);
    EXPECT_GT(compareEncodedIDBKeys(maxKey, stringKey), 0);
    EXPECT_GT(compareEncodedIDBKeys(maxKey, numberKey), 0);
    EXPECT_GT(compareEncodedIDBKeys(maxKey, dateKey), 0);
}

TEST(IDBLevelDBCodingTest, MinIDBKey)
{
    Vector<char> minKey = minIDBKey();

    Vector<char> maxKey = maxIDBKey();
    Vector<char> stringKey = encodeIDBKey(*IDBKey::createString("Hello world"));
    Vector<char> numberKey = encodeIDBKey(*IDBKey::createNumber(3.14));
    Vector<char> dateKey = encodeIDBKey(*IDBKey::createDate(1000000));

    EXPECT_LT(compareEncodedIDBKeys(minKey, maxKey), 0);
    EXPECT_LT(compareEncodedIDBKeys(minKey, stringKey), 0);
    EXPECT_LT(compareEncodedIDBKeys(minKey, numberKey), 0);
    EXPECT_LT(compareEncodedIDBKeys(minKey, dateKey), 0);
}

TEST(IDBLevelDBCodingTest, EncodeInt)
{
    EXPECT_EQ(static_cast<size_t>(1), encodeInt(0).size());
    EXPECT_EQ(static_cast<size_t>(1), encodeInt(1).size());
    EXPECT_EQ(static_cast<size_t>(1), encodeInt(255).size());
    EXPECT_EQ(static_cast<size_t>(2), encodeInt(256).size());
    EXPECT_EQ(static_cast<size_t>(4), encodeInt(0xffffffff).size());
}

TEST(IDBLevelDBCodingTest, DecodeInt)
{
    Vector<int64_t> testCases;
    testCases.append(0);
    testCases.append(1);
    testCases.append(255);
    testCases.append(256);
    testCases.append(65535);
    testCases.append(655536);
    testCases.append(7711192431755665792ll);
    testCases.append(0x7fffffffffffffffll);

    for (size_t i = 0; i < testCases.size(); ++i) {
        int64_t n = testCases[i];
        Vector<char> v = encodeInt(n);
        EXPECT_EQ(n, decodeInt(v.data(), v.data() + v.size()));
    }
}

TEST(IDBLevelDBCodingTest, EncodeVarInt)
{
    EXPECT_EQ(static_cast<size_t>(1), encodeVarInt(0).size());
    EXPECT_EQ(static_cast<size_t>(1), encodeVarInt(1).size());
    EXPECT_EQ(static_cast<size_t>(2), encodeVarInt(255).size());
    EXPECT_EQ(static_cast<size_t>(2), encodeVarInt(256).size());
    EXPECT_EQ(static_cast<size_t>(5), encodeVarInt(0xffffffff).size());
}

TEST(IDBLevelDBCodingTest, DecodeVarInt)
{
    Vector<int64_t> testCases;
    testCases.append(0);
    testCases.append(1);
    testCases.append(255);
    testCases.append(256);
    testCases.append(65535);
    testCases.append(655536);
    testCases.append(7711192431755665792ll);
    testCases.append(0x7fffffffffffffffll);

    for (size_t i = 0; i < testCases.size(); ++i) {
        int64_t n = testCases[i];
        Vector<char> v = encodeVarInt(n);

        int64_t res;
        const char* p = decodeVarInt(v.data(), v.data() + v.size(), res);
        EXPECT_EQ(n, res);
        EXPECT_EQ(v.data() + v.size(), p);

        p = decodeVarInt(v.data(), v.data() + v.size() - 1, res);
        EXPECT_EQ(0, p);
        p = decodeVarInt(v.data(), v.data(), res);
        EXPECT_EQ(0, p);
    }
}

TEST(IDBLevelDBCodingTest, EncodeString)
{
    const UChar testStringA[] = {'f', 'o', 'o', '\0'};
    const UChar testStringB[] = {0xdead, 0xbeef, '\0'};

    EXPECT_EQ(static_cast<size_t>(0), encodeString(String("")).size());
    EXPECT_EQ(static_cast<size_t>(2), encodeString(String("a")).size());
    EXPECT_EQ(static_cast<size_t>(6), encodeString(String("foo")).size());
    EXPECT_EQ(static_cast<size_t>(6), encodeString(String(testStringA)).size());
    EXPECT_EQ(static_cast<size_t>(4), encodeString(String(testStringB)).size());
}

TEST(IDBLevelDBCodingTest, DecodeString)
{
    const UChar testStringA[] = {'f', 'o', 'o', '\0'};
    const UChar testStringB[] = {0xdead, 0xbeef, '\0'};
    Vector<char> v;

    v = encodeString(String(""));
    EXPECT_EQ(String(""), decodeString(v.data(), v.data() + v.size()));

    v = encodeString(String("a"));
    EXPECT_EQ(String("a"), decodeString(v.data(), v.data() + v.size()));

    v = encodeString(String("foo"));
    EXPECT_EQ(String("foo"), decodeString(v.data(), v.data() + v.size()));

    v = encodeString(String(testStringA));
    EXPECT_EQ(testStringA, decodeString(v.data(), v.data() + v.size()));

    v = encodeString(String(testStringB));
    EXPECT_EQ(testStringB, decodeString(v.data(), v.data() + v.size()));
}

TEST(IDBLevelDBCodingTest, EncodeStringWithLength)
{
    const UChar testStringA[] = {'f', 'o', 'o', '\0'};
    const UChar testStringB[] = {0xdead, 0xbeef, '\0'};

    EXPECT_EQ(static_cast<size_t>(1), encodeStringWithLength(String("")).size());
    EXPECT_EQ(static_cast<size_t>(3), encodeStringWithLength(String("a")).size());
    EXPECT_EQ(static_cast<size_t>(7), encodeStringWithLength(String(testStringA)).size());
    EXPECT_EQ(static_cast<size_t>(5), encodeStringWithLength(String(testStringB)).size());
}

TEST(IDBLevelDBCodingTest, DecodeStringWithLength)
{
    const UChar testStringA[] = {'f', 'o', 'o', '\0'};
    const UChar testStringB[] = {0xdead, 0xbeef, '\0'};

    const int kLongStringLen = 1234;
    UChar longString[kLongStringLen + 1];
    for (int i = 0; i < kLongStringLen; i++)
        longString[i] = i;
    longString[kLongStringLen] = 0;

    Vector<String> testCases;
    testCases.append(String(""));
    testCases.append(String("a"));
    testCases.append(String("foo"));
    testCases.append(String(testStringA));
    testCases.append(String(testStringB));
    testCases.append(String(longString));

    for (size_t i = 0; i < testCases.size(); ++i) {
        String s = testCases[i];
        Vector<char> v = encodeStringWithLength(s);
        String res;
        const char* p = decodeStringWithLength(v.data(), v.data() + v.size(), res);
        EXPECT_EQ(s, res);
        EXPECT_EQ(v.data() + v.size(), p);

        EXPECT_EQ(0, decodeStringWithLength(v.data(), v.data() + v.size() - 1, res));
        EXPECT_EQ(0, decodeStringWithLength(v.data(), v.data(), res));
    }
}

TEST(IDBLevelDBCodingTest, EncodeDouble)
{
    EXPECT_EQ(static_cast<size_t>(8), encodeDouble(0).size());
    EXPECT_EQ(static_cast<size_t>(8), encodeDouble(3.14).size());
}

TEST(IDBLevelDBCodingTest, DecodeDouble)
{
    Vector<char> v;
    const char* p;
    double d;

    v = encodeDouble(3.14);
    p = decodeDouble(v.data(), v.data() + v.size(), &d);
    EXPECT_EQ(3.14, d);
    EXPECT_EQ(v.data() + v.size(), p);

    v = encodeDouble(-3.14);
    p = decodeDouble(v.data(), v.data() + v.size(), &d);
    EXPECT_EQ(-3.14, d);
    EXPECT_EQ(v.data() + v.size(), p);

    v = encodeDouble(3.14);
    p = decodeDouble(v.data(), v.data() + v.size() - 1, &d);
    EXPECT_EQ(0, p);
}

TEST(IDBLevelDBCodingTest, EncodeDecodeIDBKey)
{
    RefPtr<IDBKey> expectedKey;
    RefPtr<IDBKey> decodedKey;
    Vector<char> v;
    const char* p;

    expectedKey = IDBKey::createNumber(1234);
    v = encodeIDBKey(*expectedKey);
    p = decodeIDBKey(v.data(), v.data() + v.size(), decodedKey);
    EXPECT_TRUE(decodedKey->isEqual(expectedKey.get()));
    EXPECT_EQ(v.data() + v.size(), p);
    EXPECT_EQ(0, decodeIDBKey(v.data(), v.data() + v.size() - 1, decodedKey));

    expectedKey = IDBKey::createString("Hello World!");
    v = encodeIDBKey(*expectedKey);
    p = decodeIDBKey(v.data(), v.data() + v.size(), decodedKey);
    EXPECT_TRUE(decodedKey->isEqual(expectedKey.get()));
    EXPECT_EQ(v.data() + v.size(), p);
    EXPECT_EQ(0, decodeIDBKey(v.data(), v.data() + v.size() - 1, decodedKey));

    expectedKey = IDBKey::createDate(7890);
    v = encodeIDBKey(*expectedKey);
    p = decodeIDBKey(v.data(), v.data() + v.size(), decodedKey);
    EXPECT_TRUE(decodedKey->isEqual(expectedKey.get()));
    EXPECT_EQ(v.data() + v.size(), p);
    EXPECT_EQ(0, decodeIDBKey(v.data(), v.data() + v.size() - 1, decodedKey));
}

TEST(IDBLevelDBCodingTest, ExtractAndCompareIDBKeys)
{
    Vector<RefPtr<IDBKey> > keys;

    keys.append(IDBKey::createNumber(-10));
    keys.append(IDBKey::createNumber(0));
    keys.append(IDBKey::createNumber(3.14));

    keys.append(IDBKey::createDate(0));
    keys.append(IDBKey::createDate(100));
    keys.append(IDBKey::createDate(100000));

    keys.append(IDBKey::createString(""));
    keys.append(IDBKey::createString("a"));
    keys.append(IDBKey::createString("b"));
    keys.append(IDBKey::createString("baaa"));

    for (size_t i = 0; i < keys.size() - 1; i++) {
        RefPtr<IDBKey> keyA = keys[i];
        RefPtr<IDBKey> keyB = keys[i + 1];

        EXPECT_TRUE(keyA->isLessThan(keyB.get()));

        Vector<char> encodedA = encodeIDBKey(*keyA);
        EXPECT_TRUE(encodedA.size());
        Vector<char> encodedB = encodeIDBKey(*keyB);
        EXPECT_TRUE(encodedB.size());

        Vector<char> extractedA;
        Vector<char> extractedB;

        const char* p = extractEncodedIDBKey(encodedA.data(), encodedA.data() + encodedA.size(), &extractedA);
        EXPECT_EQ(encodedA.data() + encodedA.size(), p);
        EXPECT_EQ(encodedA, extractedA);

        const char* q = extractEncodedIDBKey(encodedB.data(), encodedB.data() + encodedB.size(), &extractedB);
        EXPECT_EQ(encodedB.data() + encodedB.size(), q);
        EXPECT_EQ(encodedB, extractedB);

        EXPECT_LT(compareEncodedIDBKeys(extractedA, extractedB), 0);
        EXPECT_GT(compareEncodedIDBKeys(extractedB, extractedA), 0);
        EXPECT_EQ(compareEncodedIDBKeys(extractedA, extractedA), 0);
        EXPECT_EQ(compareEncodedIDBKeys(extractedB, extractedB), 0);

        EXPECT_EQ(0, extractEncodedIDBKey(encodedA.data(), encodedA.data() + encodedA.size() - 1, &extractedA));
    }
}

} // namespace

#endif // ENABLE(LEVELDB)
#endif // ENABLE(INDEXED_DATABASE)
