// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/utility/safe_browsing/mac/read_stream.h"

#include <vector>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/utility/safe_browsing/mac/test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace safe_browsing {
namespace dmg {

struct MemoryReadStreamTest {
  void SetUp() {}

  const char* TestName() {
    return "MemoryReadStream";
  }

  std::vector<uint8_t> data;
};

struct FileReadStreamTest {
  void SetUp() {
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  }

  const char* TestName() {
    return "FileReadStream";
  }

  base::ScopedTempDir temp_dir;
  base::File file;
};

template <typename T>
class ReadStreamTest : public testing::Test {
 protected:
  void SetUp() override {
    test_helper_.SetUp();
  }

  void TearDown() override {
    if (HasFailure())
      ADD_FAILURE() << "Failing type is " << test_helper_.TestName();
  }

  scoped_ptr<ReadStream> CreateStream(size_t data_size);

 private:
  T test_helper_;
};

template <>
scoped_ptr<ReadStream>
ReadStreamTest<MemoryReadStreamTest>::CreateStream(size_t data_size) {
  test_helper_.data.resize(data_size);
  for (size_t i = 0; i < data_size; ++i) {
    test_helper_.data[i] = i % 255;
  }
  return make_scoped_ptr(
      new MemoryReadStream(&test_helper_.data[0], data_size));
}

template <>
scoped_ptr<ReadStream>
ReadStreamTest<FileReadStreamTest>::CreateStream(size_t data_size) {
  base::FilePath path = test_helper_.temp_dir.path().Append("stream");
  test_helper_.file.Initialize(path,
      base::File::FLAG_CREATE | base::File::FLAG_WRITE);
  if (!test_helper_.file.IsValid()) {
    ADD_FAILURE() << "Failed to create temp file";
    return nullptr;
  }

  for (size_t i = 0; i < data_size; ++i) {
    char value = i % 255;
    EXPECT_EQ(1, test_helper_.file.WriteAtCurrentPos(&value, 1));
  }

  test_helper_.file.Close();

  test_helper_.file.Initialize(path,
      base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!test_helper_.file.IsValid()) {
    ADD_FAILURE() << "Failed to open temp file";
    return nullptr;
  }

  return make_scoped_ptr(
      new FileReadStream(test_helper_.file.GetPlatformFile()));
}

using ReadStreamImpls = testing::Types<MemoryReadStreamTest,
                                       FileReadStreamTest>;
TYPED_TEST_CASE(ReadStreamTest, ReadStreamImpls);

TYPED_TEST(ReadStreamTest, Read) {
  scoped_ptr<ReadStream> stream = ReadStreamTest<TypeParam>::CreateStream(128);
  uint8_t buf[128] = {0};
  size_t bytes_read;

  {
    EXPECT_TRUE(stream->Read(buf, 4, &bytes_read));
    EXPECT_EQ(4u, bytes_read);
    uint8_t expected[] = { 0, 1, 2, 3, 0, 0, 0 };
    EXPECT_EQ(0, memcmp(expected, buf, sizeof(expected)));
  }

  {
    EXPECT_TRUE(stream->Read(buf, 9, &bytes_read));
    EXPECT_EQ(9u, bytes_read);
    uint8_t expected[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 0, 0 };
    EXPECT_EQ(0, memcmp(expected, buf, sizeof(expected)));
  }
}

TYPED_TEST(ReadStreamTest, ReadAll) {
  const size_t kStreamSize = 4242;
  scoped_ptr<ReadStream> stream =
      ReadStreamTest<TypeParam>::CreateStream(kStreamSize);

  std::vector<uint8_t> data;
  EXPECT_TRUE(test::ReadEntireStream(stream.get(), &data));
  EXPECT_EQ(kStreamSize, data.size());
}

TYPED_TEST(ReadStreamTest, SeekSet) {
  scoped_ptr<ReadStream> stream = ReadStreamTest<TypeParam>::CreateStream(255);
  uint8_t buf[32] = {0};
  size_t bytes_read;

  {
    EXPECT_EQ(250, stream->Seek(250, SEEK_SET));
    EXPECT_TRUE(stream->Read(buf, sizeof(buf), &bytes_read));
    EXPECT_EQ(5u, bytes_read);
    uint8_t expected[] = { 250, 251, 252, 253, 254, 0, 0 };
    EXPECT_EQ(0, memcmp(expected, buf, sizeof(expected)));
  }

  {
    EXPECT_EQ(5, stream->Seek(5, SEEK_SET));
    EXPECT_TRUE(stream->Read(buf, 3, &bytes_read));
    EXPECT_EQ(3u, bytes_read);
    uint8_t expected[] = { 5, 6, 7, 253, 254, 0, 0 };
    EXPECT_EQ(0, memcmp(expected, buf, sizeof(expected)));
  }
}

TYPED_TEST(ReadStreamTest, SeekEnd) {
  scoped_ptr<ReadStream> stream = ReadStreamTest<TypeParam>::CreateStream(32);
  uint8_t buf[32] = {0};
  size_t bytes_read;

  {
    EXPECT_EQ(32, stream->Seek(0, SEEK_END));
    EXPECT_TRUE(stream->Read(buf, sizeof(buf), &bytes_read));
    EXPECT_EQ(0u, bytes_read);
  }

  {
    EXPECT_EQ(28, stream->Seek(-4, SEEK_END));
    EXPECT_TRUE(stream->Read(buf, sizeof(buf), &bytes_read));
    EXPECT_EQ(4u, bytes_read);
    uint8_t expected[] = { 28, 29, 30, 31, 0, 0, 0 };
    EXPECT_EQ(0, memcmp(expected, buf, sizeof(expected)));
  }
}

TYPED_TEST(ReadStreamTest, SeekCur) {
  scoped_ptr<ReadStream> stream = ReadStreamTest<TypeParam>::CreateStream(100);
  uint8_t buf[32] = {0};
  size_t bytes_read;

  {
    EXPECT_EQ(0, stream->Seek(0, SEEK_CUR));
  }

  {
    EXPECT_TRUE(stream->Read(buf, sizeof(buf), &bytes_read));
    EXPECT_EQ(sizeof(buf), bytes_read);
    for (size_t i = 0; i < sizeof(buf); ++i) {
      EXPECT_EQ(i, buf[i]);
    }
    EXPECT_EQ(32, stream->Seek(0, SEEK_CUR));
  }

  {
    EXPECT_EQ(30, stream->Seek(-2, SEEK_CUR));
    EXPECT_TRUE(stream->Read(buf, 3, &bytes_read));
    EXPECT_EQ(3u, bytes_read);
    uint8_t expected[] = { 30, 31, 32 };
    EXPECT_EQ(0, memcmp(expected, buf, sizeof(expected)));
  }

  {
    EXPECT_EQ(100, stream->Seek(0, SEEK_END));
    EXPECT_EQ(100, stream->Seek(0, SEEK_CUR));
  }
}

}  // namespace dmg
}  // namespace safe_browsing
