// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/guid.h"
#include "base/memory/scoped_vector.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/autofill/core/browser/autofill_profile.h"
#include "components/autofill/core/browser/autofill_test_utils.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/credit_card.h"
#include "components/autofill/core/browser/webdata/autofill_change.h"
#include "components/autofill/core/browser/webdata/autofill_entry.h"
#include "components/autofill/core/browser/webdata/autofill_table.h"
#include "components/autofill/core/common/autofill_switches.h"
#include "components/autofill/core/common/autofill_util.h"
#include "components/autofill/core/common/form_field_data.h"
#include "components/os_crypt/os_crypt.h"
#include "components/webdata/common/web_database.h"
#include "sql/statement.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::ASCIIToUTF16;
using base::Time;
using base::TimeDelta;

namespace autofill {

// So we can compare AutofillKeys with EXPECT_EQ().
std::ostream& operator<<(std::ostream& os, const AutofillKey& key) {
  return os << base::UTF16ToASCII(key.name()) << ", "
            << base::UTF16ToASCII(key.value());
}

// So we can compare AutofillChanges with EXPECT_EQ().
std::ostream& operator<<(std::ostream& os, const AutofillChange& change) {
  switch (change.type()) {
    case AutofillChange::ADD: {
      os << "ADD";
      break;
    }
    case AutofillChange::UPDATE: {
      os << "UPDATE";
      break;
    }
    case AutofillChange::REMOVE: {
      os << "REMOVE";
      break;
    }
  }
  return os << " " << change.key();
}

namespace {

typedef std::set<AutofillEntry,
    bool (*)(const AutofillEntry&, const AutofillEntry&)> AutofillEntrySet;
typedef AutofillEntrySet::iterator AutofillEntrySetIterator;

bool CompareAutofillEntries(const AutofillEntry& a, const AutofillEntry& b) {
  int compVal = a.key().name().compare(b.key().name());
  if (compVal != 0)
    return compVal < 0;

  compVal = a.key().value().compare(b.key().value());
  if (compVal != 0)
    return compVal < 0;

  if (a.date_created() != b.date_created())
    return a.date_created() < b.date_created();

  return a.date_last_used() < b.date_last_used();
}

AutofillEntry MakeAutofillEntry(const char* name,
                                const char* value,
                                time_t date_created,
                                time_t date_last_used) {
  if (date_last_used < 0)
    date_last_used = date_created;
  return AutofillEntry(AutofillKey(ASCIIToUTF16(name), ASCIIToUTF16(value)),
                       Time::FromTimeT(date_created),
                       Time::FromTimeT(date_last_used));
}

// Checks |actual| and |expected| contain the same elements.
void CompareAutofillEntrySets(const AutofillEntrySet& actual,
                              const AutofillEntrySet& expected) {
  ASSERT_EQ(expected.size(), actual.size());
  size_t count = 0;
  for (AutofillEntrySet::const_iterator it = actual.begin();
       it != actual.end(); ++it) {
    count += expected.count(*it);
  }
  EXPECT_EQ(actual.size(), count);
}

int GetAutofillEntryCount(const base::string16& name,
                          const base::string16& value,
                          WebDatabase* db) {
  sql::Statement s(db->GetSQLConnection()->GetUniqueStatement(
      "SELECT count FROM autofill WHERE name = ? AND value = ?"));
  s.BindString16(0, name);
  s.BindString16(1, value);
  s.Step();
  return s.ColumnInt(0);
}

}  // namespace

class AutofillTableTest : public testing::Test {
 public:
  AutofillTableTest() {}
  ~AutofillTableTest() override {}

 protected:
  void SetUp() override {
#if defined(OS_MACOSX)
    OSCrypt::UseMockKeychain(true);
#endif
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    file_ = temp_dir_.path().AppendASCII("TestWebDatabase");

    table_.reset(new AutofillTable);
    db_.reset(new WebDatabase);
    db_->AddTable(table_.get());
    ASSERT_EQ(sql::INIT_OK, db_->Init(file_));
  }

  base::FilePath file_;
  base::ScopedTempDir temp_dir_;
  scoped_ptr<AutofillTable> table_;
  scoped_ptr<WebDatabase> db_;

 private:
  DISALLOW_COPY_AND_ASSIGN(AutofillTableTest);
};

TEST_F(AutofillTableTest, Autofill) {
  Time t1 = Time::Now();

  // Simulate the submission of a handful of entries in a field called "Name",
  // some more often than others.
  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  base::Time now = base::Time::Now();
  base::TimeDelta two_seconds = base::TimeDelta::FromSeconds(2);
  EXPECT_FALSE(table_->HasFormElements());
  EXPECT_TRUE(table_->AddFormFieldValue(field, &changes));
  EXPECT_TRUE(table_->HasFormElements());
  std::vector<base::string16> v;
  for (int i = 0; i < 5; ++i) {
    field.value = ASCIIToUTF16("Clark Kent");
    EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                              now + i * two_seconds));
  }
  for (int i = 0; i < 3; ++i) {
    field.value = ASCIIToUTF16("Clark Sutter");
    EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                              now + i * two_seconds));
  }
  for (int i = 0; i < 2; ++i) {
    field.name = ASCIIToUTF16("Favorite Color");
    field.value = ASCIIToUTF16("Green");
    EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                              now + i * two_seconds));
  }

  // We have added the name Clark Kent 5 times, so count should be 5.
  EXPECT_EQ(5, GetAutofillEntryCount(ASCIIToUTF16("Name"),
                                     ASCIIToUTF16("Clark Kent"), db_.get()));

  // Storing in the data base should be case sensitive, so there should be no
  // database entry for clark kent lowercase.
  EXPECT_EQ(0, GetAutofillEntryCount(ASCIIToUTF16("Name"),
                                     ASCIIToUTF16("clark kent"), db_.get()));

  EXPECT_EQ(2, GetAutofillEntryCount(ASCIIToUTF16("Favorite Color"),
                                     ASCIIToUTF16("Green"), db_.get()));

  // This is meant to get a list of suggestions for Name.  The empty prefix
  // in the second argument means it should return all suggestions for a name
  // no matter what they start with.  The order that the names occur in the list
  // should be decreasing order by count.
  EXPECT_TRUE(table_->GetFormValuesForElementName(
      ASCIIToUTF16("Name"), base::string16(), &v, 6));
  EXPECT_EQ(3U, v.size());
  if (v.size() == 3) {
    EXPECT_EQ(ASCIIToUTF16("Clark Kent"), v[0]);
    EXPECT_EQ(ASCIIToUTF16("Clark Sutter"), v[1]);
    EXPECT_EQ(ASCIIToUTF16("Superman"), v[2]);
  }

  // If we query again limiting the list size to 1, we should only get the most
  // frequent entry.
  EXPECT_TRUE(table_->GetFormValuesForElementName(
      ASCIIToUTF16("Name"), base::string16(), &v, 1));
  EXPECT_EQ(1U, v.size());
  if (v.size() == 1) {
    EXPECT_EQ(ASCIIToUTF16("Clark Kent"), v[0]);
  }

  // Querying for suggestions given a prefix is case-insensitive, so the prefix
  // "cLa" shoud get suggestions for both Clarks.
  EXPECT_TRUE(table_->GetFormValuesForElementName(
      ASCIIToUTF16("Name"), ASCIIToUTF16("cLa"), &v, 6));
  EXPECT_EQ(2U, v.size());
  if (v.size() == 2) {
    EXPECT_EQ(ASCIIToUTF16("Clark Kent"), v[0]);
    EXPECT_EQ(ASCIIToUTF16("Clark Sutter"), v[1]);
  }

  // Removing all elements since the beginning of this function should remove
  // everything from the database.
  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(t1, Time(), &changes));

  const AutofillChange kExpectedChanges[] = {
    AutofillChange(AutofillChange::REMOVE,
                   AutofillKey(ASCIIToUTF16("Name"),
                               ASCIIToUTF16("Superman"))),
    AutofillChange(AutofillChange::REMOVE,
                   AutofillKey(ASCIIToUTF16("Name"),
                               ASCIIToUTF16("Clark Kent"))),
    AutofillChange(AutofillChange::REMOVE,
                   AutofillKey(ASCIIToUTF16("Name"),
                               ASCIIToUTF16("Clark Sutter"))),
    AutofillChange(AutofillChange::REMOVE,
                   AutofillKey(ASCIIToUTF16("Favorite Color"),
                               ASCIIToUTF16("Green"))),
  };
  EXPECT_EQ(arraysize(kExpectedChanges), changes.size());
  for (size_t i = 0; i < arraysize(kExpectedChanges); ++i) {
    EXPECT_EQ(kExpectedChanges[i], changes[i]);
  }

  EXPECT_EQ(0, GetAutofillEntryCount(ASCIIToUTF16("Name"),
                                     ASCIIToUTF16("Clark Kent"), db_.get()));

  EXPECT_TRUE(table_->GetFormValuesForElementName(
      ASCIIToUTF16("Name"), base::string16(), &v, 6));
  EXPECT_EQ(0U, v.size());

  // Now add some values with empty strings.
  const base::string16 kValue = ASCIIToUTF16("  toto   ");
  field.name = ASCIIToUTF16("blank");
  field.value = base::string16();
  EXPECT_TRUE(table_->AddFormFieldValue(field, &changes));
  field.name = ASCIIToUTF16("blank");
  field.value = ASCIIToUTF16(" ");
  EXPECT_TRUE(table_->AddFormFieldValue(field, &changes));
  field.name = ASCIIToUTF16("blank");
  field.value = ASCIIToUTF16("      ");
  EXPECT_TRUE(table_->AddFormFieldValue(field, &changes));
  field.name = ASCIIToUTF16("blank");
  field.value = kValue;
  EXPECT_TRUE(table_->AddFormFieldValue(field, &changes));

  // They should be stored normally as the DB layer does not check for empty
  // values.
  v.clear();
  EXPECT_TRUE(table_->GetFormValuesForElementName(
      ASCIIToUTF16("blank"), base::string16(), &v, 10));
  EXPECT_EQ(4U, v.size());
}

