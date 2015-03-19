// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Namespace for common types shared between VolumeManager and
 * VolumeManagerWrapper.
 */
var VolumeManagerCommon = {};

/**
 * Type of a root directory.
 * @enum {string}
 * @const
 */
VolumeManagerCommon.RootType = {
  // Root for a downloads directory.
  DOWNLOADS: 'downloads',

  // Root for a mounted archive volume.
  ARCHIVE: 'archive',

  // Root for a removable volume.
  REMOVABLE: 'removable',

  // Root for a drive volume.
  DRIVE: 'drive',

  // Root for a privet storage volume.
  CLOUD_DEVICE: 'cloud_device',

  // Root for a MTP volume.
  MTP: 'mtp',

  // Root for a provided volume.
  PROVIDED: 'provided',

  // Root for entries that is not located under RootType.DRIVE. e.g. shared
  // files.
  DRIVE_OTHER: 'drive_other',

  // Fake root for offline available files on the drive.
  DRIVE_OFFLINE: 'drive_offline',

  // Fake root for shared files on the drive.
  DRIVE_SHARED_WITH_ME: 'drive_shared_with_me',

  // Fake root for recent files on the drive.
  DRIVE_RECENT: 'drive_recent'
};
Object.freeze(VolumeManagerCommon.RootType);

/**
 * Error type of VolumeManager.
 * @enum {string}
 * @const
 */
VolumeManagerCommon.VolumeError = {
  /* Internal errors */
  TIMEOUT: 'timeout',

  /* System events */
  UNKNOWN: 'error_unknown',
  INTERNAL: 'error_internal',
  INVALID_ARGUMENT: 'error_invalid_argument',
  INVALID_PATH: 'error_invalid_path',
  ALREADY_MOUNTED: 'error_path_already_mounted',
  PATH_NOT_MOUNTED: 'error_path_not_mounted',
  DIRECTORY_CREATION_FAILED: 'error_directory_creation_failed',
  INVALID_MOUNT_OPTIONS: 'error_invalid_mount_options',
  INVALID_UNMOUNT_OPTIONS: 'error_invalid_unmount_options',
  INSUFFICIENT_PERMISSIONS: 'error_insufficient_permissions',
  MOUNT_PROGRAM_NOT_FOUND: 'error_mount_program_not_found',
  MOUNT_PROGRAM_FAILED: 'error_mount_program_failed',
  INVALID_DEVICE_PATH: 'error_invalid_device_path',
  UNKNOWN_FILESYSTEM: 'error_unknown_filesystem',
  UNSUPPORTED_FILESYSTEM: 'error_unsupported_filesystem',
  INVALID_ARCHIVE: 'error_invalid_archive',
  AUTHENTICATION: 'error_authentication',
  PATH_UNMOUNTED: 'error_path_unmounted'
};
Object.freeze(VolumeManagerCommon.VolumeError);

/**
 * List of connection types of drive.
 *
 * Keep this in sync with the kDriveConnectionType* constants in
 * private_api_dirve.cc.
 *
 * @enum {string}
 * @const
 */
VolumeManagerCommon.DriveConnectionType = {
  OFFLINE: 'offline',  // Connection is offline or drive is unavailable.
  METERED: 'metered',  // Connection is metered. Should limit traffic.
  ONLINE: 'online'     // Connection is online.
};
Object.freeze(VolumeManagerCommon.DriveConnectionType);

/**
 * List of reasons of DriveConnectionType.
 *
 * Keep this in sync with the kDriveConnectionReason constants in
 * private_api_drive.cc.
 *
 * @enum {string}
 * @const
 */
VolumeManagerCommon.DriveConnectionReason = {
  NOT_READY: 'not_ready',    // Drive is not ready or authentication is failed.
  NO_NETWORK: 'no_network',  // Network connection is unavailable.
  NO_SERVICE: 'no_service'   // Drive service is unavailable.
};
Object.freeze(VolumeManagerCommon.DriveConnectionReason);

/**
 * The type of each volume.
 * @enum {string}
 * @const
 */
VolumeManagerCommon.VolumeType = {
  DRIVE: 'drive',
  DOWNLOADS: 'downloads',
  REMOVABLE: 'removable',
  ARCHIVE: 'archive',
  CLOUD_DEVICE: 'cloud_device',
  MTP: 'mtp',
  PROVIDED: 'provided'
};

/**
 * Returns if the volume is linux native file system or not. Non-native file
 * system does not support few operations (e.g. load unpacked extension).
 * @param {VolumeManagerCommon.VolumeType} type
 * @return {boolean}
 */
VolumeManagerCommon.VolumeType.isNative = function(type) {
  return type === VolumeManagerCommon.VolumeType.DOWNLOADS ||
      type === VolumeManagerCommon.VolumeType.REMOVABLE ||
      type === VolumeManagerCommon.VolumeType.ARCHIVE;
};

Object.freeze(VolumeManagerCommon.VolumeType);

/**
 * Obtains volume type from root type.
 * @param {VolumeManagerCommon.RootType} rootType RootType
 * @return {VolumeManagerCommon.VolumeType}
 */
VolumeManagerCommon.getVolumeTypeFromRootType = function(rootType) {
  switch (rootType) {
    case VolumeManagerCommon.RootType.DOWNLOADS:
      return VolumeManagerCommon.VolumeType.DOWNLOADS;
    case VolumeManagerCommon.RootType.ARCHIVE:
      return VolumeManagerCommon.VolumeType.ARCHIVE;
    case VolumeManagerCommon.RootType.REMOVABLE:
      return VolumeManagerCommon.VolumeType.REMOVABLE;
    case VolumeManagerCommon.RootType.DRIVE:
    case VolumeManagerCommon.RootType.DRIVE_OTHER:
    case VolumeManagerCommon.RootType.DRIVE_OFFLINE:
    case VolumeManagerCommon.RootType.DRIVE_SHARED_WITH_ME:
    case VolumeManagerCommon.RootType.DRIVE_RECENT:
      return VolumeManagerCommon.VolumeType.DRIVE;
    case VolumeManagerCommon.RootType.CLOUD_DEVICE:
      return VolumeManagerCommon.VolumeType.CLOUD_DEVICE;
    case VolumeManagerCommon.RootType.MTP:
      return VolumeManagerCommon.VolumeType.MTP;
    case VolumeManagerCommon.RootType.PROVIDED:
      return VolumeManagerCommon.VolumeType.PROVIDED;
  }
  assertNotReached('Unknown root type: ' + rootType);
};

/**
 * @typedef {{
 *   type: VolumeManagerCommon.DriveConnectionType,
 *   reason: VolumeManagerCommon.DriveConnectionReason
 * }}
 */
VolumeManagerCommon.DriveConnectionState;

/**
 * Interface for classes providing access to {@code VolumeInfo}
 * for {@code Entry} instances.
 *
 * @interface
 */
VolumeManagerCommon.VolumeInfoProvider = function() {};

/**
 * Obtains a volume info containing the passed entry.
 * @param {Entry|Object} entry Entry on the volume to be returned. Can be fake.
 * @return {?VolumeInfo} The VolumeInfo instance or null if not found.
 */
VolumeManagerCommon.VolumeInfoProvider.prototype.getVolumeInfo;

/**
 * Fake entries for Google Drive's virtual folders.
 * (OFFLINE, RECENT, and SHARED_WITH_ME)
 * @typedef {?{
 *   isDirectory: boolean,
 *   rootType: VolumeManagerCommon.RootType,
 *   toURL: function(): string
 * }}
 */
var FakeEntry;
