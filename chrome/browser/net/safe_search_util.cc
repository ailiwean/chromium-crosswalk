// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/safe_search_util.h"

#include <string>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "chrome/common/url_constants.h"
#include "components/google/core/browser/google_util.h"
#include "net/cookies/cookie_util.h"
#include "net/http/http_request_headers.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"

namespace {

int g_force_google_safe_search_count_for_test = 0;
int g_force_youtube_safety_mode_count_for_test = 0;

const char kYouTubeSafetyModeHeaderName[] = "YouTube-Safety-Mode";
const char kYouTubeSafetyModeHeaderValue[] = "Active";

// Returns whether a URL parameter, |first_parameter| (e.g. foo=bar), has the
// same key as the the |second_parameter| (e.g. foo=baz). Both parameters
// must be in key=value form.
bool HasSameParameterKey(const std::string& first_parameter,
                         const std::string& second_parameter) {
  DCHECK(second_parameter.find("=") != std::string::npos);
  // Prefix for "foo=bar" is "foo=".
  std::string parameter_prefix = second_parameter.substr(
      0, second_parameter.find("=") + 1);
  return base::StartsWith(first_parameter, parameter_prefix,
                          base::CompareCase::INSENSITIVE_ASCII);
}

// Examines the query string containing parameters and adds the necessary ones
// so that SafeSearch is active. |query| is the string to examine and the
// return value is the |query| string modified such that SafeSearch is active.
std::string AddSafeSearchParameters(const std::string& query) {
  std::vector<std::string> new_parameters;
  std::string safe_parameter = chrome::kSafeSearchSafeParameter;
  std::string ssui_parameter = chrome::kSafeSearchSsuiParameter;

  std::vector<std::string> parameters;
  base::SplitString(query, '&', &parameters);

  std::vector<std::string>::iterator it;
  for (it = parameters.begin(); it < parameters.end(); ++it) {
    if (!HasSameParameterKey(*it, safe_parameter) &&
        !HasSameParameterKey(*it, ssui_parameter)) {
      new_parameters.push_back(*it);
    }
  }

  new_parameters.push_back(safe_parameter);
  new_parameters.push_back(ssui_parameter);
  return base::JoinString(new_parameters, "&");
}

} // namespace

namespace safe_search_util {

// If |request| is a request to Google Web Search the function
// enforces that the SafeSearch query parameters are set to active.
// Sets the query part of |new_url| with the new value of the parameters.
void ForceGoogleSafeSearch(const net::URLRequest* request, GURL* new_url) {
  ++g_force_google_safe_search_count_for_test;

  if (!google_util::IsGoogleSearchUrl(request->url()) &&
      !google_util::IsGoogleHomePageUrl(request->url()))
    return;

  std::string query = request->url().query();
  std::string new_query = AddSafeSearchParameters(query);
  if (query == new_query)
    return;

  GURL::Replacements replacements;
  replacements.SetQueryStr(new_query);
  *new_url = request->url().ReplaceComponents(replacements);
}

// If |request| is a request to YouTube, enforces YouTube's Safety Mode by
// setting YouTube's Safety Mode header.
void ForceYouTubeSafetyMode(const net::URLRequest* request,
                            net::HttpRequestHeaders* headers) {
  ++g_force_youtube_safety_mode_count_for_test;

  if (!google_util::IsYoutubeDomainUrl(
          request->url(),
          google_util::ALLOW_SUBDOMAIN,
          google_util::DISALLOW_NON_STANDARD_PORTS))
    return;

  headers->SetHeader(kYouTubeSafetyModeHeaderName,
                     kYouTubeSafetyModeHeaderValue);
}

int GetForceGoogleSafeSearchCountForTesting() {
  return g_force_google_safe_search_count_for_test;
}

int GetForceYouTubeSafetyModeCountForTesting() {
  return g_force_youtube_safety_mode_count_for_test;
}

void ClearForceGoogleSafeSearchCountForTesting() {
  g_force_google_safe_search_count_for_test = 0;
}

void ClearForceYouTubeSafetyModeCountForTesting() {
  g_force_youtube_safety_mode_count_for_test = 0;
}

}  // namespace safe_search_util