TEST_F(AutofillTableTest, Autofill_RemoveBetweenChanges) {
  TimeDelta one_day(TimeDelta::FromDays(1));
  Time t1 = Time::Now();
  Time t2 = t1 + one_day;

  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes, t1));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes, t2));

  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(t1, t2, &changes));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::UPDATE,
                           AutofillKey(ASCIIToUTF16("Name"),
                                       ASCIIToUTF16("Superman"))),
            changes[0]);
  changes.clear();

  EXPECT_TRUE(
      table_->RemoveFormElementsAddedBetween(t2, t2 + one_day, &changes));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::REMOVE,
                           AutofillKey(ASCIIToUTF16("Name"),
                                       ASCIIToUTF16("Superman"))),
            changes[0]);
}

TEST_F(AutofillTableTest, Autofill_AddChanges) {
  TimeDelta one_day(TimeDelta::FromDays(1));
  Time t1 = Time::Now();
  Time t2 = t1 + one_day;

  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes, t1));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::ADD,
                           AutofillKey(ASCIIToUTF16("Name"),
                                       ASCIIToUTF16("Superman"))),
            changes[0]);

  changes.clear();
  EXPECT_TRUE(
      table_->AddFormFieldValueTime(field, &changes, t2));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::UPDATE,
                           AutofillKey(ASCIIToUTF16("Name"),
                                       ASCIIToUTF16("Superman"))),
            changes[0]);
}

TEST_F(AutofillTableTest, Autofill_UpdateOneWithOneTimestamp) {
  AutofillEntry entry(MakeAutofillEntry("foo", "bar", 1, -1));
  std::vector<AutofillEntry> entries;
  entries.push_back(entry);
  ASSERT_TRUE(table_->UpdateAutofillEntries(entries));

  EXPECT_EQ(1, GetAutofillEntryCount(ASCIIToUTF16("foo"), ASCIIToUTF16("bar"),
                                     db_.get()));

  std::vector<AutofillEntry> all_entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&all_entries));
  ASSERT_EQ(1U, all_entries.size());
  EXPECT_EQ(entry, all_entries[0]);
}

TEST_F(AutofillTableTest, Autofill_UpdateOneWithTwoTimestamps) {
  AutofillEntry entry(MakeAutofillEntry("foo", "bar", 1, 2));
  std::vector<AutofillEntry> entries;
  entries.push_back(entry);
  ASSERT_TRUE(table_->UpdateAutofillEntries(entries));

  EXPECT_EQ(2, GetAutofillEntryCount(ASCIIToUTF16("foo"), ASCIIToUTF16("bar"),
                                     db_.get()));

  std::vector<AutofillEntry> all_entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&all_entries));
  ASSERT_EQ(1U, all_entries.size());
  EXPECT_EQ(entry, all_entries[0]);
}

TEST_F(AutofillTableTest, Autofill_GetAutofillTimestamps) {
  AutofillEntry entry(MakeAutofillEntry("foo", "bar", 1, 2));
  std::vector<AutofillEntry> entries;
  entries.push_back(entry);
  ASSERT_TRUE(table_->UpdateAutofillEntries(entries));

  Time date_created, date_last_used;
  ASSERT_TRUE(table_->GetAutofillTimestamps(ASCIIToUTF16("foo"),
                                            ASCIIToUTF16("bar"),
                                            &date_created,
                                            &date_last_used));
  EXPECT_EQ(Time::FromTimeT(1), date_created);
  EXPECT_EQ(Time::FromTimeT(2), date_last_used);
}

TEST_F(AutofillTableTest, Autofill_UpdateTwo) {
  AutofillEntry entry0(MakeAutofillEntry("foo", "bar0", 1, -1));
  AutofillEntry entry1(MakeAutofillEntry("foo", "bar1", 2, 3));
  std::vector<AutofillEntry> entries;
  entries.push_back(entry0);
  entries.push_back(entry1);
  ASSERT_TRUE(table_->UpdateAutofillEntries(entries));

  EXPECT_EQ(1, GetAutofillEntryCount(ASCIIToUTF16("foo"), ASCIIToUTF16("bar0"),
                                     db_.get()));
  EXPECT_EQ(2, GetAutofillEntryCount(ASCIIToUTF16("foo"), ASCIIToUTF16("bar1"),
                                     db_.get()));
}

TEST_F(AutofillTableTest, Autofill_UpdateReplace) {
  AutofillChangeList changes;
  // Add a form field.  This will be replaced.
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValue(field, &changes));

  AutofillEntry entry(MakeAutofillEntry("Name", "Superman", 1, 2));
  std::vector<AutofillEntry> entries;
  entries.push_back(entry);
  ASSERT_TRUE(table_->UpdateAutofillEntries(entries));

  std::vector<AutofillEntry> all_entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&all_entries));
  ASSERT_EQ(1U, all_entries.size());
  EXPECT_EQ(entry, all_entries[0]);
}

TEST_F(AutofillTableTest, Autofill_UpdateDontReplace) {
  Time t = Time::Now();
  AutofillEntry existing(
      MakeAutofillEntry("Name", "Superman", t.ToTimeT(), -1));

  AutofillChangeList changes;
  // Add a form field.  This will NOT be replaced.
  FormFieldData field;
  field.name = existing.key().name();
  field.value = existing.key().value();
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes, t));
  AutofillEntry entry(MakeAutofillEntry("Name", "Clark Kent", 1, 2));
  std::vector<AutofillEntry> entries;
  entries.push_back(entry);
  ASSERT_TRUE(table_->UpdateAutofillEntries(entries));

  std::vector<AutofillEntry> all_entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&all_entries));
  ASSERT_EQ(2U, all_entries.size());
  AutofillEntrySet expected_entries(all_entries.begin(),
                                    all_entries.end(),
                                    CompareAutofillEntries);
  EXPECT_EQ(1U, expected_entries.count(existing));
  EXPECT_EQ(1U, expected_entries.count(entry));
}

TEST_F(AutofillTableTest, Autofill_AddFormFieldValues) {
  Time t = Time::Now();

  // Add multiple values for "firstname" and "lastname" names.  Test that only
  // first value of each gets added. Related to security issue:
  // http://crbug.com/51727.
  std::vector<FormFieldData> elements;
  FormFieldData field;
  field.name = ASCIIToUTF16("firstname");
  field.value = ASCIIToUTF16("Joe");
  elements.push_back(field);

  field.name = ASCIIToUTF16("firstname");
  field.value = ASCIIToUTF16("Jane");
  elements.push_back(field);

  field.name = ASCIIToUTF16("lastname");
  field.value = ASCIIToUTF16("Smith");
  elements.push_back(field);

  field.name = ASCIIToUTF16("lastname");
  field.value = ASCIIToUTF16("Jones");
  elements.push_back(field);

  std::vector<AutofillChange> changes;
  table_->AddFormFieldValuesTime(elements, &changes, t);

  ASSERT_EQ(2U, changes.size());
  EXPECT_EQ(changes[0], AutofillChange(AutofillChange::ADD,
                                       AutofillKey(ASCIIToUTF16("firstname"),
                                       ASCIIToUTF16("Joe"))));
  EXPECT_EQ(changes[1], AutofillChange(AutofillChange::ADD,
                                       AutofillKey(ASCIIToUTF16("lastname"),
                                       ASCIIToUTF16("Smith"))));

  std::vector<AutofillEntry> all_entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&all_entries));
  ASSERT_EQ(2U, all_entries.size());
}

TEST_F(AutofillTableTest,
       Autofill_RemoveFormElementsAddedBetween_UsedOnlyBefore) {
  // Add an entry used only before the targetted range.
  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(10)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(20)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(30)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(40)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(50)));

  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));

  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(base::Time::FromTimeT(51),
                                                     base::Time::FromTimeT(60),
                                                     &changes));
  EXPECT_TRUE(changes.empty());
  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));
}

TEST_F(AutofillTableTest,
       Autofill_RemoveFormElementsAddedBetween_UsedOnlyAfter) {
  // Add an entry used only after the targetted range.
  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(50)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(60)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(70)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(80)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(90)));

  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));

  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(base::Time::FromTimeT(40),
                                                     base::Time::FromTimeT(50),
                                                     &changes));
  EXPECT_TRUE(changes.empty());
  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));
}

TEST_F(AutofillTableTest,
       Autofill_RemoveFormElementsAddedBetween_UsedOnlyDuring) {
  // Add an entry used entirely during the targetted range.
  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(10)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(20)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(30)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(40)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(50)));

  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));

  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(base::Time::FromTimeT(10),
                                                     base::Time::FromTimeT(51),
                                                     &changes));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::REMOVE,
                           AutofillKey(field.name, field.value)),
            changes[0]);
  EXPECT_EQ(0, GetAutofillEntryCount(field.name, field.value, db_.get()));
}

TEST_F(AutofillTableTest,
       Autofill_RemoveFormElementsAddedBetween_UsedBeforeAndDuring) {
  // Add an entry used both before and during the targetted range.
  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(10)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(20)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(30)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(40)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(50)));

  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));

  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(base::Time::FromTimeT(40),
                                                     base::Time::FromTimeT(60),
                                                     &changes));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::UPDATE,
                           AutofillKey(field.name, field.value)),
            changes[0]);
  EXPECT_EQ(4, GetAutofillEntryCount(field.name, field.value, db_.get()));
  base::Time date_created, date_last_used;
  EXPECT_TRUE(
      table_->GetAutofillTimestamps(field.name, field.value,
                                    &date_created, &date_last_used));
  EXPECT_EQ(base::Time::FromTimeT(10), date_created);
  EXPECT_EQ(base::Time::FromTimeT(39), date_last_used);
}

