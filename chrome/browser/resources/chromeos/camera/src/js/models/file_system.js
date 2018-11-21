// Copyright 2018 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * Namespace for the Camera app.
 */
var cca = cca || {};

/**
 * Namespace for models.
 */
cca.models = cca.models || {};

/**
 * Creates the file system controller.
 * @constructor
 */
cca.models.FileSystem = function() {
  // End of properties, seal the object.
  Object.seal(this);
};

/**
 * The prefix of image files.
 * @type {string}
 * @const
 */
cca.models.FileSystem.IMAGE_PREFIX = 'IMG_';

/**
 * The prefix of video files.
 * @type {string}
 * @const
 */
cca.models.FileSystem.VIDEO_PREFIX = 'VID_';

/**
 * The prefix of thumbnail files.
 * @type {string}
 * @const
 */
cca.models.FileSystem.THUMBNAIL_PREFIX = 'thumb-';

/**
 * Internal file system.
 * @type {FileSystem}
 */
cca.models.FileSystem.internalFs = null;

/**
 * External file system.
 * @type {FileSystem}
 */
cca.models.FileSystem.externalFs = null;

/**
 * Initializes the internal file system.
 * @return {!Promise<FileSystem>} Promise for the internal file system.
 * @private
 */
cca.models.FileSystem.initInternalFs_ = function() {
  return new Promise((resolve, reject) => {
    webkitRequestFileSystem(
        window.PERSISTENT, 768 * 1024 * 1024 /* 768MB */, resolve, reject);
  });
};

/**
 * Initializes the external file system.
 * @return {!Promise<?FileSystem>} Promise for the external file system.
 * @private
 */
cca.models.FileSystem.initExternalFs_ = function() {
  return new Promise((resolve) => {
    if (!cca.util.isChromeOS()) {
      resolve(null);
      return;
    }
    chrome.fileSystem.getVolumeList((volumes) => {
      if (volumes) {
        for (var i = 0; i < volumes.length; i++) {
          if (volumes[i].volumeId.indexOf('downloads:Downloads') !== -1) {
            chrome.fileSystem.requestFileSystem(volumes[i], resolve);
            return;
          }
        }
      }
      resolve(null);
    });
  });
};

/**
 * Initializes file systems, migrating pictures if needed. This function
 * should be called only once in the beginning of the app.
 * @param {function()} promptMigrate Callback to instantiate a promise that
       prompts users to migrate pictures if no acknowledgement yet.
 * @return {!Promise} Promise for the operation.
 */
cca.models.FileSystem.initialize = function(promptMigrate) {
  var checkAcked = new Promise((resolve) => {
    // ackMigratePictures 1: User acknowledges to migrate pictures to Downloads.
    chrome.storage.local.get({ackMigratePictures: 0}, (values) => {
      resolve(values.ackMigratePictures >= 1);
    });
  });
  var checkMigrated = new Promise((resolve) => {
    if (chrome.chromeosInfoPrivate) {
      chrome.chromeosInfoPrivate.get(['cameraMediaConsolidated'], (values) => {
        resolve(values['cameraMediaConsolidated']);
      });
    } else {
      resolve(false);
    }
  });
  var ackMigrate = () => {
    chrome.storage.local.set({ackMigratePictures: 1});
  };
  var doneMigrate = () => {
    if (chrome.chromeosInfoPrivate) {
      chrome.chromeosInfoPrivate.set('cameraMediaConsolidated', true);
    }
  };

  return Promise.all([
    cca.models.FileSystem.initInternalFs_(),
    cca.models.FileSystem.initExternalFs_(),
    checkAcked,
    checkMigrated,
  ]).then(([internalFs, externalFs, acked, migrated]) => {
    cca.models.FileSystem.internalFs = internalFs;
    cca.models.FileSystem.externalFs = externalFs;
    if (migrated && !cca.models.FileSystem.externalFs) {
      throw 'External file system should be available.';
    }

    // Check if acknowledge-prompt and migrate-pictures are needed.
    if (migrated || !cca.models.FileSystem.externalFs) {
      return [false, false];
    } else {
      return cca.models.FileSystem.hasInternalPictures_().then((result) => {
        if (result) {
          return [!acked, true];
        } else {
          // If the external file system is supported and there is already no
          // picture in the internal file system, it implies done migration and
          // then doesn't need acknowledge-prompt.
          ackMigrate();
          doneMigrate();
          return [false, false];
        }
      });
    }
  }).then(([promptNeeded, migrateNeeded]) => {
    if (promptNeeded) {
      return promptMigrate().then(() => {
        ackMigrate();
        return migrateNeeded;
      });
    }
    return migrateNeeded;
  }).then((migrateNeeded) => {
    if (migrateNeeded) {
      return cca.models.FileSystem.migratePictures().then(doneMigrate);
    }
  });
};

