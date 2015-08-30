// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "base/test/test_reg_util_win.h"
#include "base/win/registry.h"
#include "chrome/installer/util/delete_reg_value_work_item.h"
#include "chrome/installer/util/work_item.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::win::RegKey;

namespace {

const wchar_t kTestKey[] = L"DeleteRegValueWorkItemTest";
const wchar_t kNameStr[] = L"name_str";
const wchar_t kNameDword[] = L"name_dword";

class DeleteRegValueWorkItemTest : public testing::Test {
 protected:
  DeleteRegValueWorkItemTest() {}

  void SetUp() override {
    registry_override_manager_.OverrideRegistry(HKEY_CURRENT_USER);
  }

 private:
  registry_util::RegistryOverrideManager registry_override_manager_;

  DISALLOW_COPY_AND_ASSIGN(DeleteRegValueWorkItemTest);
};

}  // namespace

// Delete a value. The value should get deleted after Do() and should be
// recreated after Rollback().
TEST_F(DeleteRegValueWorkItemTest, DeleteExistingValue) {
  RegKey key;
  ASSERT_EQ(ERROR_SUCCESS,
            key.Create(HKEY_CURRENT_USER, kTestKey, KEY_READ | KEY_WRITE));
  const base::string16 data_str(L"data_111");
  ASSERT_EQ(ERROR_SUCCESS, key.WriteValue(kNameStr, data_str.c_str()));
  const DWORD data_dword = 100;
  ASSERT_EQ(ERROR_SUCCESS, key.WriteValue(kNameDword, data_dword));

  static const wchar_t kNameEmpty[](L"name_empty");
  ASSERT_EQ(ERROR_SUCCESS,
            RegSetValueEx(key.Handle(), kNameEmpty, NULL, REG_SZ, NULL, 0));

  scoped_ptr<DeleteRegValueWorkItem> work_item1(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameStr));
  scoped_ptr<DeleteRegValueWorkItem> work_item2(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameDword));
  scoped_ptr<DeleteRegValueWorkItem> work_item3(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameEmpty));

  EXPECT_TRUE(key.HasValue(kNameStr));
  EXPECT_TRUE(key.HasValue(kNameDword));
  EXPECT_TRUE(key.HasValue(kNameEmpty));

  EXPECT_TRUE(work_item1->Do());
  EXPECT_TRUE(work_item2->Do());
  EXPECT_TRUE(work_item3->Do());

  EXPECT_FALSE(key.HasValue(kNameStr));
  EXPECT_FALSE(key.HasValue(kNameDword));
  EXPECT_FALSE(key.HasValue(kNameEmpty));

  work_item1->Rollback();
  work_item2->Rollback();
  work_item3->Rollback();

  EXPECT_TRUE(key.HasValue(kNameStr));
  EXPECT_TRUE(key.HasValue(kNameDword));
  EXPECT_TRUE(key.HasValue(kNameEmpty));

  base::string16 read_str;
  DWORD read_dword;
  EXPECT_EQ(ERROR_SUCCESS, key.ReadValue(kNameStr, &read_str));
  EXPECT_EQ(ERROR_SUCCESS, key.ReadValueDW(kNameDword, &read_dword));
  EXPECT_EQ(read_str, data_str);
  EXPECT_EQ(read_dword, data_dword);

  // Verify empty value.
  DWORD type = 0;
  DWORD size = 0;
  EXPECT_EQ(ERROR_SUCCESS, key.ReadValue(kNameEmpty, NULL, &size, &type));
  EXPECT_EQ(REG_SZ, type);
  EXPECT_EQ(0, size);
}

// Try deleting a value that doesn't exist.
TEST_F(DeleteRegValueWorkItemTest, DeleteNonExistentValue) {
  RegKey key;
  ASSERT_EQ(ERROR_SUCCESS,
            key.Create(HKEY_CURRENT_USER, kTestKey, KEY_READ | KEY_WRITE));
  EXPECT_FALSE(key.HasValue(kNameStr));
  EXPECT_FALSE(key.HasValue(kNameDword));

  scoped_ptr<DeleteRegValueWorkItem> work_item1(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameStr));
  scoped_ptr<DeleteRegValueWorkItem> work_item2(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameDword));

  EXPECT_TRUE(work_item1->Do());
  EXPECT_TRUE(work_item2->Do());

  EXPECT_FALSE(key.HasValue(kNameStr));
  EXPECT_FALSE(key.HasValue(kNameDword));

  work_item1->Rollback();
  work_item2->Rollback();

  EXPECT_FALSE(key.HasValue(kNameStr));
  EXPECT_FALSE(key.HasValue(kNameDword));
}

// Try deleting a value whose key doesn't even exist.
TEST_F(DeleteRegValueWorkItemTest, DeleteValueInNonExistentKey) {
  RegKey key;
  // Confirm the key doesn't exist.
  ASSERT_EQ(ERROR_FILE_NOT_FOUND,
            key.Open(HKEY_CURRENT_USER, kTestKey, KEY_READ));

  scoped_ptr<DeleteRegValueWorkItem> work_item1(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameStr));
  scoped_ptr<DeleteRegValueWorkItem> work_item2(
      WorkItem::CreateDeleteRegValueWorkItem(
          HKEY_CURRENT_USER, kTestKey, WorkItem::kWow64Default, kNameDword));

  EXPECT_TRUE(work_item1->Do());
  EXPECT_TRUE(work_item2->Do());

  EXPECT_EQ(ERROR_FILE_NOT_FOUND,
            key.Open(HKEY_CURRENT_USER, kTestKey, KEY_READ));

  work_item1->Rollback();
  work_item2->Rollback();

  EXPECT_EQ(ERROR_FILE_NOT_FOUND,
            key.Open(HKEY_CURRENT_USER, kTestKey, KEY_READ));
}
