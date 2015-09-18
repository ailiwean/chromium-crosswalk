// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_DRIVER_CURRENT_TAB_MATCHER_H_
#define COMPONENTS_SYNC_DRIVER_CURRENT_TAB_MATCHER_H_

#include "base/gtest_prod_util.h"
#include "components/sessions/session_types.h"
#include "components/sync_driver/revisit/page_equality.h"
#include "components/sync_driver/revisit/page_visit_observer.h"

namespace sync_driver {

// This class checks the current navigation entry for the given tabs to see if
// they correspond to the same page we were constructed to look for. Upon
// finding multiple matches, the most recently modified will be chosen.
class CurrentTabMatcher {
 public:
  explicit CurrentTabMatcher(const PageEquality& page_equality);
  void Check(const sessions::SessionTab* tab);
  void Emit(const PageVisitObserver::TransitionType transition);

 private:
  FRIEND_TEST_ALL_PREFIXES(CurrentTabMatcherTest, Timestamp);

  const PageEquality page_equality_;
  const sessions::SessionTab* most_recent_match_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(CurrentTabMatcher);
};

}  // namespace sync_driver

#endif  // COMPONENTS_SYNC_DRIVER_CURRENT_TAB_MATCHER_H_