TEST_F(AutofillTableTest,
       Autofill_RemoveFormElementsAddedBetween_UsedDuringAndAfter) {
  // Add an entry used both during and after the targetted range.
  AutofillChangeList changes;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(50)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(60)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(70)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(80)));
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            base::Time::FromTimeT(90)));

  EXPECT_EQ(5, GetAutofillEntryCount(field.name, field.value, db_.get()));

  changes.clear();
  EXPECT_TRUE(table_->RemoveFormElementsAddedBetween(base::Time::FromTimeT(40),
                                                     base::Time::FromTimeT(80),
                                                     &changes));
  ASSERT_EQ(1U, changes.size());
  EXPECT_EQ(AutofillChange(AutofillChange::UPDATE,
                           AutofillKey(field.name, field.value)),
            changes[0]);
  EXPECT_EQ(2, GetAutofillEntryCount(field.name, field.value, db_.get()));
  base::Time date_created, date_last_used;
  EXPECT_TRUE(
      table_->GetAutofillTimestamps(field.name, field.value,
                                    &date_created, &date_last_used));
  EXPECT_EQ(base::Time::FromTimeT(80), date_created);
  EXPECT_EQ(base::Time::FromTimeT(90), date_last_used);
}

TEST_F(AutofillTableTest, AutofillProfile) {
  // Add a 'Home' profile.
  AutofillProfile home_profile;
  home_profile.set_origin(std::string());
  home_profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("John"));
  home_profile.SetRawInfo(NAME_MIDDLE, ASCIIToUTF16("Q."));
  home_profile.SetRawInfo(NAME_LAST, ASCIIToUTF16("Smith"));
  home_profile.SetRawInfo(EMAIL_ADDRESS, ASCIIToUTF16("js@smith.xyz"));
  home_profile.SetRawInfo(COMPANY_NAME, ASCIIToUTF16("Google"));
  home_profile.SetRawInfo(ADDRESS_HOME_LINE1, ASCIIToUTF16("1234 Apple Way"));
  home_profile.SetRawInfo(ADDRESS_HOME_LINE2, ASCIIToUTF16("unit 5"));
  home_profile.SetRawInfo(ADDRESS_HOME_DEPENDENT_LOCALITY,
                          ASCIIToUTF16("Beverly Hills"));
  home_profile.SetRawInfo(ADDRESS_HOME_CITY, ASCIIToUTF16("Los Angeles"));
  home_profile.SetRawInfo(ADDRESS_HOME_STATE, ASCIIToUTF16("CA"));
  home_profile.SetRawInfo(ADDRESS_HOME_ZIP, ASCIIToUTF16("90025"));
  home_profile.SetRawInfo(ADDRESS_HOME_SORTING_CODE, ASCIIToUTF16("MAGIC ###"));
  home_profile.SetRawInfo(ADDRESS_HOME_COUNTRY, ASCIIToUTF16("US"));
  home_profile.SetRawInfo(PHONE_HOME_WHOLE_NUMBER, ASCIIToUTF16("18181234567"));
  home_profile.set_language_code("en");

  Time pre_creation_time = Time::Now();
  EXPECT_TRUE(table_->AddAutofillProfile(home_profile));
  Time post_creation_time = Time::Now();

  // Get the 'Home' profile.
  scoped_ptr<AutofillProfile> db_profile =
      table_->GetAutofillProfile(home_profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(home_profile, *db_profile);
  sql::Statement s_home(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified "
      "FROM autofill_profiles WHERE guid=?"));
  s_home.BindString(0, home_profile.guid());
  ASSERT_TRUE(s_home.is_valid());
  ASSERT_TRUE(s_home.Step());
  EXPECT_GE(s_home.ColumnInt64(0), pre_creation_time.ToTimeT());
  EXPECT_LE(s_home.ColumnInt64(0), post_creation_time.ToTimeT());
  EXPECT_FALSE(s_home.Step());

  // Add a 'Billing' profile.
  AutofillProfile billing_profile = home_profile;
  billing_profile.set_guid(base::GenerateGUID());
  billing_profile.set_origin("https://www.example.com/");
  billing_profile.SetRawInfo(ADDRESS_HOME_LINE1,
                             ASCIIToUTF16("5678 Bottom Street"));
  billing_profile.SetRawInfo(ADDRESS_HOME_LINE2, ASCIIToUTF16("suite 3"));

  pre_creation_time = Time::Now();
  EXPECT_TRUE(table_->AddAutofillProfile(billing_profile));
  post_creation_time = Time::Now();

  // Get the 'Billing' profile.
  db_profile = table_->GetAutofillProfile(billing_profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(billing_profile, *db_profile);
  sql::Statement s_billing(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles WHERE guid=?"));
  s_billing.BindString(0, billing_profile.guid());
  ASSERT_TRUE(s_billing.is_valid());
  ASSERT_TRUE(s_billing.Step());
  EXPECT_GE(s_billing.ColumnInt64(0), pre_creation_time.ToTimeT());
  EXPECT_LE(s_billing.ColumnInt64(0), post_creation_time.ToTimeT());
  EXPECT_FALSE(s_billing.Step());

  // Update the 'Billing' profile, name only.
  billing_profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("Jane"));
  Time pre_modification_time = Time::Now();
  EXPECT_TRUE(table_->UpdateAutofillProfile(billing_profile));
  Time post_modification_time = Time::Now();
  db_profile = table_->GetAutofillProfile(billing_profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(billing_profile, *db_profile);
  sql::Statement s_billing_updated(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles WHERE guid=?"));
  s_billing_updated.BindString(0, billing_profile.guid());
  ASSERT_TRUE(s_billing_updated.is_valid());
  ASSERT_TRUE(s_billing_updated.Step());
  EXPECT_GE(s_billing_updated.ColumnInt64(0),
            pre_modification_time.ToTimeT());
  EXPECT_LE(s_billing_updated.ColumnInt64(0),
            post_modification_time.ToTimeT());
  EXPECT_FALSE(s_billing_updated.Step());

  // Update the 'Billing' profile.
  billing_profile.set_origin("Chrome settings");
  billing_profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("Janice"));
  billing_profile.SetRawInfo(NAME_MIDDLE, ASCIIToUTF16("C."));
  billing_profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("Joplin"));
  billing_profile.SetRawInfo(EMAIL_ADDRESS, ASCIIToUTF16("jane@singer.com"));
  billing_profile.SetRawInfo(COMPANY_NAME, ASCIIToUTF16("Indy"));
  billing_profile.SetRawInfo(ADDRESS_HOME_LINE1, ASCIIToUTF16("Open Road"));
  billing_profile.SetRawInfo(ADDRESS_HOME_LINE2, ASCIIToUTF16("Route 66"));
  billing_profile.SetRawInfo(ADDRESS_HOME_DEPENDENT_LOCALITY,
                             ASCIIToUTF16("District 9"));
  billing_profile.SetRawInfo(ADDRESS_HOME_CITY, ASCIIToUTF16("NFA"));
  billing_profile.SetRawInfo(ADDRESS_HOME_STATE, ASCIIToUTF16("NY"));
  billing_profile.SetRawInfo(ADDRESS_HOME_ZIP, ASCIIToUTF16("10011"));
  billing_profile.SetRawInfo(ADDRESS_HOME_SORTING_CODE, ASCIIToUTF16("123456"));
  billing_profile.SetRawInfo(ADDRESS_HOME_COUNTRY, ASCIIToUTF16("US"));
  billing_profile.SetRawInfo(PHONE_HOME_WHOLE_NUMBER,
                             ASCIIToUTF16("18181230000"));
  Time pre_modification_time_2 = Time::Now();
  EXPECT_TRUE(table_->UpdateAutofillProfile(billing_profile));
  Time post_modification_time_2 = Time::Now();
  db_profile = table_->GetAutofillProfile(billing_profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(billing_profile, *db_profile);
  sql::Statement s_billing_updated_2(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM autofill_profiles WHERE guid=?"));
  s_billing_updated_2.BindString(0, billing_profile.guid());
  ASSERT_TRUE(s_billing_updated_2.is_valid());
  ASSERT_TRUE(s_billing_updated_2.Step());
  EXPECT_GE(s_billing_updated_2.ColumnInt64(0),
            pre_modification_time_2.ToTimeT());
  EXPECT_LE(s_billing_updated_2.ColumnInt64(0),
            post_modification_time_2.ToTimeT());
  EXPECT_FALSE(s_billing_updated_2.Step());

  // Remove the 'Billing' profile.
  EXPECT_TRUE(table_->RemoveAutofillProfile(billing_profile.guid()));
  db_profile = table_->GetAutofillProfile(billing_profile.guid());
  EXPECT_FALSE(db_profile);
}

TEST_F(AutofillTableTest, AutofillProfileTrash) {
  std::vector<std::string> guids;
  table_->GetAutofillProfilesInTrash(&guids);
  EXPECT_TRUE(guids.empty());

  ASSERT_TRUE(table_->AddAutofillGUIDToTrash(
      "00000000-0000-0000-0000-000000000000"));
  ASSERT_TRUE(table_->AddAutofillGUIDToTrash(
      "00000000-0000-0000-0000-000000000001"));
  ASSERT_TRUE(table_->GetAutofillProfilesInTrash(&guids));
  EXPECT_EQ(2UL, guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000000", guids[0]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000001", guids[1]);

  ASSERT_TRUE(table_->EmptyAutofillProfilesTrash());
  ASSERT_TRUE(table_->GetAutofillProfilesInTrash(&guids));
  EXPECT_TRUE(guids.empty());
}

TEST_F(AutofillTableTest, AutofillProfileTrashInteraction) {
  std::vector<std::string> guids;
  table_->GetAutofillProfilesInTrash(&guids);
  EXPECT_TRUE(guids.empty());

  AutofillProfile profile;
  profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("John"));
  profile.SetRawInfo(NAME_MIDDLE, ASCIIToUTF16("Q."));
  profile.SetRawInfo(NAME_LAST, ASCIIToUTF16("Smith"));
  profile.SetRawInfo(EMAIL_ADDRESS, ASCIIToUTF16("js@smith.xyz"));
  profile.SetRawInfo(ADDRESS_HOME_LINE1, ASCIIToUTF16("1 Main St"));
  profile.SetRawInfo(ADDRESS_HOME_CITY, ASCIIToUTF16("Los Angeles"));
  profile.SetRawInfo(ADDRESS_HOME_STATE, ASCIIToUTF16("CA"));
  profile.SetRawInfo(ADDRESS_HOME_ZIP, ASCIIToUTF16("90025"));
  profile.SetRawInfo(ADDRESS_HOME_COUNTRY, ASCIIToUTF16("US"));

  // Mark this profile as in the trash.  This stops |AddAutofillProfile| from
  // adding it.
  EXPECT_TRUE(table_->AddAutofillGUIDToTrash(profile.guid()));
  EXPECT_TRUE(table_->AddAutofillProfile(profile));
  scoped_ptr<AutofillProfile> added_profile =
      table_->GetAutofillProfile(profile.guid());
  EXPECT_FALSE(added_profile);

  // Add the profile for real this time.
  EXPECT_TRUE(table_->EmptyAutofillProfilesTrash());
  EXPECT_TRUE(table_->GetAutofillProfilesInTrash(&guids));
  EXPECT_TRUE(guids.empty());
  EXPECT_TRUE(table_->AddAutofillProfile(profile));
  added_profile = table_->GetAutofillProfile(profile.guid());
  EXPECT_TRUE(added_profile);

  // Mark this profile as in the trash.  This stops |UpdateAutofillProfileMulti|
  // from updating it.  In normal operation a profile should not be both in the
  // trash and in the profiles table simultaneously.
  EXPECT_TRUE(table_->AddAutofillGUIDToTrash(profile.guid()));
  profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("Jane"));
  EXPECT_TRUE(table_->UpdateAutofillProfile(profile));
  scoped_ptr<AutofillProfile> updated_profile =
      table_->GetAutofillProfile(profile.guid());
  EXPECT_TRUE(updated_profile);
  EXPECT_EQ(ASCIIToUTF16("John"), updated_profile->GetRawInfo(NAME_FIRST));

  // Try to delete the trashed profile.  This stops |RemoveAutofillProfile| from
  // deleting it.  In normal operation deletion is done by migration step, and
  // removal from trash is done by |WebDataService|.  |RemoveAutofillProfile|
  // does remove the item from the trash if it is found however, so that if
  // other clients remove it (via Sync say) then it is gone and doesn't need to
  // be processed further by |WebDataService|.
  EXPECT_TRUE(table_->RemoveAutofillProfile(profile.guid()));
  scoped_ptr<AutofillProfile> removed_profile =
      table_->GetAutofillProfile(profile.guid());
  EXPECT_TRUE(removed_profile);
  EXPECT_FALSE(table_->IsAutofillGUIDInTrash(profile.guid()));

  // Check that emptying the trash now allows removal to occur.
  EXPECT_TRUE(table_->EmptyAutofillProfilesTrash());
  EXPECT_TRUE(table_->RemoveAutofillProfile(profile.guid()));
  removed_profile = table_->GetAutofillProfile(profile.guid());
  EXPECT_FALSE(removed_profile);
}

