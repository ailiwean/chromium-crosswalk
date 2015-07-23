# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Presubmit for Chromium HTML resources. See chrome/browser/PRESUBMIT.py.
"""

import regex_check


class HtmlChecker(object):
  def __init__(self, input_api, output_api, file_filter=None):
    self.input_api = input_api
    self.output_api = output_api
    self.file_filter = file_filter

  def ClassesUseDashFormCheck(self, line_number, line):
    regex = self.input_api.re.compile("""
        (?:^|\s)                    # start of line or whitespace
        (class="[^"]*[A-Z_][^"]*")  # class contains caps or '_'
        """,
        self.input_api.re.VERBOSE)
    return regex_check.RegexCheck(self.input_api.re, line_number, line, regex,
        "Classes should use dash-form.")

  def DoNotCloseSingleTagsCheck(self, line_number, line):
    regex = r"(/>)"
    return regex_check.RegexCheck(self.input_api.re, line_number, line, regex,
        "Do not close single tags.")

  def DoNotUseBrElementCheck(self, line_number, line):
    regex = r"(<br)"
    return regex_check.RegexCheck(self.input_api.re, line_number, line, regex,
        "Do not use <br>; place blocking elements (<div>) as appropriate.")

  def DoNotUseInputTypeButtonCheck(self, line_number, line):
    regex = self.input_api.re.compile("""
        (<input [^>]*  # "<input " followed by anything but ">"
        type="button"  # type="button"
        [^>]*>)        # anything but ">" then ">"
        """,
        self.input_api.re.VERBOSE)
    return regex_check.RegexCheck(self.input_api.re, line_number, line, regex,
        'Use the button element instead of <input type="button">')

  def I18nContentJavaScriptCaseCheck(self, line_number, line):
    regex = self.input_api.re.compile("""
        (?:^|\s)                      # start of line or whitespace
        i18n-content="                # i18n-content="
        ([A-Z][^"]*|[^"]*[-_][^"]*)"  # starts with caps or contains '-' or '_'
        """,
        self.input_api.re.VERBOSE)
    return regex_check.RegexCheck(self.input_api.re, line_number, line, regex,
        "For i18n-content use javaScriptCase.")

  def LabelCheck(self, line_number, line):
    regex = self.input_api.re.compile("""
        (?:^|\s) # start of line or whitespace
        (for=)   # for=
        """,
        self.input_api.re.VERBOSE)
    return regex_check.RegexCheck(self.input_api.re, line_number, line, regex,
        "Avoid 'for' attribute on <label>. Place the input within the <label>, "
        "or use aria-labelledby for <select>.")

  def RunChecks(self):
    """Check for violations of the Chromium web development style guide. See
       http://chromium.org/developers/web-development-style-guide
    """
    results = []

    affected_files = self.input_api.change.AffectedFiles(
        file_filter=self.file_filter, include_deletes=False)

    for f in affected_files:
      errors = []

      for line_number, line in f.ChangedContents():
        errors.extend(filter(None, [
            self.ClassesUseDashFormCheck(line_number, line),
            self.DoNotCloseSingleTagsCheck(line_number, line),
            self.DoNotUseBrElementCheck(line_number, line),
            self.DoNotUseInputTypeButtonCheck(line_number, line),
            self.I18nContentJavaScriptCaseCheck(line_number, line),
            self.LabelCheck(line_number, line),
        ]))

      if errors:
        abs_local_path = f.AbsoluteLocalPath()
        file_indicator = 'Found HTML style issues in %s' % abs_local_path
        prompt_msg = file_indicator + '\n\n' + '\n'.join(errors) + '\n'
        results.append(self.output_api.PresubmitPromptWarning(prompt_msg))

    return results
