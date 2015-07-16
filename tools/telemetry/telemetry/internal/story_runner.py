#  Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import optparse
import os
import sys
import time

from catapult_base import cloud_storage
from telemetry.core import exceptions
from telemetry.internal.actions import page_action
from telemetry.internal.results import results_options
from telemetry.internal.util import exception_formatter
from telemetry.page import page_test
from telemetry import story as story_module
from telemetry.util import wpr_modes
from telemetry.value import failure
from telemetry.value import skip
from telemetry.web_perf import story_test


class ArchiveError(Exception):
  pass


def AddCommandLineArgs(parser):
  story_module.StoryFilter.AddCommandLineArgs(parser)
  results_options.AddResultsOptions(parser)

  # Page set options
  group = optparse.OptionGroup(parser, 'Page set repeat options')
  group.add_option('--page-repeat', default=1, type='int',
                   help='Number of times to repeat each individual page '
                   'before proceeding with the next page in the pageset.')
  group.add_option('--pageset-repeat', default=1, type='int',
                   help='Number of times to repeat the entire pageset.')
  group.add_option('--max-failures', default=None, type='int',
                   help='Maximum number of test failures before aborting '
                   'the run. Defaults to the number specified by the '
                   'PageTest.')
  parser.add_option_group(group)

  # WPR options
  group = optparse.OptionGroup(parser, 'Web Page Replay options')
  group.add_option('--use-live-sites',
      dest='use_live_sites', action='store_true',
      help='Run against live sites and ignore the Web Page Replay archives.')
  parser.add_option_group(group)

  parser.add_option('-d', '--also-run-disabled-tests',
                    dest='run_disabled_tests',
                    action='store_true', default=False,
                    help='Ignore @Disabled and @Enabled restrictions.')

def ProcessCommandLineArgs(parser, args):
  story_module.StoryFilter.ProcessCommandLineArgs(parser, args)
  results_options.ProcessCommandLineArgs(parser, args)

  # Page set options
  if args.page_repeat < 1:
    parser.error('--page-repeat must be a positive integer.')
  if args.pageset_repeat < 1:
    parser.error('--pageset-repeat must be a positive integer.')


def _RunStoryAndProcessErrorIfNeeded(story, results, state, test):
  def ProcessError():
    results.AddValue(failure.FailureValue(story, sys.exc_info()))
  try:
    if isinstance(test, story_test.StoryTest):
      test.WillRunStory(state.platform)
    state.WillRunStory(story)
    if not state.CanRunStory(story):
      results.AddValue(skip.SkipValue(
          story,
          'Skipped because story is not supported '
          '(SharedState.CanRunStory() returns False).'))
      return
    state.RunStory(results)
    if isinstance(test, story_test.StoryTest):
      test.Measure(state.platform, results)
  except (page_test.Failure, exceptions.TimeoutException,
          exceptions.LoginException, exceptions.ProfilingException):
    ProcessError()
  except exceptions.Error:
    ProcessError()
    raise
  except page_action.PageActionNotSupported as e:
    results.AddValue(
        skip.SkipValue(story, 'Unsupported page action: %s' % e))
  except Exception:
    results.AddValue(
        failure.FailureValue(
            story, sys.exc_info(), 'Unhandlable exception raised.'))
    raise
  finally:
    has_existing_exception = sys.exc_info() is not None
    try:
      state.DidRunStory(results)
      # if state.DidRunStory raises exception, things are messed up badly and we
      # do not need to run test.DidRunStory at that point.
      if isinstance(test, story_test.StoryTest):
        test.DidRunStory(state.platform)
    except Exception:
      if not has_existing_exception:
        raise
      # Print current exception and propagate existing exception.
      exception_formatter.PrintFormattedException(
          msg='Exception raised when cleaning story run: ')


class StoryGroup(object):
  def __init__(self, shared_state_class):
    self._shared_state_class = shared_state_class
    self._stories = []

  @property
  def shared_state_class(self):
    return self._shared_state_class

  @property
  def stories(self):
    return self._stories

  def AddStory(self, story):
    assert (story.shared_state_class is
            self._shared_state_class)
    self._stories.append(story)


def StoriesGroupedByStateClass(story_set, allow_multiple_groups):
  """ Returns a list of story groups which each contains stories with
  the same shared_state_class.

  Example:
    Assume A1, A2, A3 are stories with same shared story class, and
    similar for B1, B2.
    If their orders in story set is A1 A2 B1 B2 A3, then the grouping will
    be [A1 A2] [B1 B2] [A3].

  It's purposefully done this way to make sure that order of
  stories are the same of that defined in story_set. It's recommended that
  stories with the same states should be arranged next to each others in
  story sets to reduce the overhead of setting up & tearing down the
  shared story state.
  """
  story_groups = []
  story_groups.append(
      StoryGroup(story_set[0].shared_state_class))
  for story in story_set:
    if (story.shared_state_class is not
        story_groups[-1].shared_state_class):
      if not allow_multiple_groups:
        raise ValueError('This StorySet is only allowed to have one '
                         'SharedState but contains the following '
                         'SharedState classes: %s, %s.\n Either '
                         'remove the extra SharedStates or override '
                         'allow_mixed_story_states.' % (
                         story_groups[-1].shared_state_class,
                         story.shared_state_class))
      story_groups.append(
          StoryGroup(story.shared_state_class))
    story_groups[-1].AddStory(story)
  return story_groups