TEST_F(AutofillTableTest, CreditCard) {
  // Add a 'Work' credit card.
  CreditCard work_creditcard;
  work_creditcard.set_origin("https://www.example.com/");
  work_creditcard.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Jack Torrance"));
  work_creditcard.SetRawInfo(CREDIT_CARD_NUMBER,
                             ASCIIToUTF16("1234567890123456"));
  work_creditcard.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("04"));
  work_creditcard.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR,
                             ASCIIToUTF16("2013"));

  Time pre_creation_time = Time::Now();
  EXPECT_TRUE(table_->AddCreditCard(work_creditcard));
  Time post_creation_time = Time::Now();

  // Get the 'Work' credit card.
  scoped_ptr<CreditCard> db_creditcard =
      table_->GetCreditCard(work_creditcard.guid());
  ASSERT_TRUE(db_creditcard);
  EXPECT_EQ(work_creditcard, *db_creditcard);
  sql::Statement s_work(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT guid, name_on_card, expiration_month, expiration_year, "
      "card_number_encrypted, date_modified "
      "FROM credit_cards WHERE guid=?"));
  s_work.BindString(0, work_creditcard.guid());
  ASSERT_TRUE(s_work.is_valid());
  ASSERT_TRUE(s_work.Step());
  EXPECT_GE(s_work.ColumnInt64(5), pre_creation_time.ToTimeT());
  EXPECT_LE(s_work.ColumnInt64(5), post_creation_time.ToTimeT());
  EXPECT_FALSE(s_work.Step());

  // Add a 'Target' credit card.
  CreditCard target_creditcard;
  target_creditcard.set_origin(std::string());
  target_creditcard.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Jack Torrance"));
  target_creditcard.SetRawInfo(CREDIT_CARD_NUMBER,
                               ASCIIToUTF16("1111222233334444"));
  target_creditcard.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("06"));
  target_creditcard.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR,
                               ASCIIToUTF16("2012"));

  pre_creation_time = Time::Now();
  EXPECT_TRUE(table_->AddCreditCard(target_creditcard));
  post_creation_time = Time::Now();
  db_creditcard = table_->GetCreditCard(target_creditcard.guid());
  ASSERT_TRUE(db_creditcard);
  EXPECT_EQ(target_creditcard, *db_creditcard);
  sql::Statement s_target(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT guid, name_on_card, expiration_month, expiration_year, "
      "card_number_encrypted, date_modified "
      "FROM credit_cards WHERE guid=?"));
  s_target.BindString(0, target_creditcard.guid());
  ASSERT_TRUE(s_target.is_valid());
  ASSERT_TRUE(s_target.Step());
  EXPECT_GE(s_target.ColumnInt64(5), pre_creation_time.ToTimeT());
  EXPECT_LE(s_target.ColumnInt64(5), post_creation_time.ToTimeT());
  EXPECT_FALSE(s_target.Step());

  // Update the 'Target' credit card.
  target_creditcard.set_origin("Interactive Autofill dialog");
  target_creditcard.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Charles Grady"));
  Time pre_modification_time = Time::Now();
  EXPECT_TRUE(table_->UpdateCreditCard(target_creditcard));
  Time post_modification_time = Time::Now();
  db_creditcard = table_->GetCreditCard(target_creditcard.guid());
  ASSERT_TRUE(db_creditcard);
  EXPECT_EQ(target_creditcard, *db_creditcard);
  sql::Statement s_target_updated(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT guid, name_on_card, expiration_month, expiration_year, "
      "card_number_encrypted, date_modified "
      "FROM credit_cards WHERE guid=?"));
  s_target_updated.BindString(0, target_creditcard.guid());
  ASSERT_TRUE(s_target_updated.is_valid());
  ASSERT_TRUE(s_target_updated.Step());
  EXPECT_GE(s_target_updated.ColumnInt64(5), pre_modification_time.ToTimeT());
  EXPECT_LE(s_target_updated.ColumnInt64(5), post_modification_time.ToTimeT());
  EXPECT_FALSE(s_target_updated.Step());

  // Remove the 'Target' credit card.
  EXPECT_TRUE(table_->RemoveCreditCard(target_creditcard.guid()));
  db_creditcard = table_->GetCreditCard(target_creditcard.guid());
  EXPECT_FALSE(db_creditcard);
}

