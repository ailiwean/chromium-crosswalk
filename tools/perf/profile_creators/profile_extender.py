# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry.core import platform
from telemetry.util import wpr_modes
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import browser_finder_exceptions


class ProfileExtender(object):
  """Abstract base class for an object that constructs a Chrome profile."""

  def __init__(self, finder_options):
    """Initializer.

    |finder_options| is an instance of BrowserFinderOptions. When subclass
    implementations of this method inevitably attempt to find and launch a
    browser, they should pass |finder_options| to the relevant methods.

    Several properties of |finder_options| might require direct manipulation by
    subclasses. These are:
      |finder_options.output_profile_path|: The path at which the profile
      should be created.
      |finder_options.browser_options.profile_dir|: If this property is None,
      then a new profile is created. Otherwise, the existing profile is
      appended on to.
    """
    self._finder_options = finder_options

    # A reference to the browser that will be performing all of the tab
    # navigations.
    # This member is initialized during SetUpBrowser().
    self._browser = None

  def Run(self):
    """Creates or extends the profile."""
    raise NotImplementedError()

  def WebPageReplayArchivePath(self):
    """Returns the path to the WPR archive.

    Can be overridden by subclasses.
    """
    return None

  @property
  def finder_options(self):
    """The options to use to find and run the browser."""
    return self._finder_options

  @property
  def profile_path(self):
    """The path of the profile that the browser will use while it's running."""
    return self.finder_options.output_profile_path

  @property
  def browser(self):
    return self._browser

  def EnabledOSList(self):
    """Returns a list of OSes that this extender can run on.

    Can be overridden by subclasses.

    Returns:
        List of OS ('win', 'mac', or 'linux') that this extender can run on.
        None if this extender can run on all platforms.
    """
    return None

  def SetUpBrowser(self):
    """Finds and starts the browser.

    Can be overridden by subclasses. The subclass implementation must call the
    super class implementation.

    Subclasses do not need to call this method. This method is only necessary
    if the subclass needs to start a browser. If a subclass does call this
    method, the subclass must also call TearDownBrowser().
    """
    possible_browser = self._GetPossibleBrowser(self.finder_options)

    os_name = possible_browser.platform.GetOSName()
    enabled_os_list = self.EnabledOSList()
    if enabled_os_list is not None and os_name not in enabled_os_list:
      raise NotImplementedError(
        'This profile extender on %s is not yet supported'
        % os_name)

    assert possible_browser.supports_tab_control
    assert (platform.GetHostPlatform().GetOSName() in
        ["win", "mac", "linux"])

    self._SetUpWebPageReplay(self.finder_options, possible_browser)
    self._browser = possible_browser.Create(self.finder_options)

  def TearDownBrowser(self):
    """Tears down the browser.

    Can be overridden by subclasses. The subclass implementation must call the
    super class implementation.
    """
    if self._browser:
      self._browser.Close()
      self._browser = None

  def FetchWebPageReplayArchives(self):
    """Fetches the web page replay archives.

    Can be overridden by subclasses.
    """
    pass

  def _SetUpWebPageReplay(self, finder_options, possible_browser):
    """Sets up Web Page Replay, if necessary."""

    wpr_archive_path = self.WebPageReplayArchivePath()
    if not wpr_archive_path:
      return

    self.FetchWebPageReplayArchives()

    # The browser options needs to be passed to both the network controller
    # as well as the browser backend.
    browser_options = finder_options.browser_options
    if finder_options.use_live_sites:
      browser_options.wpr_mode = wpr_modes.WPR_OFF
    else:
      browser_options.wpr_mode = wpr_modes.WPR_REPLAY

    network_controller = possible_browser.platform.network_controller
    make_javascript_deterministic = True

    network_controller.SetReplayArgs(
        wpr_archive_path, browser_options.wpr_mode, browser_options.netsim,
        browser_options.extra_wpr_args, make_javascript_deterministic)

  def _GetPossibleBrowser(self, finder_options):
    """Return a possible_browser with the given options."""
    possible_browser = browser_finder.FindBrowser(finder_options)
    if not possible_browser:
      raise browser_finder_exceptions.BrowserFinderException(
          'No browser found.\n\nAvailable browsers:\n%s\n' %
          '\n'.join(browser_finder.GetAllAvailableBrowserTypes(finder_options)))
    finder_options.browser_options.browser_type = (
        possible_browser.browser_type)

    return possible_browser