/**
 * Reads file entries from the file system.
 * @param {FileSystem} fs File system to be read.
 * @return {!Promise<!Array.<FileEntry>>} Promise for the read file entries.
 * @private
 */
cca.models.FileSystem.readFs_ = function(fs) {
  return new Promise((resolve, reject) => {
    if (fs) {
      var dirReader = fs.root.createReader();
      var entries = [];
      var readEntries = () => {
        dirReader.readEntries((inEntries) => {
          if (inEntries.length == 0) {
            resolve(entries);
            return;
          }
          entries = entries.concat(inEntries);
          readEntries();
        }, reject);
      };
      readEntries();
    } else {
      resolve([]);
    }
  });
};

/**
 * Checks if there is an image or video in the internal file system.
 * @return {!Promise<boolean>} Promise for the result.
 * @private
 */
cca.models.FileSystem.hasInternalPictures_ = function() {
  var fs = cca.models.FileSystem.internalFs;
  return cca.models.FileSystem.readFs_(fs).then((entries) => {
    for (var index = entries.length - 1; index >= 0; index--) {
      // Check if there is any picture other than thumbnail in file entries.
      // Pictures taken by old Camera App may not have IMG_ or VID_ prefix.
      if (!cca.models.FileSystem.hasThumbnailPrefix_(entries[index])) {
        return true;
      }
    }
    return false;
  });
};

/**
 * Migrates all picture-files from internal storage to external storage.
 * @return {!Promise} Promise for the operation.
 */
cca.models.FileSystem.migratePictures = function() {
  var internalFs = cca.models.FileSystem.internalFs;
  var externalFs = cca.models.FileSystem.externalFs;

  var migratePicture = (pictureEntry, thumbnailEntry) => {
    var name = cca.models.FileSystem.regulatePictureName(pictureEntry);
    return cca.models.FileSystem.getFile_(
        externalFs, name, true).then((entry) => {
      return new Promise((resolve, reject) => {
        pictureEntry.copyTo(externalFs.root, entry.name, (result) => {
          if (result.name != pictureEntry.name && thumbnailEntry) {
            // Thumbnails can be recreated later if failing to rename them here.
            thumbnailEntry.moveTo(internalFs.root,
                cca.models.FileSystem.getThumbnailName(result));
          }
          pictureEntry.remove(() => {});
          resolve();
        }, reject);
      });
    });
  };

  return cca.models.FileSystem.readFs_(internalFs).then((internalEntries) => {
    var pictureEntries = [];
    var thumbnailEntriesByName = {};
    cca.models.FileSystem.parseInternalEntries_(
        internalEntries, thumbnailEntriesByName, pictureEntries);

    var migrated = [];
    for (var index = 0; index < pictureEntries.length; index++) {
      var entry = pictureEntries[index];
      var thumbnailName = cca.models.FileSystem.getThumbnailName(entry);
      var thumbnailEntry = thumbnailEntriesByName[thumbnailName];
      migrated.push(migratePicture(entry, thumbnailEntry));
    }
    return Promise.all(migrated);
  });
};