TEST_F(AutofillTableTest, UpdateAutofillProfile) {
  // Add a profile to the db.
  AutofillProfile profile;
  profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("John"));
  profile.SetRawInfo(NAME_MIDDLE, ASCIIToUTF16("Q."));
  profile.SetRawInfo(NAME_LAST, ASCIIToUTF16("Smith"));
  profile.SetRawInfo(EMAIL_ADDRESS, ASCIIToUTF16("js@example.com"));
  profile.SetRawInfo(COMPANY_NAME, ASCIIToUTF16("Google"));
  profile.SetRawInfo(ADDRESS_HOME_LINE1, ASCIIToUTF16("1234 Apple Way"));
  profile.SetRawInfo(ADDRESS_HOME_LINE2, ASCIIToUTF16("unit 5"));
  profile.SetRawInfo(ADDRESS_HOME_CITY, ASCIIToUTF16("Los Angeles"));
  profile.SetRawInfo(ADDRESS_HOME_STATE, ASCIIToUTF16("CA"));
  profile.SetRawInfo(ADDRESS_HOME_ZIP, ASCIIToUTF16("90025"));
  profile.SetRawInfo(ADDRESS_HOME_COUNTRY, ASCIIToUTF16("US"));
  profile.SetRawInfo(PHONE_HOME_WHOLE_NUMBER, ASCIIToUTF16("18181234567"));
  profile.set_language_code("en");
  table_->AddAutofillProfile(profile);

  // Set a mocked value for the profile's creation time.
  const time_t kMockCreationDate = Time::Now().ToTimeT() - 13;
  sql::Statement s_mock_creation_date(
      db_->GetSQLConnection()->GetUniqueStatement(
          "UPDATE autofill_profiles SET date_modified = ?"));
  ASSERT_TRUE(s_mock_creation_date.is_valid());
  s_mock_creation_date.BindInt64(0, kMockCreationDate);
  ASSERT_TRUE(s_mock_creation_date.Run());

  // Get the profile.
  scoped_ptr<AutofillProfile> db_profile =
      table_->GetAutofillProfile(profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(profile, *db_profile);
  sql::Statement s_original(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_original.is_valid());
  ASSERT_TRUE(s_original.Step());
  EXPECT_EQ(kMockCreationDate, s_original.ColumnInt64(0));
  EXPECT_FALSE(s_original.Step());

  // Now, update the profile and save the update to the database.
  // The modification date should change to reflect the update.
  profile.SetRawInfo(EMAIL_ADDRESS, ASCIIToUTF16("js@smith.xyz"));
  table_->UpdateAutofillProfile(profile);

  // Get the profile.
  db_profile = table_->GetAutofillProfile(profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(profile, *db_profile);
  sql::Statement s_updated(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_updated.is_valid());
  ASSERT_TRUE(s_updated.Step());
  EXPECT_LT(kMockCreationDate, s_updated.ColumnInt64(0));
  EXPECT_FALSE(s_updated.Step());

  // Set a mocked value for the profile's modification time.
  const time_t mock_modification_date = Time::Now().ToTimeT() - 7;
  sql::Statement s_mock_modification_date(
      db_->GetSQLConnection()->GetUniqueStatement(
          "UPDATE autofill_profiles SET date_modified = ?"));
  ASSERT_TRUE(s_mock_modification_date.is_valid());
  s_mock_modification_date.BindInt64(0, mock_modification_date);
  ASSERT_TRUE(s_mock_modification_date.Run());

  // Finally, call into |UpdateAutofillProfile()| without changing the
  // profile.  The modification date should not change.
  table_->UpdateAutofillProfile(profile);

  // Get the profile.
  db_profile = table_->GetAutofillProfile(profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(profile, *db_profile);
  sql::Statement s_unchanged(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_unchanged.is_valid());
  ASSERT_TRUE(s_unchanged.Step());
  EXPECT_EQ(mock_modification_date, s_unchanged.ColumnInt64(0));
  EXPECT_FALSE(s_unchanged.Step());
}

TEST_F(AutofillTableTest, UpdateCreditCard) {
  // Add a credit card to the db.
  CreditCard credit_card;
  credit_card.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Jack Torrance"));
  credit_card.SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("1234567890123456"));
  credit_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("04"));
  credit_card.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2013"));
  table_->AddCreditCard(credit_card);

  // Set a mocked value for the credit card's creation time.
  const time_t kMockCreationDate = Time::Now().ToTimeT() - 13;
  sql::Statement s_mock_creation_date(
      db_->GetSQLConnection()->GetUniqueStatement(
          "UPDATE credit_cards SET date_modified = ?"));
  ASSERT_TRUE(s_mock_creation_date.is_valid());
  s_mock_creation_date.BindInt64(0, kMockCreationDate);
  ASSERT_TRUE(s_mock_creation_date.Run());

  // Get the credit card.
  scoped_ptr<CreditCard> db_credit_card =
      table_->GetCreditCard(credit_card.guid());
  ASSERT_TRUE(db_credit_card);
  EXPECT_EQ(credit_card, *db_credit_card);
  sql::Statement s_original(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_original.is_valid());
  ASSERT_TRUE(s_original.Step());
  EXPECT_EQ(kMockCreationDate, s_original.ColumnInt64(0));
  EXPECT_FALSE(s_original.Step());

  // Now, update the credit card and save the update to the database.
  // The modification date should change to reflect the update.
  credit_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("01"));
  table_->UpdateCreditCard(credit_card);

  // Get the credit card.
  db_credit_card = table_->GetCreditCard(credit_card.guid());
  ASSERT_TRUE(db_credit_card);
  EXPECT_EQ(credit_card, *db_credit_card);
  sql::Statement s_updated(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_updated.is_valid());
  ASSERT_TRUE(s_updated.Step());
  EXPECT_LT(kMockCreationDate, s_updated.ColumnInt64(0));
  EXPECT_FALSE(s_updated.Step());

  // Set a mocked value for the credit card's modification time.
  const time_t mock_modification_date = Time::Now().ToTimeT() - 7;
  sql::Statement s_mock_modification_date(
      db_->GetSQLConnection()->GetUniqueStatement(
          "UPDATE credit_cards SET date_modified = ?"));
  ASSERT_TRUE(s_mock_modification_date.is_valid());
  s_mock_modification_date.BindInt64(0, mock_modification_date);
  ASSERT_TRUE(s_mock_modification_date.Run());

  // Finally, call into |UpdateCreditCard()| without changing the credit card.
  // The modification date should not change.
  table_->UpdateCreditCard(credit_card);

  // Get the credit card.
  db_credit_card = table_->GetCreditCard(credit_card.guid());
  ASSERT_TRUE(db_credit_card);
  EXPECT_EQ(credit_card, *db_credit_card);
  sql::Statement s_unchanged(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_unchanged.is_valid());
  ASSERT_TRUE(s_unchanged.Step());
  EXPECT_EQ(mock_modification_date, s_unchanged.ColumnInt64(0));
  EXPECT_FALSE(s_unchanged.Step());
}