def Run(test, story_set, finder_options, results, max_failures=None):
  """Runs a given test against a given page_set with the given options.

  Stop execution for unexpected exceptions such as KeyboardInterrupt.
  We "white list" certain exceptions for which the story runner
  can continue running the remaining stories.
  """
  # Filter page set based on options.
  stories = filter(story_module.StoryFilter.IsSelected, story_set)

  if (not finder_options.use_live_sites and story_set.bucket and
      finder_options.browser_options.wpr_mode != wpr_modes.WPR_RECORD):
    serving_dirs = story_set.serving_dirs
    for directory in serving_dirs:
      cloud_storage.GetFilesInDirectoryIfChanged(directory,
                                                 story_set.bucket)
    if not _UpdateAndCheckArchives(
        story_set.archive_data_file, story_set.wpr_archive_info,
        stories):
      return

  if not stories:
    return

  # Effective max failures gives priority to command-line flag value.
  effective_max_failures = finder_options.max_failures
  if effective_max_failures is None:
    effective_max_failures = max_failures

  story_groups = StoriesGroupedByStateClass(
      stories,
      story_set.allow_mixed_story_states)

  for group in story_groups:
    state = None
    try:
      for _ in xrange(finder_options.pageset_repeat):
        for story in group.stories:
          for _ in xrange(finder_options.page_repeat):
            if not state:
              state = group.shared_state_class(
                  test, finder_options, story_set)
            results.WillRunPage(story)
            try:
              _WaitForThermalThrottlingIfNeeded(state.platform)
              _RunStoryAndProcessErrorIfNeeded(story, results, state, test)
            except exceptions.Error:
              # Catch all Telemetry errors to give the story a chance to retry.
              # The retry is enabled by tearing down the state and creating
              # a new state instance in the next iteration.
              try:
                # If TearDownState raises, do not catch the exception.
                # (The Error was saved as a failure value.)
                state.TearDownState()
              finally:
                # Later finally-blocks use state, so ensure it is cleared.
                state = None
            finally:
              has_existing_exception = sys.exc_info() is not None
              try:
                if state:
                  _CheckThermalThrottling(state.platform)
                results.DidRunPage(story)
              except Exception:
                if not has_existing_exception:
                  raise
                # Print current exception and propagate existing exception.
                exception_formatter.PrintFormattedException(
                    msg='Exception from result processing:')
          if (effective_max_failures is not None and
              len(results.failures) > effective_max_failures):
            logging.error('Too many failures. Aborting.')
            return
    finally:
      if state:
        has_existing_exception = sys.exc_info() is not None
        try:
          state.TearDownState()
        except Exception:
          if not has_existing_exception:
            raise
          # Print current exception and propagate existing exception.
          exception_formatter.PrintFormattedException(
              msg='Exception from TearDownState:')


def _UpdateAndCheckArchives(archive_data_file, wpr_archive_info,
                            filtered_stories):
  """Verifies that all stories are local or have WPR archives.

  Logs warnings and returns False if any are missing.
  """
  # Report any problems with the entire story set.
  if any(not story.is_local for story in filtered_stories):
    if not archive_data_file:
      logging.error('The story set is missing an "archive_data_file" '
                    'property.\nTo run from live sites pass the flag '
                    '--use-live-sites.\nTo create an archive file add an '
                    'archive_data_file property to the story set and then '
                    'run record_wpr.')
      raise ArchiveError('No archive data file.')
    if not wpr_archive_info:
      logging.error('The archive info file is missing.\n'
                    'To fix this, either add svn-internal to your '
                    '.gclient using http://goto/read-src-internal, '
                    'or create a new archive using record_wpr.')
      raise ArchiveError('No archive info file.')
    wpr_archive_info.DownloadArchivesIfNeeded()

  # Report any problems with individual story.
  stories_missing_archive_path = []
  stories_missing_archive_data = []
  for story in filtered_stories:
    if not story.is_local:
      archive_path = wpr_archive_info.WprFilePathForStory(story)
      if not archive_path:
        stories_missing_archive_path.append(story)
      elif not os.path.isfile(archive_path):
        stories_missing_archive_data.append(story)
  if stories_missing_archive_path:
    logging.error(
        'The story set archives for some stories do not exist.\n'
        'To fix this, record those stories using record_wpr.\n'
        'To ignore this warning and run against live sites, '
        'pass the flag --use-live-sites.')
    logging.error(
        'stories without archives: %s',
        ', '.join(story.display_name
                  for story in stories_missing_archive_path))
  if stories_missing_archive_data:
    logging.error(
        'The story set archives for some stories are missing.\n'
        'Someone forgot to check them in, uploaded them to the '
        'wrong cloud storage bucket, or they were deleted.\n'
        'To fix this, record those stories using record_wpr.\n'
        'To ignore this warning and run against live sites, '
        'pass the flag --use-live-sites.')
    logging.error(
        'stories missing archives: %s',
        ', '.join(story.display_name
                  for story in stories_missing_archive_data))
  if stories_missing_archive_path or stories_missing_archive_data:
    raise ArchiveError('Archive file is missing stories.')
  # Only run valid stories if no problems with the story set or
  # individual stories.
  return True


def _WaitForThermalThrottlingIfNeeded(platform):
  if not platform.CanMonitorThermalThrottling():
    return
  thermal_throttling_retry = 0
  while (platform.IsThermallyThrottled() and
         thermal_throttling_retry < 3):
    logging.warning('Thermally throttled, waiting (%d)...',
                    thermal_throttling_retry)
    thermal_throttling_retry += 1
    time.sleep(thermal_throttling_retry * 2)

  if thermal_throttling_retry and platform.IsThermallyThrottled():
    logging.warning('Device is thermally throttled before running '
                    'performance tests, results will vary.')


def _CheckThermalThrottling(platform):
  if not platform.CanMonitorThermalThrottling():
    return
  if platform.HasBeenThermallyThrottled():
    logging.warning('Device has been thermally throttled during '
                    'performance tests, results will vary.')
