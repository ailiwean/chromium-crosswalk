// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/settings/md_settings_localized_strings_provider.h"

#include "chrome/grit/generated_resources.h"
#include "chrome/grit/locale_settings.h"
#include "content/public/browser/web_ui_data_source.h"

namespace {

// Note that md_settings.html contains a <script> tag which imports a script of
// the following name. These names must be kept in sync.
const char kLocalizedStringsFile[] = "strings.js";

void AddA11yStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString(
      "a11yPageTitle", IDS_SETTINGS_ACCESSIBILITY_PAGE_TITLE);
  html_source->AddLocalizedString(
      "moreFeaturesLink", IDS_SETTINGS_MORE_FEATURES_LINK);
  html_source->AddLocalizedString(
      "optionsInMenuLabel", IDS_SETTINGS_OPTIONS_IN_MENU_LABEL);
  html_source->AddLocalizedString(
      "largeMouseCursorLabel", IDS_SETTINGS_LARGE_MOUSE_CURSOR_LABEL);
  html_source->AddLocalizedString(
      "highContrastLabel", IDS_SETTINGS_HIGH_CONTRAST_LABEL);
  html_source->AddLocalizedString(
      "stickyKeysLabel", IDS_SETTINGS_STICKY_KEYS_LABEL);
  html_source->AddLocalizedString(
      "stickyKeysSublabel", IDS_SETTINGS_STICKY_KEYS_SUBLABEL);
  html_source->AddLocalizedString(
      "chromeVoxLabel", IDS_SETTINGS_CHROMEVOX_LABEL);
  html_source->AddLocalizedString(
      "chromeVoxSublabel", IDS_SETTINGS_CHROMEVOX_SUBLABEL);
  html_source->AddLocalizedString(
      "screenMagnifierLabel", IDS_SETTINGS_SCREEN_MAGNIFIER_LABEL);
  html_source->AddLocalizedString(
      "tapDraggingLabel", IDS_SETTINGS_TAP_DRAGGING_LABEL);
  html_source->AddLocalizedString(
      "clickOnStopLabel", IDS_SETTINGS_CLICK_ON_STOP_LABEL);
  html_source->AddLocalizedString(
      "delayBeforeClickLabel", IDS_SETTINGS_DELAY_BEFORE_CLICK_LABEL);
  html_source->AddLocalizedString(
      "delayBeforeClickExtremelyShort",
      IDS_SETTINGS_DELAY_BEFORE_CLICK_EXTREMELY_SHORT);
  html_source->AddLocalizedString(
      "delayBeforeClickVeryShort", IDS_SETTINGS_DELAY_BEFORE_CLICK_VERY_SHORT);
  html_source->AddLocalizedString(
      "delayBeforeClickShort", IDS_SETTINGS_DELAY_BEFORE_CLICK_SHORT);
  html_source->AddLocalizedString(
      "delayBeforeClickLong", IDS_SETTINGS_DELAY_BEFORE_CLICK_LONG);
  html_source->AddLocalizedString(
      "delayBeforeClickVeryLong", IDS_SETTINGS_DELAY_BEFORE_CLICK_VERY_LONG);
  html_source->AddLocalizedString(
      "onScreenKeyboardLabel", IDS_SETTINGS_ON_SCREEN_KEYBOARD_LABEL);
}

void AddAppearanceStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString(
      "appearancePageTitle", IDS_SETTINGS_APPEARANCE_PAGE_TITLE);
  html_source->AddLocalizedString(
      "setWallpaper", IDS_SETTINGS_SET_WALLPAPER);
  html_source->AddLocalizedString(
      "getThemes", IDS_SETTINGS_GET_THEMES);
  html_source->AddLocalizedString(
      "resetToDefaultTheme", IDS_SETTINGS_RESET_TO_DEFAULT_THEME);
  html_source->AddLocalizedString(
      "showHomeButton", IDS_SETTINGS_SHOW_HOME_BUTTON);
  html_source->AddLocalizedString(
      "showBookmarksBar", IDS_SETTINGS_SHOW_BOOKMARKS_BAR);
  html_source->AddLocalizedString(
      "homePageNtp", IDS_SETTINGS_HOME_PAGE_NTP);
  html_source->AddLocalizedString(
      "changeHomePage", IDS_SETTINGS_CHANGE_HOME_PAGE);
  html_source->AddLocalizedString(
      "themesGalleryUrl", IDS_THEMES_GALLERY_URL);
}

void AddCommonStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString("addLabel", IDS_ADD);
}

void AddDownloadsStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString(
      "downloadsPageTitle", IDS_SETTINGS_DOWNLOADS_PAGE_TITLE);
  html_source->AddLocalizedString(
      "downloadLocation", IDS_SETTINGS_DOWNLOAD_LOCATION);
  html_source->AddLocalizedString(
      "changeDownloadLocation", IDS_SETTINGS_CHANGE_DOWNLOAD_LOCATION);
  html_source->AddLocalizedString(
      "promptForDownload", IDS_SETTINGS_PROMPT_FOR_DOWNLOAD);
}

void AddDateTimeStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString(
      "dateTimePageTitle", IDS_SETTINGS_DATE_TIME_PAGE_TITLE);
  html_source->AddLocalizedString(
      "timeZone", IDS_SETTINGS_TIME_ZONE);
  html_source->AddLocalizedString(
      "use24HourClock", IDS_SETTINGS_USE_24_HOUR_CLOCK);
  html_source->AddLocalizedString(
      "dateTimeSetAutomatically", IDS_SETTINGS_DATE_TIME_SET_AUTOMATICALLY);
}

#if defined(OS_CHROMEOS)
void AddInternetStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString(
      "internetPageTitle", IDS_SETTINGS_INTERNET_PAGE_TITLE);
  html_source->AddLocalizedString(
      "internetDetailPageTitle", IDS_SETTINGS_INTERNET_DETAIL_PAGE_TITLE);
}
#endif

void AddSearchStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString("searchPageTitle",
                                  IDS_SETTINGS_SEARCH_PAGE_TITLE);
  html_source->AddLocalizedString("searchExplanation",
                                  IDS_SETTINGS_SEARCH_EXPLANATION);
  html_source->AddLocalizedString("searchManageButtonLabel",
                                  IDS_SETTINGS_SEARCH_MANAGE_BUTTON_LABEL);
  html_source->AddLocalizedString("searchOkGoogleLabel",
                                  IDS_SETTINGS_SEARCH_OK_GOOGLE_LABEL);
  html_source->AddLocalizedString(
      "searchOkGoogleLearnMoreLink",
      IDS_SETTINGS_SEARCH_OK_GOOGLE_LEARN_MORE_LINK);
  html_source->AddLocalizedString(
      "searchOkGoogleDescriptionLabel",
      IDS_SETTINGS_SEARCH_OK_GOOGLE_DESCRIPTION_LABEL);
}

void AddSearchEnginesStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString("searchEnginesPageTitle",
                                  IDS_SETTINGS_SEARCH_ENGINES_PAGE_TITLE);
  html_source->AddLocalizedString(
      "searchEnginesAddSearchEngineLabel",
      IDS_SETTINGS_SEARCH_ENGINES_ADD_SEARCH_ENGINE_LABEL);
  html_source->AddLocalizedString("searchEnginesLabel",
                                  IDS_SETTINGS_SEARCH_ENGINES_LABEL);
  html_source->AddLocalizedString(
      "searchEnginesOtherLabel",
      IDS_SETTINGS_SEARCH_ENGINES_OTHER_ENGINES_LABEL);
  html_source->AddLocalizedString("searchEnginesDomainLabel",
                                  IDS_SETTINGS_SEARCH_ENGINES_DOMAIN_LABEL);
  html_source->AddLocalizedString("searchEnginesKeywordLabel",
                                  IDS_SETTINGS_SEARCH_ENGINES_KEYWORD_LABEL);
  html_source->AddLocalizedString("searchEnginesQueryURLLabel",
                                  IDS_SETTINGS_SEARCH_ENGINES_QUERY_URL_LABEL);
  html_source->AddLocalizedString("searchEnginesAddButtonLabel",
                                  IDS_SETTINGS_SEARCH_ENGINES_ADD_BUTTON_LABEL);
}

void AddUsersStrings(content::WebUIDataSource* html_source) {
  html_source->AddLocalizedString("usersPageTitle",
                                  IDS_SETTINGS_USERS_PAGE_TITLE);
  html_source->AddLocalizedString("usersModifiedByOwnerLabel",
                                  IDS_SETTINGS_USERS_MODIFIED_BY_OWNER_LABEL);
  html_source->AddLocalizedString("guestBrowsingLabel",
                                  IDS_SETTINGS_USERS_GUEST_BROWSING_LABEL);
  html_source->AddLocalizedString("supervisedUsersLabel",
                                  IDS_SETTINGS_USERS_SUPERVISED_USERS_LABEL);
  html_source->AddLocalizedString("showOnSigninLabel",
                                  IDS_SETTINGS_USERS_SHOW_ON_SIGNIN_LABEL);
  html_source->AddLocalizedString("restrictSigninLabel",
                                  IDS_SETTINGS_USERS_RESTRICT_SIGNIN_LABEL);
  html_source->AddLocalizedString("addUsersLabel",
                                  IDS_SETTINGS_USERS_ADD_USERS_LABEL);
}

}  // namespace

namespace settings {

void AddLocalizedStrings(content::WebUIDataSource* html_source) {
  AddA11yStrings(html_source);
  AddAppearanceStrings(html_source);
  AddCommonStrings(html_source);
  AddDownloadsStrings(html_source);
  AddDateTimeStrings(html_source);
#if defined(OS_CHROMEOS)
  AddInternetStrings(html_source);
#endif
  AddSearchStrings(html_source);
  AddSearchEnginesStrings(html_source);
  AddUsersStrings(html_source);
  html_source->SetJsonPath(kLocalizedStringsFile);
}

}  // namespace settings
