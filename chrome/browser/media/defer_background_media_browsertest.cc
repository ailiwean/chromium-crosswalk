// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "media/base/test_data_util.h"

// TODO(dalecurtis): Android does not correctly defer media today, fix and
// enable this test (needs disable of user gesture).  http://crbug.com/522157
#if !defined(OS_ANDROID)
typedef InProcessBrowserTest MediaBrowserTest;

IN_PROC_BROWSER_TEST_F(MediaBrowserTest, BackgroundMediaIsDeferred) {
  // Navigate to a video file, which would autoplay in the foreground, but won't
  // in the background due to deferred media loading for hidden tabs.
  ui_test_utils::NavigateToURLWithDisposition(
      browser(), content::GetFileUrlWithQuery(
                     media::GetTestDataFilePath("bear-640x360.webm"), ""),
      NEW_BACKGROUND_TAB, ui_test_utils::BROWSER_TEST_WAIT_FOR_NAVIGATION);

  ASSERT_EQ(2, browser()->tab_strip_model()->count());
  content::WebContents* background_contents =
      browser()->tab_strip_model()->GetWebContentsAt(1);
  EXPECT_TRUE(
      content::WaitForRenderFrameReady(background_contents->GetMainFrame()));
  EXPECT_NE(background_contents,
            browser()->tab_strip_model()->GetActiveWebContents());

  // If autoplay for background tabs isn't deferred the play event listener will
  // be attached too late to catch the event, and subsequently the test will hit
  // the ended event before the play event.
  EXPECT_TRUE(
      content::ExecuteScript(background_contents,
                             "var video = document.querySelector('video');"
                             "video.addEventListener('ended', function(event) {"
                             "  document.title = 'ended';"
                             "}, false);"
                             "video.addEventListener('play', function(event) {"
                             "  document.title = 'playing';"
                             "}, false);"));

  // Make the background tab w/ our video the active tab.
  browser()->tab_strip_model()->SelectNextTab();
  EXPECT_EQ(background_contents,
            browser()->tab_strip_model()->GetActiveWebContents());

  // If everything worked, we should see "playing" and not "ended".
  const base::string16 playing_str = base::UTF8ToUTF16("playing");
  const base::string16 ended_str = base::UTF8ToUTF16("ended");
  content::TitleWatcher watcher(background_contents, playing_str);
  watcher.AlsoWaitForTitle(ended_str);
  EXPECT_EQ(playing_str, watcher.WaitAndGetTitle());
}
#endif  // !defined(OS_ANDROID)