/**
 * Generates a file name for the picture.
 * @param {boolean} isVideo Picture is a video.
 * @param {number} time Timestamp of the picture in millisecond.
 * @return {string} Picture's file name.
 * @private
 */
cca.models.FileSystem.generatePictureName_ = function(isVideo, time) {
  const pad = (n) => (n < 10 ? '0' : '') + n;

  // File name base will be formatted as IMG/VID_yyyyMMdd_HHmmss.
  var prefix = isVideo ?
      cca.models.FileSystem.VIDEO_PREFIX :
      cca.models.FileSystem.IMAGE_PREFIX;
  var date = new Date(time);
  var result = prefix + date.getFullYear() + pad(date.getMonth() + 1) +
      pad(date.getDate()) + '_' + pad(date.getHours()) +
      pad(date.getMinutes()) + pad(date.getSeconds());

  return result + (isVideo ? '.mkv' : '.jpg');
};

/**
 * Regulates the picture name to the desired format if it's in legacy formats.
 * @param {FileEntry} entry Picture entry whose name to be regulated.
 * @return {string} Name in the desired format.
 */
cca.models.FileSystem.regulatePictureName = function(entry) {
  if (cca.models.FileSystem.hasVideoPrefix(entry) ||
      cca.models.FileSystem.hasImagePrefix_(entry)) {
    var match = entry.name.match(/(\w{3}_\d{8}_\d{6})(?:_(\d+))?(\..+)?$/);
    if (match) {
      var idx = match[2] ? ' (' + match[2] + ')' : '';
      var ext = match[3] ? match[3].replace(/\.webm$/, '.mkv') : '';
      return match[1] + idx + ext;
    }
  } else {
    // Early pictures are in legacy file name format (crrev.com/c/310064).
    var match = entry.name.match(/(\d+).(?:\d+)/);
    if (match) {
      return cca.models.FileSystem.generatePictureName_(
          false, parseInt(match[1], 10));
    }
  }
  return entry.name;
};

/**
 * Saves the blob to the given file name. Name of the actually saved file
 * might be different from the given file name if the file already exists.
 * @param {FileSystem} fs File system to be written.
 * @param {string} name Name of the file.
 * @param {Blob} blob Data of the file to be saved.
 * @return {!Promise<FileEntry>} Promise for the result.
 * @private
 */
cca.models.FileSystem.saveToFile_ = function(fs, name, blob) {
  return cca.models.FileSystem.getFile_(fs, name, true).then((entry) => {
    return new Promise((resolve, reject) => {
      entry.createWriter((fileWriter) => {
        fileWriter.onwriteend = () => {
          resolve(entry);
        };
        fileWriter.onerror = reject;
        fileWriter.write(blob);
      }, reject);
    });
  });
};

/**
 * Saves the picture into the external or internal file system.
 * @param {boolean} isVideo Picture is a video.
 * @param {Blob} blob Data of the picture to be saved.
 * @return {!Promise<FileEntry>} Promise for the result.
 */
cca.models.FileSystem.savePicture = function(isVideo, blob) {
  var fs = cca.models.FileSystem.externalFs || cca.models.FileSystem.internalFs;
  var name = cca.models.FileSystem.generatePictureName_(isVideo, Date.now());
  return cca.models.FileSystem.saveToFile_(fs, name, blob);
};

/**
 * Creates a thumbnail from the picture.
 * @param {boolean} isVideo Picture is a video.
 * @param {string} url Picture as an URL.
 * @return {!Promise<Blob>} Promise for the result.
 * @private
 */
