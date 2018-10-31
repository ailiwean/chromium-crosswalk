// Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * Namespace for the Camera app.
 */
var camera = camera || {};

/**
 * Namespace for models.
 */
camera.models = camera.models || {};

/**
 * Creates the Gallery view controller.
 * @constructor
 */
camera.models.Gallery = function() {
  /**
   * @type {Array.<camera.models.Gallery.Observer>}
   * @private
   */
  this.observers_ = [];

  /**
   * @type {Promise<Array.<camera.models.Gallery.Picture>>}
   * @private
   */
  this.loaded_ = null;

  // End of properties, seal the object.
  Object.seal(this);
};

/**
 * Wraps an image/video and its thumbnail as a picture in the model.
 * @param {FileEntry} thumbnailEntry Thumbnail file entry.
 * @param {FileEntry} pictureEntry Picture file entry.
 * @param {boolean} isMotionPicture True if it's a motion picture (video),
 *     false it's a still picture (image).
 * @constructor
 */
camera.models.Gallery.Picture = function(
    thumbnailEntry, pictureEntry, isMotionPicture) {
  /**
   * @type {?FileEntry}
   * @private
   */
  this.thumbnailEntry_ = thumbnailEntry;

  /**
   * @type {FileEntry}
   * @private
   */
  this.pictureEntry_ = pictureEntry;

  /**
   * @type {boolean}
   * @private
   */
  this.isMotionPicture_ = isMotionPicture;

  /**
   * @type {Date}
   * @private
   */
  this.timestamp_ = camera.models.Gallery.Picture.parseTimestamp_(pictureEntry);

  // End of properties. Freeze the object.
  Object.freeze(this);
};

/**
 * Gets a picture's timestamp from its name.
 * @param {FileEntry} pictureEntry Picture file entry.
 * @return {Date} Picture timestamp.
 * @private
 */
camera.models.Gallery.Picture.parseTimestamp_ = function(pictureEntry) {
  var num = function(str) {
    return parseInt(str, 10);
  };

  var name = camera.models.FileSystem.regulatePictureName(pictureEntry);
  // Match numeric parts from filenames, e.g. IMG_'yyyyMMdd_HHmmss (n)'.jpg.
  // Assume no more than one picture taken within one millisecond.
  var match = name.match(
      /_(\d{4})(\d{2})(\d{2})_(\d{2})(\d{2})(\d{2})(?: \((\d+)\))?/);
  return match ? new Date(num(match[1]), num(match[2]) - 1, num(match[3]),
      num(match[4]), num(match[5]), num(match[6]),
      match[7] ? num(match[7]) : 0) : new Date(0);
};

camera.models.Gallery.Picture.prototype = {
  // Assume pictures always have different names as URL API may still point to
  // the deleted file for new files created with the same name.
  get thumbnailURL() {
    return this.thumbnailEntry_ && this.thumbnailEntry_.toURL();
  },
  get pictureEntry() {
    return this.pictureEntry_;
  },
  get isMotionPicture() {
    return this.isMotionPicture_;
  },
  get timestamp() {
    return this.timestamp_;
  },
};

/**
 * Creates and returns an URL for a picture.
 * @return {!Promise<string>} Promise for the result.
 */
camera.models.Gallery.Picture.prototype.pictureURL = function() {
  return camera.models.FileSystem.pictureURL(this.pictureEntry_);
};

/**
 * Observer interface for the pictures' model changes.
 * @constructor
 */
camera.models.Gallery.Observer = function() {
};

/**
 * Notifies about a deleted picture.
 * @param {camera.models.Gallery.Picture} picture Picture deleted.
 */
camera.models.Gallery.Observer.prototype.onPictureDeleted = function(picture) {
};

/**
 * Notifies about an added picture.
 * @param {camera.models.Gallery.Picture} picture Picture added.
 */
camera.models.Gallery.Observer.prototype.onPictureAdded = function(picture) {
};

/**
 * Loads the model.
 * @param {Array.<camera.models.Gallery.Observer>} observers Observers for
 *     the pictures' model changes.
 */
camera.models.Gallery.prototype.load = function(observers) {
  this.observers_ = observers;
  this.loaded_ = camera.models.FileSystem.getEntries().then(
      ([pictureEntries, thumbnailEntriesByName]) => {
    return this.loadStoredPictures_(pictureEntries, thumbnailEntriesByName);
  });

  this.loaded_.then(pictures => {
    pictures.forEach(picture => {
      this.notifyObservers_('onPictureAdded', picture);
    });
  }).catch(error => {
    console.warn(error);
  });
};

/**
 * Loads the pictures from the storages and adds them to the pictures model.
 * @param {Array.<FileEntry>} pictureEntries Picture entries.
 * @param {Object{string, FileEntry}} thumbnailEntriesByName Thumbanil entries
 *     mapped by thumbnail names.
 * @return {!Promise<Array.<camera.models.Gallery.Picture>>} Promise for the
 *     pictures.
 * @private
 */
camera.models.Gallery.prototype.loadStoredPictures_ = function(
    pictureEntries, thumbnailEntriesByName) {
  var wrapped = pictureEntries.filter(entry => entry.name).map(entry => {
    // Create the thumbnail if it's not cached. Ignore errors since it is
    // better to load something than nothing.
    // TODO(yuli): Remove unused thumbnails.
    var thumbnailName = camera.models.FileSystem.getThumbnailName(entry);
    var thumbnailEntry = thumbnailEntriesByName[thumbnailName];
    return this.wrapPicture_(entry, thumbnailEntry);
  });

  return Promise.all(wrapped).then(pictures => {
    // Sort pictures by timestamps. The most recent picture will be at the end.
    return pictures.sort((a, b) => {
      if (a.timestamp == null) {
        return -1;
      }
      if (b.timestamp == null) {
        return 1;
      }
      return a.timestamp - b.timestamp;
    });
  });
};