TEST_F(AutofillTableTest, UpdateProfileOriginOnly) {
  // Add a profile to the db.
  AutofillProfile profile;
  profile.SetRawInfo(NAME_FIRST, ASCIIToUTF16("John"));
  profile.SetRawInfo(NAME_MIDDLE, ASCIIToUTF16("Q."));
  profile.SetRawInfo(NAME_LAST, ASCIIToUTF16("Smith"));
  profile.SetRawInfo(EMAIL_ADDRESS, ASCIIToUTF16("js@example.com"));
  profile.SetRawInfo(COMPANY_NAME, ASCIIToUTF16("Google"));
  profile.SetRawInfo(ADDRESS_HOME_LINE1, ASCIIToUTF16("1234 Apple Way"));
  profile.SetRawInfo(ADDRESS_HOME_LINE2, ASCIIToUTF16("unit 5"));
  profile.SetRawInfo(ADDRESS_HOME_CITY, ASCIIToUTF16("Los Angeles"));
  profile.SetRawInfo(ADDRESS_HOME_STATE, ASCIIToUTF16("CA"));
  profile.SetRawInfo(ADDRESS_HOME_ZIP, ASCIIToUTF16("90025"));
  profile.SetRawInfo(ADDRESS_HOME_COUNTRY, ASCIIToUTF16("US"));
  profile.SetRawInfo(PHONE_HOME_WHOLE_NUMBER, ASCIIToUTF16("18181234567"));
  table_->AddAutofillProfile(profile);

  // Set a mocked value for the profile's creation time.
  const time_t kMockCreationDate = Time::Now().ToTimeT() - 13;
  sql::Statement s_mock_creation_date(
      db_->GetSQLConnection()->GetUniqueStatement(
          "UPDATE autofill_profiles SET date_modified = ?"));
  ASSERT_TRUE(s_mock_creation_date.is_valid());
  s_mock_creation_date.BindInt64(0, kMockCreationDate);
  ASSERT_TRUE(s_mock_creation_date.Run());

  // Get the profile.
  scoped_ptr<AutofillProfile> db_profile =
      table_->GetAutofillProfile(profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(profile, *db_profile);
  sql::Statement s_original(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_original.is_valid());
  ASSERT_TRUE(s_original.Step());
  EXPECT_EQ(kMockCreationDate, s_original.ColumnInt64(0));
  EXPECT_FALSE(s_original.Step());

  // Now, update just the profile's origin and save the update to the database.
  // The modification date should change to reflect the update.
  profile.set_origin("https://www.example.com/");
  table_->UpdateAutofillProfile(profile);

  // Get the profile.
  db_profile = table_->GetAutofillProfile(profile.guid());
  ASSERT_TRUE(db_profile);
  EXPECT_EQ(profile, *db_profile);
  sql::Statement s_updated(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_updated.is_valid());
  ASSERT_TRUE(s_updated.Step());
  EXPECT_LT(kMockCreationDate, s_updated.ColumnInt64(0));
  EXPECT_FALSE(s_updated.Step());
}

TEST_F(AutofillTableTest, UpdateCreditCardOriginOnly) {
  // Add a credit card to the db.
  CreditCard credit_card;
  credit_card.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Jack Torrance"));
  credit_card.SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("1234567890123456"));
  credit_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("04"));
  credit_card.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2013"));
  table_->AddCreditCard(credit_card);

  // Set a mocked value for the credit card's creation time.
  const time_t kMockCreationDate = Time::Now().ToTimeT() - 13;
  sql::Statement s_mock_creation_date(
      db_->GetSQLConnection()->GetUniqueStatement(
          "UPDATE credit_cards SET date_modified = ?"));
  ASSERT_TRUE(s_mock_creation_date.is_valid());
  s_mock_creation_date.BindInt64(0, kMockCreationDate);
  ASSERT_TRUE(s_mock_creation_date.Run());

  // Get the credit card.
  scoped_ptr<CreditCard> db_credit_card =
      table_->GetCreditCard(credit_card.guid());
  ASSERT_TRUE(db_credit_card);
  EXPECT_EQ(credit_card, *db_credit_card);
  sql::Statement s_original(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_original.is_valid());
  ASSERT_TRUE(s_original.Step());
  EXPECT_EQ(kMockCreationDate, s_original.ColumnInt64(0));
  EXPECT_FALSE(s_original.Step());

  // Now, update just the credit card's origin and save the update to the
  // database.  The modification date should change to reflect the update.
  credit_card.set_origin("https://www.example.com/");
  table_->UpdateCreditCard(credit_card);

  // Get the credit card.
  db_credit_card = table_->GetCreditCard(credit_card.guid());
  ASSERT_TRUE(db_credit_card);
  EXPECT_EQ(credit_card, *db_credit_card);
  sql::Statement s_updated(db_->GetSQLConnection()->GetUniqueStatement(
      "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_updated.is_valid());
  ASSERT_TRUE(s_updated.Step());
  EXPECT_LT(kMockCreationDate, s_updated.ColumnInt64(0));
  EXPECT_FALSE(s_updated.Step());
}

TEST_F(AutofillTableTest, RemoveAutofillDataModifiedBetween) {
  // Populate the autofill_profiles and credit_cards tables.
  ASSERT_TRUE(db_->GetSQLConnection()->Execute(
      "INSERT INTO autofill_profiles (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000000', 11);"
      "INSERT INTO autofill_profiles (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000001', 21);"
      "INSERT INTO autofill_profiles (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000002', 31);"
      "INSERT INTO autofill_profiles (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000003', 41);"
      "INSERT INTO autofill_profiles (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000004', 51);"
      "INSERT INTO autofill_profiles (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000005', 61);"
      "INSERT INTO credit_cards (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000006', 17);"
      "INSERT INTO credit_cards (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000007', 27);"
      "INSERT INTO credit_cards (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000008', 37);"
      "INSERT INTO credit_cards (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000009', 47);"
      "INSERT INTO credit_cards (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000010', 57);"
      "INSERT INTO credit_cards (guid, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000011', 67);"));

  // Remove all entries modified in the bounded time range [17,41).
  std::vector<std::string> profile_guids;
  std::vector<std::string> credit_card_guids;
  table_->RemoveAutofillDataModifiedBetween(
      Time::FromTimeT(17), Time::FromTimeT(41),
      &profile_guids, &credit_card_guids);
  ASSERT_EQ(2UL, profile_guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000001", profile_guids[0]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000002", profile_guids[1]);
  sql::Statement s_autofill_profiles_bounded(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_autofill_profiles_bounded.is_valid());
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(11, s_autofill_profiles_bounded.ColumnInt64(0));
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(41, s_autofill_profiles_bounded.ColumnInt64(0));
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(51, s_autofill_profiles_bounded.ColumnInt64(0));
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(61, s_autofill_profiles_bounded.ColumnInt64(0));
  EXPECT_FALSE(s_autofill_profiles_bounded.Step());
  ASSERT_EQ(3UL, credit_card_guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000006", credit_card_guids[0]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000007", credit_card_guids[1]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000008", credit_card_guids[2]);
  sql::Statement s_credit_cards_bounded(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_credit_cards_bounded.is_valid());
  ASSERT_TRUE(s_credit_cards_bounded.Step());
  EXPECT_EQ(47, s_credit_cards_bounded.ColumnInt64(0));
  ASSERT_TRUE(s_credit_cards_bounded.Step());
  EXPECT_EQ(57, s_credit_cards_bounded.ColumnInt64(0));
  ASSERT_TRUE(s_credit_cards_bounded.Step());
  EXPECT_EQ(67, s_credit_cards_bounded.ColumnInt64(0));
  EXPECT_FALSE(s_credit_cards_bounded.Step());

  // Remove all entries modified on or after time 51 (unbounded range).
  table_->RemoveAutofillDataModifiedBetween(
      Time::FromTimeT(51), Time(),
      &profile_guids, &credit_card_guids);
  ASSERT_EQ(2UL, profile_guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000004", profile_guids[0]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000005", profile_guids[1]);
  sql::Statement s_autofill_profiles_unbounded(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_autofill_profiles_unbounded.is_valid());
  ASSERT_TRUE(s_autofill_profiles_unbounded.Step());
  EXPECT_EQ(11, s_autofill_profiles_unbounded.ColumnInt64(0));
  ASSERT_TRUE(s_autofill_profiles_unbounded.Step());
  EXPECT_EQ(41, s_autofill_profiles_unbounded.ColumnInt64(0));
  EXPECT_FALSE(s_autofill_profiles_unbounded.Step());
  ASSERT_EQ(2UL, credit_card_guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000010", credit_card_guids[0]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000011", credit_card_guids[1]);
  sql::Statement s_credit_cards_unbounded(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_credit_cards_unbounded.is_valid());
  ASSERT_TRUE(s_credit_cards_unbounded.Step());
  EXPECT_EQ(47, s_credit_cards_unbounded.ColumnInt64(0));
  EXPECT_FALSE(s_credit_cards_unbounded.Step());

  // Remove all remaining entries.
  table_->RemoveAutofillDataModifiedBetween(
      Time(), Time(),
      &profile_guids, &credit_card_guids);
  ASSERT_EQ(2UL, profile_guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000000", profile_guids[0]);
  EXPECT_EQ("00000000-0000-0000-0000-000000000003", profile_guids[1]);
  sql::Statement s_autofill_profiles_empty(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM autofill_profiles"));
  ASSERT_TRUE(s_autofill_profiles_empty.is_valid());
  EXPECT_FALSE(s_autofill_profiles_empty.Step());
  ASSERT_EQ(1UL, credit_card_guids.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000009", credit_card_guids[0]);
  sql::Statement s_credit_cards_empty(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified FROM credit_cards"));
  ASSERT_TRUE(s_credit_cards_empty.is_valid());
  EXPECT_FALSE(s_credit_cards_empty.Step());
}

TEST_F(AutofillTableTest, RemoveOriginURLsModifiedBetween) {
  // Populate the autofill_profiles and credit_cards tables.
  ASSERT_TRUE(db_->GetSQLConnection()->Execute(
      "INSERT INTO autofill_profiles (guid, origin, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000000', '', 11);"
      "INSERT INTO autofill_profiles (guid, origin, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000001', "
      "       'https://www.example.com/', 21);"
      "INSERT INTO autofill_profiles (guid, origin, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000002', 'Chrome settings', 31);"
      "INSERT INTO credit_cards (guid, origin, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000003', '', 17);"
      "INSERT INTO credit_cards (guid, origin, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000004', "
      "       'https://www.example.com/', 27);"
      "INSERT INTO credit_cards (guid, origin, date_modified) "
      "VALUES('00000000-0000-0000-0000-000000000005', 'Chrome settings', "
      "       37);"));

  // Remove all origin URLs set in the bounded time range [21,27).
  ScopedVector<AutofillProfile> profiles;
  table_->RemoveOriginURLsModifiedBetween(
      Time::FromTimeT(21), Time::FromTimeT(27), &profiles);
  ASSERT_EQ(1UL, profiles.size());
  EXPECT_EQ("00000000-0000-0000-0000-000000000001", profiles[0]->guid());
  sql::Statement s_autofill_profiles_bounded(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified, origin FROM autofill_profiles"));
  ASSERT_TRUE(s_autofill_profiles_bounded.is_valid());
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(11, s_autofill_profiles_bounded.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_autofill_profiles_bounded.ColumnString(1));
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(21, s_autofill_profiles_bounded.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_autofill_profiles_bounded.ColumnString(1));
  ASSERT_TRUE(s_autofill_profiles_bounded.Step());
  EXPECT_EQ(31, s_autofill_profiles_bounded.ColumnInt64(0));
  EXPECT_EQ("Chrome settings", s_autofill_profiles_bounded.ColumnString(1));
  sql::Statement s_credit_cards_bounded(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified, origin FROM credit_cards"));
  ASSERT_TRUE(s_credit_cards_bounded.is_valid());
  ASSERT_TRUE(s_credit_cards_bounded.Step());
  EXPECT_EQ(17, s_credit_cards_bounded.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_credit_cards_bounded.ColumnString(1));
  ASSERT_TRUE(s_credit_cards_bounded.Step());
  EXPECT_EQ(27, s_credit_cards_bounded.ColumnInt64(0));
  EXPECT_EQ("https://www.example.com/",
            s_credit_cards_bounded.ColumnString(1));
  ASSERT_TRUE(s_credit_cards_bounded.Step());
  EXPECT_EQ(37, s_credit_cards_bounded.ColumnInt64(0));
  EXPECT_EQ("Chrome settings", s_credit_cards_bounded.ColumnString(1));

  // Remove all origin URLS.
  profiles.clear();
  table_->RemoveOriginURLsModifiedBetween(Time(), Time(), &profiles);
  EXPECT_EQ(0UL, profiles.size());
  sql::Statement s_autofill_profiles_all(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified, origin FROM autofill_profiles"));
  ASSERT_TRUE(s_autofill_profiles_all.is_valid());
  ASSERT_TRUE(s_autofill_profiles_all.Step());
  EXPECT_EQ(11, s_autofill_profiles_all.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_autofill_profiles_all.ColumnString(1));
  ASSERT_TRUE(s_autofill_profiles_all.Step());
  EXPECT_EQ(21, s_autofill_profiles_all.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_autofill_profiles_all.ColumnString(1));
  ASSERT_TRUE(s_autofill_profiles_all.Step());
  EXPECT_EQ(31, s_autofill_profiles_all.ColumnInt64(0));
  EXPECT_EQ("Chrome settings", s_autofill_profiles_all.ColumnString(1));
  sql::Statement s_credit_cards_all(
      db_->GetSQLConnection()->GetUniqueStatement(
          "SELECT date_modified, origin FROM credit_cards"));
  ASSERT_TRUE(s_credit_cards_all.is_valid());
  ASSERT_TRUE(s_credit_cards_all.Step());
  EXPECT_EQ(17, s_credit_cards_all.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_credit_cards_all.ColumnString(1));
  ASSERT_TRUE(s_credit_cards_all.Step());
  EXPECT_EQ(27, s_credit_cards_all.ColumnInt64(0));
  EXPECT_EQ(std::string(), s_credit_cards_all.ColumnString(1));
  ASSERT_TRUE(s_credit_cards_all.Step());
  EXPECT_EQ(37, s_credit_cards_all.ColumnInt64(0));
  EXPECT_EQ("Chrome settings", s_credit_cards_all.ColumnString(1));
}

TEST_F(AutofillTableTest, Autofill_GetAllAutofillEntries_NoResults) {
  std::vector<AutofillEntry> entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&entries));

  EXPECT_EQ(0U, entries.size());
}

TEST_F(AutofillTableTest, Autofill_GetAllAutofillEntries_OneResult) {
  AutofillChangeList changes;
  std::map<std::string, std::vector<Time> > name_value_times_map;

  time_t start = 0;
  std::vector<Time> timestamps1;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            Time::FromTimeT(start)));
  timestamps1.push_back(Time::FromTimeT(start));
  std::string key1("NameSuperman");
  name_value_times_map.insert(
      std::pair<std::string, std::vector<Time> >(key1, timestamps1));

  AutofillEntrySet expected_entries(CompareAutofillEntries);
  AutofillKey ak1(ASCIIToUTF16("Name"), ASCIIToUTF16("Superman"));
  AutofillEntry ae1(ak1, timestamps1.front(), timestamps1.back());

  expected_entries.insert(ae1);

  std::vector<AutofillEntry> entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&entries));
  AutofillEntrySet entry_set(entries.begin(), entries.end(),
                             CompareAutofillEntries);

  CompareAutofillEntrySets(entry_set, expected_entries);
}

TEST_F(AutofillTableTest, Autofill_GetAllAutofillEntries_TwoDistinct) {
  AutofillChangeList changes;
  std::map<std::string, std::vector<Time> > name_value_times_map;
  time_t start = 0;

  std::vector<Time> timestamps1;
  FormFieldData field;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Superman");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            Time::FromTimeT(start)));
  timestamps1.push_back(Time::FromTimeT(start));
  std::string key1("NameSuperman");
  name_value_times_map.insert(
      std::pair<std::string, std::vector<Time> >(key1, timestamps1));

  ++start;
  std::vector<Time> timestamps2;
  field.name = ASCIIToUTF16("Name");
  field.value = ASCIIToUTF16("Clark Kent");
  EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                            Time::FromTimeT(start)));
  timestamps2.push_back(Time::FromTimeT(start));
  std::string key2("NameClark Kent");
  name_value_times_map.insert(
      std::pair<std::string, std::vector<Time> >(key2, timestamps2));

  AutofillEntrySet expected_entries(CompareAutofillEntries);
  AutofillKey ak1(ASCIIToUTF16("Name"), ASCIIToUTF16("Superman"));
  AutofillKey ak2(ASCIIToUTF16("Name"), ASCIIToUTF16("Clark Kent"));
  AutofillEntry ae1(ak1, timestamps1.front(), timestamps1.back());
  AutofillEntry ae2(ak2, timestamps2.front(), timestamps2.back());

  expected_entries.insert(ae1);
  expected_entries.insert(ae2);

  std::vector<AutofillEntry> entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&entries));
  AutofillEntrySet entry_set(entries.begin(), entries.end(),
                             CompareAutofillEntries);

  CompareAutofillEntrySets(entry_set, expected_entries);
}