cca.models.FileSystem.createThumbnail_ = function(isVideo, url) {
  const thumbnailWidth = 480;
  var element = document.createElement(isVideo ? 'video' : 'img');
  return new Promise((resolve, reject) => {
    if (isVideo) {
      element.onloadeddata = resolve;
    } else {
      element.onload = resolve;
    }
    element.onerror = reject;
    element.src = url;
  }).then(() => {
    var canvas = document.createElement('canvas');
    var context = canvas.getContext('2d');
    var ratio = isVideo ?
        element.videoHeight / element.videoWidth :
        element.height / element.width;
    var thumbnailHeight = Math.round(thumbnailWidth * ratio);
    canvas.width = thumbnailWidth;
    canvas.height = thumbnailHeight;
    context.drawImage(element, 0, 0, thumbnailWidth, thumbnailHeight);
    return new Promise((resolve, reject) => {
      canvas.toBlob((blob) => {
        if (blob) {
          resolve(blob);
        } else {
          reject('Failed to create thumbnail.');
        }
      }, 'image/jpeg');
    });
  });
};

/**
 * Gets the thumbnail name of the given picture.
 * @param {FileEntry} entry Picture's file entry.
 * @return {string} Thumbnail name.
 */
cca.models.FileSystem.getThumbnailName = function(entry) {
  var thumbnailName = cca.models.FileSystem.THUMBNAIL_PREFIX + entry.name;
  return (thumbnailName.substr(0, thumbnailName.lastIndexOf('.')) ||
      thumbnailName) + '.jpg';
};

/**
 * Creates and saves the thumbnail of the given picture.
 * @param {boolean} isVideo Picture is a video.
 * @param {FileEntry} entry Picture's file entry whose thumbnail to be saved.
 * @return {!Promise<FileEntry>} Promise for the result.
 */
cca.models.FileSystem.saveThumbnail = function(isVideo, entry) {
  return cca.models.FileSystem.pictureURL(entry).then((url) => {
    return cca.models.FileSystem.createThumbnail_(isVideo, url);
  }).then((blob) => {
    var thumbnailName = cca.models.FileSystem.getThumbnailName(entry);
    return cca.models.FileSystem.saveToFile_(
        cca.models.FileSystem.internalFs, thumbnailName, blob);
  });
};

/**
 * Checks if the entry's name has the video prefix.
 * @param {FileEntry} entry File entry.
 * @return {boolean} Has the video prefix or not.
 * @private
 */
cca.models.FileSystem.hasVideoPrefix = function(entry) {
  return entry.name.startsWith(cca.models.FileSystem.VIDEO_PREFIX);
};

/**
 * Checks if the entry's name has the image prefix.
 * @param {FileEntry} entry File entry.
 * @return {boolean} Has the image prefix or not.
 * @private
 */
cca.models.FileSystem.hasImagePrefix_ = function(entry) {
  return entry.name.startsWith(cca.models.FileSystem.IMAGE_PREFIX);
};

/**
 * Checks if the entry's name has the thumbnail prefix.
 * @param {FileEntry} entry File entry.
 * @return {boolean} Has the thumbnail prefix or not.
 * @private
 */
cca.models.FileSystem.hasThumbnailPrefix_ = function(entry) {
  return entry.name.startsWith(cca.models.FileSystem.THUMBNAIL_PREFIX);
};

/**
 * Parses and filters the internal entries to thumbnail and picture entries.
 * @param {Array.<FileEntry>} internalEntries Internal file entries.
 * @param {Object.<string, FileEntry>} thumbnailEntriesByName Result thumbanil
 *     entries mapped by thumbnail names, initially empty.
 * @param {Array.<FileEntry>=} pictureEntries Result picture entries,
 *     initially empty.
 * @private
 */
