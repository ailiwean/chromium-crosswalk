// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sync/api/attachments/attachment_metadata.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace syncer {

class AttachmentMetadataTest : public testing::Test {};

TEST_F(AttachmentMetadataTest, Create) {
  AttachmentId id = AttachmentId::Create();
  size_t size = 42;
  AttachmentMetadata metadata(id, size);
  EXPECT_EQ(metadata.GetId(), id);
  EXPECT_EQ(metadata.GetSize(), size);
}

}  // namespace syncer