TEST_F(AutofillTableTest, Autofill_GetAllAutofillEntries_TwoSame) {
  AutofillChangeList changes;
  std::map<std::string, std::vector<Time> > name_value_times_map;

  std::vector<Time> timestamps;
  time_t start = 0;
  for (int i = 0; i < 2; ++i, ++start) {
    FormFieldData field;
    field.name = ASCIIToUTF16("Name");
    field.value = ASCIIToUTF16("Superman");
    EXPECT_TRUE(table_->AddFormFieldValueTime(field, &changes,
                                              Time::FromTimeT(start)));
    timestamps.push_back(Time::FromTimeT(start));
  }

  std::string key("NameSuperman");
  name_value_times_map.insert(
      std::pair<std::string, std::vector<Time> >(key, timestamps));

  AutofillEntrySet expected_entries(CompareAutofillEntries);
  AutofillKey ak1(ASCIIToUTF16("Name"), ASCIIToUTF16("Superman"));
  AutofillEntry ae1(ak1, timestamps.front(), timestamps.back());

  expected_entries.insert(ae1);

  std::vector<AutofillEntry> entries;
  ASSERT_TRUE(table_->GetAllAutofillEntries(&entries));
  AutofillEntrySet entry_set(entries.begin(), entries.end(),
                             CompareAutofillEntries);

  CompareAutofillEntrySets(entry_set, expected_entries);
}

TEST_F(AutofillTableTest, SetGetServerCards) {
  std::vector<CreditCard> inputs;
  inputs.push_back(CreditCard(CreditCard::FULL_SERVER_CARD, "a123"));
  inputs[0].SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Paul F. Tompkins"));
  inputs[0].SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("1"));
  inputs[0].SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2020"));
  inputs[0].SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("4111111111111111"));

  inputs.push_back(
      CreditCard(CreditCard::MASKED_SERVER_CARD, "b456"));
  inputs[1].SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Rick Roman"));
  inputs[1].SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("12"));
  inputs[1].SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("1997"));
  inputs[1].SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("1111"));
  inputs[1].SetTypeForMaskedCard(kVisaCard);
  inputs[1].SetServerStatus(CreditCard::EXPIRED);

  test::SetServerCreditCards(table_.get(), inputs);

  std::vector<CreditCard*> outputs;
  ASSERT_TRUE(table_->GetServerCreditCards(&outputs));
  ASSERT_EQ(inputs.size(), outputs.size());

  // Ordering isn't guaranteed, so fix the ordering if it's backwards.
  if (outputs[1]->server_id() == inputs[0].server_id())
    std::swap(outputs[0], outputs[1]);

  // GUIDs for server cards are dynamically generated so will be different
  // after reading from the DB. Check they're valid, but otherwise don't count
  // them in the comparison.
  inputs[0].set_guid(std::string());
  inputs[1].set_guid(std::string());
  outputs[0]->set_guid(std::string());
  outputs[1]->set_guid(std::string());

  EXPECT_EQ(inputs[0], *outputs[0]);
  EXPECT_EQ(inputs[1], *outputs[1]);

  EXPECT_EQ(CreditCard::OK, outputs[0]->GetServerStatus());
  EXPECT_EQ(CreditCard::EXPIRED, outputs[1]->GetServerStatus());

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
}