cca.models.FileSystem.parseInternalEntries_ = function(
    internalEntries, thumbnailEntriesByName, pictureEntries) {
  var isThumbnail = cca.models.FileSystem.hasThumbnailPrefix_;
  var thumbnailEntries = [];
  if (pictureEntries) {
    for (var index = 0; index < internalEntries.length; index++) {
      if (isThumbnail(internalEntries[index])) {
        thumbnailEntries.push(internalEntries[index]);
      } else {
        pictureEntries.push(internalEntries[index]);
      }
    }
  } else {
    thumbnailEntries = internalEntries.filter(isThumbnail);
  }
  for (var index = 0; index < thumbnailEntries.length; index++) {
    var thumbnailEntry = thumbnailEntries[index];
    thumbnailEntriesByName[thumbnailEntry.name] = thumbnailEntry;
  }
};

/**
 * Gets the picture and thumbnail entries.
 * @return {!Promise<!Array.<!Array.<FileEntry>|!Object.<string, FileEntry>>>}
 *     Promise for the picture entries and the thumbnail entries mapped by
 *     thumbnail names.
 */
cca.models.FileSystem.getEntries = function() {
  return Promise.all([
    cca.models.FileSystem.readFs_(cca.models.FileSystem.internalFs),
    cca.models.FileSystem.readFs_(cca.models.FileSystem.externalFs),
  ]).then(([internalEntries, externalEntries]) => {
    var pictureEntries = [];
    var thumbnailEntriesByName = {};

    if (cca.models.FileSystem.externalFs) {
      pictureEntries = externalEntries.filter((entry) => {
        if (!cca.models.FileSystem.hasVideoPrefix(entry) &&
            !cca.models.FileSystem.hasImagePrefix_(entry)) {
          return false;
        }
        return entry.name.match(/_(\d{8})_(\d{6})(?: \((\d+)\))?/);
      });
      cca.models.FileSystem.parseInternalEntries_(
          internalEntries, thumbnailEntriesByName);
    } else {
      cca.models.FileSystem.parseInternalEntries_(
          internalEntries, thumbnailEntriesByName, pictureEntries);
    }
    return [pictureEntries, thumbnailEntriesByName];
  });
};

/**
 * Returns an URL for a picture.
 * @param {FileEntry} entry File entry.
 * @return {!Promise<string>} Promise for the result.
 */
cca.models.FileSystem.pictureURL = function(entry) {
  return new Promise((resolve) => {
    if (cca.models.FileSystem.externalFs) {
      entry.file((file) => {
        resolve(URL.createObjectURL(file));
      });
    } else {
      resolve(entry.toURL());
    }
  });
};

/**
 * Gets the file by the given name, avoiding name conflicts if necessary.
 * @param {FileSystem} fs File system to get the file.
 * @param {string} name File name. Result file may have a different name.
 * @param {boolean} create True to create file, false otherwise.
 * @return {!Promise<?FileEntry>} Promise for the result.
 * @private
 */
cca.models.FileSystem.getFile_ = function(fs, name, create) {
  return new Promise((resolve, reject) => {
    var options = create ? {create: true, exclusive: true} : {create: false};
    fs.root.getFile(name, options, resolve, reject);
  }).catch((error) => {
    if (create && error.name == 'InvalidModificationError') {
      // Avoid name conflicts for creating files.
      return cca.models.FileSystem.getFile_(fs,
          cca.models.FileSystem.incrementFileName_(name), create);
    } else if (!create && error.name == 'NotFoundError') {
      return null;
    }
    throw error;
  });
};

/**
 * Increments the file index of a given file name to avoid name conflicts.
 * @param {string} name File name.
 * @return {string} File name with incremented index.
 * @private
 */
cca.models.FileSystem.incrementFileName_ = function(name) {
  var [base, ext] = ['', ''];
  var idx = 0;
  var match = name.match(/^([^.]+)(\..+)?$/);
  if (match) {
    base = match[1];
    ext = match[2];
    match = base.match(/ \((\d+)\)$/);
    if (match) {
      base = base.substring(0, match.index);
      idx = parseInt(match[1], 10);
    }
  }
  return base + ' (' + (idx + 1) + ')' + ext;
};