/**
 * Gets the last picture of the loaded pictures' model.
 * @return {!Promise<camera.models.Gallery.Picture>} Promise for the result.
 */
camera.models.Gallery.prototype.lastPicture = function() {
  return this.loaded_.then(pictures => {
    return pictures[pictures.length - 1];
  });
};

/**
 * Checks and updates the last picture of the loaded pictures' model.
 * @return {!Promise<camera.models.Gallery.Picture>} Promise for the result.
 */
camera.models.Gallery.prototype.checkLastPicture = function() {
  return this.lastPicture().then(picture => {
    // Assume only external pictures were removed without updating the model.
    if (camera.models.FileSystem.externalFs && picture) {
      var name = picture.pictureEntry.name;
      return camera.models.FileSystem.getFile_(
          camera.models.FileSystem.externalFs, name, false).then(entry => {
        return [picture, (entry != null)];
      });
    } else {
      return [picture, (picture != null)];
    }
  }).then(([picture, pictureEntryExist]) => {
    if (pictureEntryExist || !picture) {
      return picture;
    }
    return this.deletePicture(picture, true).then(
        this.checkLastPicture.bind(this));
  });
};

/**
 * Deletes the picture in the pictures' model.
 * @param {camera.models.Gallery.Picture} picture Picture to be deleted.
 * @param {boolean=} pictureEntryDeleted Whether the picture-entry was deleted.
 * @return {!Promise<>} Promise for the operation.
 */
camera.models.Gallery.prototype.deletePicture = function(
    picture, pictureEntryDeleted) {
  var removed = new Promise((resolve, reject) => {
    if (pictureEntryDeleted) {
      resolve();
    } else {
      picture.pictureEntry.remove(resolve, reject);
    }
  });
  return Promise.all([this.loaded_, removed]).then(([pictures, _]) => {
    var removal = pictures.indexOf(picture);
    if (removal != -1) {
      pictures.splice(removal, 1);
    }
    this.notifyObservers_('onPictureDeleted', picture);
    if (picture.thumbnailEntry_) {
      picture.thumbnailEntry_.remove(() => {});
    }
  });
};

/**
 * Exports the picture to the external storage.
 * @param {camera.models.Gallery.Picture} picture Picture to be exported.
 * @param {FileEntry} entry Target file entry.
 * @return {!Promise<>} Promise for the operation.
 */
camera.models.Gallery.prototype.exportPicture = function(picture, entry) {
  return new Promise((resolve, reject) => {
    entry.getParent(directory => {
      picture.pictureEntry.copyTo(directory, entry.name, resolve, reject);
    }, reject);
  });
};

/**
 * Wraps file entries as a picture for the pictures' model.
 * @param {FileEntry} pictureEntry Picture file entry.
 * @param {FileEntry=} thumbnailEntry Thumbnail file entry.
 * @return {!Promise<camera.models.Gallery.Picture>} Promise for the picture.
 * @private
 */
camera.models.Gallery.prototype.wrapPicture_ = function(
    pictureEntry, thumbnailEntry) {
  var isMotionPicture = camera.models.FileSystem.hasVideoPrefix(pictureEntry);
  var saved = () => {
    // Proceed to wrap the picture even if unable to save its thumbnail.
    return camera.models.FileSystem.saveThumbnail(
        isMotionPicture, pictureEntry).catch(() => null);
  };
  return Promise.resolve(thumbnailEntry || saved()).then(thumbnailEntry => {
    return new camera.models.Gallery.Picture(
        thumbnailEntry, pictureEntry, isMotionPicture);
  });
};

/**
 * Notifies observers about the added or deleted picture.
 * @param {string} fn Observers' callback function name.
 * @param {camera.models.Gallery.Picture} picture Picture added or deleted.
 * @private
 */
camera.models.Gallery.prototype.notifyObservers_ = function(fn, picture) {
  for (var i = 0; i < this.observers_.length; i++) {
    this.observers_[i][fn](picture);
  }
};

/**
 * Saves a picture that will also be added to the pictures' model.
 * @param {Blob} blob Data of the picture to be added.
 * @param {boolean} isMotionPicture Picture to be added is a video.
 * @return {!Promise<>} Promise for the operation.
 */
camera.models.Gallery.prototype.savePicture = function(blob, isMotionPicture) {
  // TODO(yuli): models.Gallery listens to models.FileSystem's file-added event
  // and then add a new picture into the model.
  var saved = new Promise(resolve => {
    if (isMotionPicture) {
      resolve(blob);
    } else {
      // Ignore errors since it is better to save something than nothing.
      // TODO(yuli): Support showing images by EXIF orientation instead.
      camera.util.orientPhoto(blob, resolve, () => {
        resolve(blob);
      });
    }
  }).then(blob => {
    return camera.models.FileSystem.savePicture(isMotionPicture, blob);
  }).then(pictureEntry => {
    return this.wrapPicture_(pictureEntry);
  });
  return Promise.all([this.loaded_, saved]).then(([pictures, picture]) => {
    // Insert the picture into the sorted pictures' model.
    for (var index = pictures.length - 1; index >= 0; index--) {
      if (picture.timestamp >= pictures[index].timestamp) {
        break;
      }
    }
    pictures.splice(index + 1, 0, picture);
    this.notifyObservers_('onPictureAdded', picture);
  });
};
