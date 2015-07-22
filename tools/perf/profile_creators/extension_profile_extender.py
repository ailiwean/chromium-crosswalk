# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import time
import zipfile

from catapult_base import cloud_storage
from profile_creators import profile_extender
from telemetry.core import exceptions


# Remote target upload directory in cloud storage for extensions.
REMOTE_DIR = 'extension_set'

# Target zip file.
ZIP_NAME = 'extensions.zip'


class InvalidExtensionArchiveError(exceptions.Error):
  """Exception thrown when remote archive is invalid or malformed.

  Remote archive should be located at REMOTE_DIR/ZIP_NAME. Upon failure,
  prompts user to update remote archive using update_remote_extensions
  script.
  """

  def __init__(self, msg=''):
    msg += ('\nTry running\n'
            '\tpython update_remote_extensions.py -e extension_set.csv\n'
            'in src/tools/perf/profile_creator subdirectory.')
    super(InvalidExtensionArchiveError, self).__init__(msg)


class ExtensionProfileExtender(profile_extender.ProfileExtender):
  """Creates a profile with many extensions."""

  def __init__(self, finder_options):
    super(ExtensionProfileExtender, self).__init__(finder_options)
    self._extensions = []
    finder_options.browser_options.disable_default_apps = False

  def Run(self):
    """Superclass override."""
    # Download extensions from cloud and force-install extensions into profile.
    local_extensions_dir = os.path.join(self.profile_path,
                                        'external_extensions_crx')
    self._DownloadRemoteExtensions(cloud_storage.PARTNER_BUCKET,
                                   local_extensions_dir)
    self._LoadExtensions(local_extensions_dir, self.profile_path)

    try:
      self.SetUpBrowser()
      self._WaitForExtensionsToLoad()
    finally:
      self.TearDownBrowser()

  def RestrictedOSList(self):
    """Superclass override."""
    return ['mac']

  def _DownloadRemoteExtensions(self, remote_bucket, local_extensions_dir):
    """Downloads and unzips archive of common extensions to disk.

    Args:
        remote_bucket: bucket to download remote archive from.
        local_extensions_dir: destination extensions directory.

    Raises:
        InvalidExtensionArchiveError if remote archive is not found.
    """
    remote_zip_path = os.path.join(REMOTE_DIR, ZIP_NAME)
    local_zip_path = os.path.join(local_extensions_dir, ZIP_NAME)
    try:
      cloud_storage.Get(remote_bucket, remote_zip_path, local_zip_path)
    except:
      raise InvalidExtensionArchiveError('Can\'t find archive at gs://%s/%s..'
                                         % (remote_bucket, remote_zip_path))
    try:
      with zipfile.ZipFile(local_zip_path, 'r') as extensions_zip:
        extensions_zip.extractall(local_extensions_dir)
    finally:
      os.remove(local_zip_path)

  def _GetExtensionInfoFromCrx(self, crx_file):
    """Retrieves version + name of extension from CRX archive."""
    with zipfile.ZipFile(crx_file, 'r') as crx_zip:
      manifest_contents = crx_zip.read('manifest.json')
      decoded_manifest = json.loads(manifest_contents)
      crx_version = decoded_manifest['version']
      extension_name = decoded_manifest['name']
    return (crx_version, extension_name)

  def _LoadExtensions(self, local_extensions_dir, profile_dir):
    """Loads extensions in _local_extensions_dir into user profile.

    Extensions are loaded according to platform specifications at
    https://developer.chrome.com/extensions/external_extensions.html

    Args:
        local_extensions_dir: directory containing CRX files.
        profile_dir: target profile directory for the extensions.

    Raises:
        InvalidExtensionArchiveError if archive contains a non-CRX file.
    """
    ext_files = os.listdir(local_extensions_dir)
    external_ext_dir = os.path.join(profile_dir, 'External Extensions')
    os.makedirs(external_ext_dir)
    for ext_file in ext_files:
      ext_path = os.path.join(local_extensions_dir, ext_file)
      if not ext_file.endswith('.crx'):
        raise InvalidExtensionArchiveError('Archive contains non-crx file %s.'
                                           % ext_file)
      (version, name) = self._GetExtensionInfoFromCrx(ext_path)
      extension_info = {
        'external_crx': ext_path,
        'external_version': version,
        '_comment': name
      }
      ext_id = os.path.splitext(os.path.basename(ext_path))[0]
      extension_json_path = os.path.join(external_ext_dir, '%s.json' % ext_id)
      with open(extension_json_path, 'w') as f:
        f.write(json.dumps(extension_info))
      self._extensions.append(ext_id)

  def _WaitForExtensionsToLoad(self):
    """Stall until browser has finished installing/loading all extensions."""
    for extension_id in self._extensions:
      while True:
        try:
          self.browser.extensions.GetByExtensionId(extension_id)
          break
        except KeyError:
          # There's no event signalling when browser finishes installing
          # or loading an extension so re-check every 5 seconds.
          time.sleep(5)