TEST_F(AutofillTableTest, MaskUnmaskServerCards) {
  base::string16 masked_number(ASCIIToUTF16("1111"));
  std::vector<CreditCard> inputs;
  inputs.push_back(CreditCard(CreditCard::MASKED_SERVER_CARD, "a123"));
  inputs[0].SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Jay Johnson"));
  inputs[0].SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("1"));
  inputs[0].SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2020"));
  inputs[0].SetRawInfo(CREDIT_CARD_NUMBER, masked_number);
  inputs[0].SetTypeForMaskedCard(kVisaCard);
  test::SetServerCreditCards(table_.get(), inputs);

  // Unmask the number. The full number should be available.
  base::string16 full_number(ASCIIToUTF16("4111111111111111"));
  ASSERT_TRUE(table_->UnmaskServerCreditCard(inputs[0],
                                             full_number));

  std::vector<CreditCard*> outputs;
  table_->GetServerCreditCards(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_TRUE(CreditCard::FULL_SERVER_CARD == outputs[0]->record_type());
  EXPECT_EQ(full_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();

  // Re-mask the number, we should only get the last 4 digits out.
  ASSERT_TRUE(table_->MaskServerCreditCard(inputs[0].server_id()));
  table_->GetServerCreditCards(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_TRUE(CreditCard::MASKED_SERVER_CARD == outputs[0]->record_type());
  EXPECT_EQ(masked_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();
}

// Calling SetServerCreditCards should replace all existing cards, but unmasked
// cards should not be re-masked.
TEST_F(AutofillTableTest, SetServerCardModify) {
  // Add a masked card.
  CreditCard masked_card(CreditCard::MASKED_SERVER_CARD, "a123");
  masked_card.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Paul F. Tompkins"));
  masked_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("1"));
  masked_card.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2020"));
  masked_card.SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("1111"));
  masked_card.SetTypeForMaskedCard(kVisaCard);

  std::vector<CreditCard> inputs;
  inputs.push_back(masked_card);
  test::SetServerCreditCards(table_.get(), inputs);

  // Now unmask it.
  base::string16 full_number = ASCIIToUTF16("4111111111111111");
  table_->UnmaskServerCreditCard(masked_card, full_number);

  // The card should now be unmasked.
  std::vector<CreditCard*> outputs;
  table_->GetServerCreditCards(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_TRUE(outputs[0]->record_type() == CreditCard::FULL_SERVER_CARD);
  EXPECT_EQ(full_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();

  // Call set again with the masked number.
  inputs[0] = masked_card;
  test::SetServerCreditCards(table_.get(), inputs);

  // The card should stay unmasked.
  table_->GetServerCreditCards(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_TRUE(outputs[0]->record_type() == CreditCard::FULL_SERVER_CARD);
  EXPECT_EQ(full_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();

  // Set inputs that do not include our old card.
  CreditCard random_card(CreditCard::MASKED_SERVER_CARD, "b456");
  random_card.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Rick Roman"));
  random_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("12"));
  random_card.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("1997"));
  random_card.SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("2222"));
  random_card.SetTypeForMaskedCard(kVisaCard);
  inputs[0] = random_card;
  test::SetServerCreditCards(table_.get(), inputs);

  // We should have only the new card, the other one should have been deleted.
  table_->GetServerCreditCards(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_TRUE(outputs[0]->record_type() == CreditCard::MASKED_SERVER_CARD);
  EXPECT_EQ(random_card.server_id(), outputs[0]->server_id());
  EXPECT_EQ(ASCIIToUTF16("2222"), outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();

  // Putting back the original card masked should make it masked (this tests
  // that the unmasked data was really deleted).
  inputs[0] = masked_card;
  test::SetServerCreditCards(table_.get(), inputs);
  table_->GetServerCreditCards(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_TRUE(outputs[0]->record_type() == CreditCard::MASKED_SERVER_CARD);
  EXPECT_EQ(masked_card.server_id(), outputs[0]->server_id());
  EXPECT_EQ(ASCIIToUTF16("1111"), outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();
}

TEST_F(AutofillTableTest, SetServerCardUpdateUsageStats) {
  // Add a masked card.
  CreditCard masked_card(CreditCard::MASKED_SERVER_CARD, "a123");
  masked_card.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Paul F. Tompkins"));
  masked_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("1"));
  masked_card.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2020"));
  masked_card.SetRawInfo(CREDIT_CARD_NUMBER, ASCIIToUTF16("1111"));
  masked_card.SetTypeForMaskedCard(kVisaCard);

  std::vector<CreditCard> inputs;
  inputs.push_back(masked_card);
  test::SetServerCreditCards(table_.get(), inputs);

  ScopedVector<CreditCard> outputs;
  table_->GetServerCreditCards(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(masked_card.server_id(), outputs[0]->server_id());
  EXPECT_EQ(0U, outputs[0]->use_count());
  EXPECT_EQ(base::Time(), outputs[0]->use_date());
  outputs.clear();

  // Update the usage stats; make sure they're reflected in GetServerProfiles.
  inputs.back().set_use_count(4U);
  inputs.back().set_use_date(base::Time::Now());
  table_->UpdateServerCardUsageStats(inputs.back());
  table_->GetServerCreditCards(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(masked_card.server_id(), outputs[0]->server_id());
  EXPECT_EQ(4U, outputs[0]->use_count());
  EXPECT_NE(base::Time(), outputs[0]->use_date());
  outputs.clear();

  // Setting the cards again shouldn't delete the usage stats.
  table_->SetServerCreditCards(inputs);
  table_->GetServerCreditCards(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(masked_card.server_id(), outputs[0]->server_id());
  EXPECT_EQ(4U, outputs[0]->use_count());
  EXPECT_NE(base::Time(), outputs[0]->use_date());
  outputs.clear();

  // Set a card list where the card is missing --- this should clear metadata.
  CreditCard masked_card2(CreditCard::MASKED_SERVER_CARD, "b456");
  inputs.back() = masked_card2;
  table_->SetServerCreditCards(inputs);

  // Back to the original card list.
  inputs.back() = masked_card;
  table_->SetServerCreditCards(inputs);
  table_->GetServerCreditCards(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(masked_card.server_id(), outputs[0]->server_id());
  EXPECT_EQ(0U, outputs[0]->use_count());
  EXPECT_EQ(base::Time(), outputs[0]->use_date());
  outputs.clear();
}

TEST_F(AutofillTableTest, SetServerProfile) {
  AutofillProfile one(AutofillProfile::SERVER_PROFILE, "a123");
  std::vector<AutofillProfile> inputs;
  inputs.push_back(one);
  table_->SetServerProfiles(inputs);

  std::vector<AutofillProfile*> outputs;
  table_->GetServerProfiles(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(one.server_id(), outputs[0]->server_id());

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();

  // Set a different profile.
  AutofillProfile two(AutofillProfile::SERVER_PROFILE, "b456");
  inputs[0] = two;
  table_->SetServerProfiles(inputs);

  // The original one should have been replaced.
  table_->GetServerProfiles(&outputs);
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(two.server_id(), outputs[0]->server_id());

  STLDeleteContainerPointers(outputs.begin(), outputs.end());
  outputs.clear();
}

TEST_F(AutofillTableTest, SetServerProfileUpdateUsageStats) {
  AutofillProfile one(AutofillProfile::SERVER_PROFILE, "a123");
  std::vector<AutofillProfile> inputs;
  inputs.push_back(one);
  table_->SetServerProfiles(inputs);

  ScopedVector<AutofillProfile> outputs;
  table_->GetServerProfiles(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(one.server_id(), outputs[0]->server_id());
  EXPECT_EQ(0U, outputs[0]->use_count());
  EXPECT_EQ(base::Time(), outputs[0]->use_date());
  outputs.clear();

  // Update the usage stats; make sure they're reflected in GetServerProfiles.
  inputs.back().set_use_count(4U);
  inputs.back().set_use_date(base::Time::Now());
  table_->UpdateServerAddressUsageStats(inputs.back());
  table_->GetServerProfiles(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(one.server_id(), outputs[0]->server_id());
  EXPECT_EQ(4U, outputs[0]->use_count());
  EXPECT_NE(base::Time(), outputs[0]->use_date());
  outputs.clear();

  // Setting the profiles again shouldn't delete the usage stats.
  table_->SetServerProfiles(inputs);
  table_->GetServerProfiles(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(one.server_id(), outputs[0]->server_id());
  EXPECT_EQ(4U, outputs[0]->use_count());
  EXPECT_NE(base::Time(), outputs[0]->use_date());
  outputs.clear();

  // Set a null profile list --- this should clear metadata.
  table_->SetServerProfiles(std::vector<AutofillProfile>());
  // Reset the old profile list and see the metadata is reset.
  table_->SetServerProfiles(inputs);
  table_->GetServerProfiles(&outputs.get());
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(one.server_id(), outputs[0]->server_id());
  EXPECT_EQ(0U, outputs[0]->use_count());
  EXPECT_EQ(base::Time(), outputs[0]->use_date());
  outputs.clear();
}

// Tests that deleting time ranges re-masks server credit cards that were
// unmasked in that time.
TEST_F(AutofillTableTest, DeleteUnmaskedCard) {
  // This isn't the exact unmasked time, since the database will use the
  // current time that it is called. The code below has to be approximate.
  base::Time unmasked_time = base::Time::Now();

  // Add a masked card.
  base::string16 masked_number = ASCIIToUTF16("1111");
  CreditCard masked_card(CreditCard::MASKED_SERVER_CARD, "a123");
  masked_card.SetRawInfo(CREDIT_CARD_NAME, ASCIIToUTF16("Paul F. Tompkins"));
  masked_card.SetRawInfo(CREDIT_CARD_EXP_MONTH, ASCIIToUTF16("1"));
  masked_card.SetRawInfo(CREDIT_CARD_EXP_4_DIGIT_YEAR, ASCIIToUTF16("2020"));
  masked_card.SetRawInfo(CREDIT_CARD_NUMBER, masked_number);
  masked_card.SetTypeForMaskedCard(kVisaCard);

  std::vector<CreditCard> inputs;
  inputs.push_back(masked_card);
  table_->SetServerCreditCards(inputs);

  // Unmask it.
  base::string16 full_number = ASCIIToUTF16("4111111111111111");
  table_->UnmaskServerCreditCard(masked_card, full_number);

  // Delete data in a range a year in the future.
  std::vector<std::string> profile_guids;
  std::vector<std::string> credit_card_guids;
  ASSERT_TRUE(table_->RemoveAutofillDataModifiedBetween(
      unmasked_time + base::TimeDelta::FromDays(365),
      unmasked_time + base::TimeDelta::FromDays(530),
      &profile_guids, &credit_card_guids));

  // This should not affect the unmasked card (should be unmasked).
  ScopedVector<CreditCard> outputs;
  ASSERT_TRUE(table_->GetServerCreditCards(&outputs.get()));
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(CreditCard::FULL_SERVER_CARD, outputs[0]->record_type());
  EXPECT_EQ(full_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));
  outputs.clear();

  // Delete data in the range of the last 24 hours.
  // Fudge |now| to make sure it's strictly greater than the |now| that
  // the database uses.
  base::Time now = base::Time::Now() + base::TimeDelta::FromSeconds(1);
  ASSERT_TRUE(table_->RemoveAutofillDataModifiedBetween(
      now - base::TimeDelta::FromDays(1), now,
      &profile_guids, &credit_card_guids));

  // This should re-mask.
  ASSERT_TRUE(table_->GetServerCreditCards(&outputs.get()));
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(CreditCard::MASKED_SERVER_CARD, outputs[0]->record_type());
  EXPECT_EQ(masked_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));
  outputs.clear();

  // Unmask again, the card should be back.
  table_->UnmaskServerCreditCard(masked_card, full_number);
  ASSERT_TRUE(table_->GetServerCreditCards(&outputs.get()));
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(CreditCard::FULL_SERVER_CARD, outputs[0]->record_type());
  EXPECT_EQ(full_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));
  outputs.clear();

  // Delete all data.
  ASSERT_TRUE(table_->RemoveAutofillDataModifiedBetween(
      base::Time(), base::Time::Max(), &profile_guids, &credit_card_guids));

  // Should be masked again.
  ASSERT_TRUE(table_->GetServerCreditCards(&outputs.get()));
  ASSERT_EQ(1u, outputs.size());
  EXPECT_EQ(CreditCard::MASKED_SERVER_CARD, outputs[0]->record_type());
  EXPECT_EQ(masked_number, outputs[0]->GetRawInfo(CREDIT_CARD_NUMBER));
  outputs.clear();
}

TEST_F(AutofillTableTest, GetFormValuesForElementName_SubstringMatchEnabled) {
  // Token matching is currently behind a flag.
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      switches::kEnableSuggestionsWithSubstringMatch);

  const size_t kMaxCount = 2;
  const struct {
    const char* const field_suggestion[kMaxCount];
    const char* const field_contents;
    size_t expected_suggestion_count;
    const char* const expected_suggestion[kMaxCount];
  } kTestCases[] = {
      {{"user.test", "test_user"}, "TEST",   2, {"test_user", "user.test"}},
      {{"user test", "test-user"}, "user",   2, {"user test", "test-user"}},
      {{"user test", "test-rest"}, "user",   1, {"user test", nullptr}},
      {{"user@test", "test_user"}, "user@t", 1, {"user@test", nullptr}},
      {{"user.test", "test_user"}, "er.tes", 0, {nullptr,     nullptr}},
      {{"user test", "test_user"}, "_ser",   0, {nullptr,     nullptr}},
      {{"user.test", "test_user"}, "%ser",   0, {nullptr,     nullptr}},
      {{"user.test", "test_user"},
       "; DROP TABLE autofill;",
       0,
       {nullptr, nullptr}},
  };

  for (size_t i = 0; i < arraysize(kTestCases); ++i) {
    SCOPED_TRACE(testing::Message()
                 << "suggestion = " << kTestCases[i].field_suggestion[0]
                 << ", contents = " << kTestCases[i].field_contents);

    Time t1 = Time::Now();

    // Simulate the submission of a handful of entries in a field called "Name".
    AutofillChangeList changes;
    FormFieldData field;
    for (size_t k = 0; k < kMaxCount; ++k) {
      field.name = ASCIIToUTF16("Name");
      field.value = ASCIIToUTF16(kTestCases[i].field_suggestion[k]);
      table_->AddFormFieldValue(field, &changes);
    }

    std::vector<base::string16> v;
    table_->GetFormValuesForElementName(
        ASCIIToUTF16("Name"), ASCIIToUTF16(kTestCases[i].field_contents), &v,
        6);

    EXPECT_EQ(kTestCases[i].expected_suggestion_count, v.size());
    for (size_t j = 0; j < kTestCases[i].expected_suggestion_count; ++j) {
      EXPECT_EQ(ASCIIToUTF16(kTestCases[i].expected_suggestion[j]), v[j]);
    }

    changes.clear();
    table_->RemoveFormElementsAddedBetween(t1, Time(), &changes);
  }
}

}  // namespace autofill
