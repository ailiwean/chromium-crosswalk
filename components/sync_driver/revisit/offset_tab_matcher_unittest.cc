// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync_driver/revisit/offset_tab_matcher.h"

#include "base/memory/scoped_ptr.h"
#include "base/test/histogram_tester.h"
#include "base/time/time.h"
#include "components/sessions/serialized_navigation_entry.h"
#include "components/sessions/serialized_navigation_entry_test_helper.h"
#include "components/sessions/session_types.h"
#include "components/sync_driver/revisit/page_equality.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

using sessions::SessionTab;

namespace sync_driver {

namespace {

static const std::string kExampleUrl = "http://www.example.com";
static const std::string kDifferentUrl = "http://www.different.com";

sessions::SerializedNavigationEntry Entry(const std::string& url) {
  return sessions::SerializedNavigationEntryTestHelper::CreateNavigation(url,
                                                                         "");
}

scoped_ptr<SessionTab> Tab(const int index,
                           const base::Time timestamp = base::Time::Now()) {
  scoped_ptr<SessionTab> tab(new SessionTab());
  tab->current_navigation_index = index;
  tab->timestamp = timestamp;
  return tab;
}

void VerifyMatch(OffsetTabMatcher& matcher, const int offset) {
  base::HistogramTester histogram_tester;
  matcher.Emit(PageVisitObserver::kTransitionPage);
  histogram_tester.ExpectUniqueSample("Sync.PageRevisitNavigationMatchOffset",
                                      offset, 1);
  histogram_tester.ExpectUniqueSample(
      "Sync.PageRevisitNavigationMatchTransition",
      PageVisitObserver::kTransitionPage, 1);
  histogram_tester.ExpectTotalCount("Sync.PageRevisitNavigationMatchAge", 1);
}

void VerifyMiss(OffsetTabMatcher& matcher) {
  base::HistogramTester histogram_tester;
  matcher.Emit(PageVisitObserver::kTransitionPage);
  histogram_tester.ExpectUniqueSample(
      "Sync.PageRevisitNavigationMissTransition",
      PageVisitObserver::kTransitionPage, 1);
}

}  // namespace

TEST(OffsetTabMatcherTest, NoCheck) {
  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  VerifyMiss(matcher);
}

TEST(OffsetTabMatcherTest, EmptyTab) {
  scoped_ptr<SessionTab> tab = Tab(0);
  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  VerifyMiss(matcher);
}

TEST(OffsetTabMatcherTest, HasMatchForward) {
  scoped_ptr<SessionTab> tab = Tab(0);
  tab->navigations.push_back(Entry(kDifferentUrl));
  tab->navigations.push_back(Entry(kExampleUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  VerifyMatch(matcher, 1);
}

TEST(OffsetTabMatcherTest, HasMatchBackward) {
  scoped_ptr<SessionTab> tab = Tab(1);
  tab->navigations.push_back(Entry(kExampleUrl));
  tab->navigations.push_back(Entry(kDifferentUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  VerifyMatch(matcher, -1);
}

TEST(OffsetTabMatcherTest, NoMatch) {
  scoped_ptr<SessionTab> tab = Tab(0);
  tab->navigations.push_back(Entry(kExampleUrl));
  tab->navigations.push_back(Entry(kDifferentUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  VerifyMiss(matcher);
}

TEST(OffsetTabMatcherTest, MultipleBackwardOffsets) {
  scoped_ptr<SessionTab> tab = Tab(4);
  tab->navigations.push_back(Entry(kExampleUrl));
  tab->navigations.push_back(Entry(kExampleUrl));
  tab->navigations.push_back(Entry(kExampleUrl));  // Expected.
  tab->navigations.push_back(Entry(kDifferentUrl));
  tab->navigations.push_back(Entry(kExampleUrl));  // Current.

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  VerifyMatch(matcher, -2);
}

TEST(OffsetTabMatcherTest, MultipleOffsets) {
  scoped_ptr<SessionTab> tab = Tab(1);
  tab->navigations.push_back(Entry(kExampleUrl));
  tab->navigations.push_back(Entry(kExampleUrl));  // Current.
  tab->navigations.push_back(Entry(kExampleUrl));
  tab->navigations.push_back(Entry(kExampleUrl));  // Expected.
  tab->navigations.push_back(Entry(kDifferentUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  VerifyMatch(matcher, 2);
}

TEST(OffsetTabMatcherTest, VeryForwardOffset) {
  scoped_ptr<SessionTab> tab = Tab(0);
  for (int i = 0; i < 20; i++) {
    tab->navigations.push_back(Entry(kDifferentUrl));
  }
  tab->navigations.push_back(Entry(kExampleUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  // Expect the offset to be clamped to +10.
  VerifyMatch(matcher, 10);
}

TEST(OffsetTabMatcherTest, VeryBackwardOffset) {
  scoped_ptr<SessionTab> tab = Tab(20);
  tab->navigations.push_back(Entry(kExampleUrl));
  for (int i = 0; i < 20; i++) {
    tab->navigations.push_back(Entry(kDifferentUrl));
  }

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab.get());
  // Expect the offset to be clamped to -10.
  VerifyMatch(matcher, -10);
}

TEST(OffsetTabMatcherTest, MultipleTabs) {
  scoped_ptr<SessionTab> tab1 = Tab(0, base::Time::UnixEpoch());
  tab1->navigations.push_back(Entry(kExampleUrl));
  tab1->navigations.push_back(Entry(kExampleUrl));

  scoped_ptr<SessionTab> tab2 = Tab(1, base::Time::Now());
  tab2->navigations.push_back(Entry(kExampleUrl));
  tab2->navigations.push_back(Entry(kExampleUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab1.get());
  matcher.Check(tab2.get());
  VerifyMatch(matcher, -1);
}

TEST(OffsetTabMatcherTest, MultipleTabsSameTime) {
  base::Time shared_now = base::Time::Now();

  scoped_ptr<SessionTab> tab1 = Tab(0, shared_now);
  tab1->navigations.push_back(Entry(kExampleUrl));
  tab1->navigations.push_back(Entry(kExampleUrl));

  scoped_ptr<SessionTab> tab2 = Tab(1, shared_now);
  tab2->navigations.push_back(Entry(kExampleUrl));
  tab2->navigations.push_back(Entry(kExampleUrl));

  OffsetTabMatcher matcher((PageEquality(GURL(kExampleUrl))));
  matcher.Check(tab1.get());
  matcher.Check(tab2.get());
  VerifyMatch(matcher, 1);
}

}  // namespace sync_driver
