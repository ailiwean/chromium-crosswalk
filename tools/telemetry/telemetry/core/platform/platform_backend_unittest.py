# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import time
import unittest

from telemetry import decorators
from telemetry.core import platform as platform_module


class PlatformBackendTest(unittest.TestCase):
  @decorators.Disabled('mac')  # http://crbug.com/440666
  def testPowerMonitoringSync(self):
    # Tests that the act of monitoring power doesn't blow up.
    platform = platform_module.GetHostPlatform()
    can_monitor_power = platform.CanMonitorPower()
    self.assertIsInstance(can_monitor_power, bool)
    if not can_monitor_power:
      logging.warning('Test not supported on this platform.')
      return

    browser_mock = lambda: None
    # Android needs to access the package of the monitored app.
    if platform.GetOSName() == 'android':
      # pylint: disable=W0212
      browser_mock._browser_backend = lambda: None
      # Monitor the launcher, which is always present.
      browser_mock._browser_backend.package = 'com.android.launcher'

    platform.StartMonitoringPower(browser_mock)
    time.sleep(0.001)
    output = platform.StopMonitoringPower()
    self.assertTrue(output.has_key('energy_consumption_mwh'))
    self.assertTrue(output.has_key('identifier'))
