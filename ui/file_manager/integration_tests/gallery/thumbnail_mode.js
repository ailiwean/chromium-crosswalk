// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Renames an image in thumbnail mode and confirms that thumbnail of renamed
 * image is successfully updated.
 * @param {string} testVolumeName Test volume name.
 * @param {VolumeManagerCommon.VolumeType} volumeType Volume type.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
function renameImageInThumbnailMode(testVolumeName, volumeType) {
  var launchedPromise = launch(testVolumeName, volumeType,
      [ENTRIES.desktop, ENTRIES.image3], [ENTRIES.desktop]);
  var appId;
  return launchedPromise.then(function(result) {
    // Confirm initial state after the launch.
    appId = result.appId;
    return gallery.waitForSlideImage(appId, 800, 600, 'My Desktop Background');
  }).then(function() {
    // Goes to thumbnail mode.
    return gallery.waitAndClickElement(appId, 'button.mode');
  }).then(function() {
    return gallery.selectImageInThumbnailMode(appId, 'image3.jpg');
  }).then(function(result) {
    chrome.test.assertTrue(result);
    return gallery.callRemoteTestUtil('changeName', appId, ['New Image Name']);
  }).then(function() {
    // Assert that rename had done successfully.
    return gallery.waitForAFile(volumeType, 'New Image Name.jpg');
  }).then(function() {
    return gallery.selectImageInThumbnailMode(
        appId, 'My Desktop Background.png');
  }).then(function(result) {
    chrome.test.assertTrue(result);
    return gallery.callRemoteTestUtil('queryAllElements', appId,
        ['.thumbnail-view > ul > li.selected']);
  }).then(function(result) {
    // Only My Desktop Background.png is selected.
    chrome.test.assertEq(1, result.length);

    chrome.test.assertEq('My Desktop Background.png',
        result[0].attributes['title']);
    return gallery.callRemoteTestUtil('queryAllElements', appId,
        ['.thumbnail-view > ul > li:not(.selected)']);
  }).then(function(result) {
    // Confirm that thumbnail of renamed image has updated.
    chrome.test.assertEq(1, result.length);
    chrome.test.assertEq('New Image Name.jpg',
        result[0].attributes['title']);
  });
};

/**
 * Delete all images in thumbnail mode and confirm that no-images error banner
 * is shown.
 * @param {string} testVolumeName Test volume name.
 * @param {VolumeManagerCommon.VolumeType} volumeType Volume type.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
function deleteAllImagesInThumbnailMode(testVolumeName, volumeType) {
  var launchedPromise = launch(testVolumeName, volumeType,
      [ENTRIES.desktop, ENTRIES.image3]);
  var appId;
  return launchedPromise.then(function(result) {
    appId = result.appId;
    // Click delete button.
    return gallery.waitAndClickElement(appId, 'paper-button.delete');
  }).then(function(result) {
    chrome.test.assertTrue(!!result);
    // Wait and click delete button of confirmation dialog.
    return gallery.waitAndClickElement(appId, '.cr-dialog-ok');
  }).then(function(result) {
    chrome.test.assertTrue(!!result);
    // Wait until error banner is shown.
    return gallery.waitForElement(appId, '.gallery[error] .error-banner');
  });
}

/**
 * Clicks an empty space in thumbnail view and confirms that current selection
 * is unselected.
 * @param {string} testVolumeName Test volume name.
 * @param {VolumeManagerCommon.VolumeType} volumeType Volume type.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
function emptySpaceClickUnselectsInThumbnailMode(testVolumeName, volumeType) {
  var launchedPromise = launch(testVolumeName, volumeType,
      [ENTRIES.desktop, ENTRIES.image3], [ENTRIES.desktop]);
  var appId;
  return launchedPromise.then(function(result) {
    // Confirm initial state after the launch.
    appId = result.appId;
    return gallery.waitForSlideImage(appId, 800, 600, 'My Desktop Background');
  }).then(function(result) {
    // Switch to thumbnail mode.
    return gallery.waitAndClickElement(appId, 'button.mode');
  }).then(function(result) {
    // Confirm My Desktop Background.png is selected in thumbnail view.
    return gallery.callRemoteTestUtil('queryAllElements', appId,
        ['.thumbnail-view > ul > li.selected']);
  }).then(function(results) {
    chrome.test.assertEq(1, results.length);
    chrome.test.assertEq('My Desktop Background.png',
        results[0].attributes['title']);
    // Click empty space of thumbnail view.
    return gallery.waitAndClickElement(appId, '.thumbnail-view > ul');
  }).then(function(result) {
    // Confirm no image is selected.
    return gallery.callRemoteTestUtil('queryAllElements', appId,
        ['.thumbnail-view > ul > li.selected']);
  }).then(function(results) {
    chrome.test.assertEq(0, results.length);
    // Confirm delete button is disabled.
    return gallery.waitForElement(appId, 'paper-button.delete[disabled]');
  }).then(function(result) {
    // Confirm slideshow button is disabled.
    return gallery.waitForElement(appId, 'paper-button.slideshow[disabled]');
  }).then(function() {
    // Switch back to slide mode by clicking mode button.
    return gallery.waitAndClickElement(appId, 'button.mode:not([disabled])');
  }).then(function(result) {
    // First image in the image set (image3) should be shown.
    return gallery.waitForSlideImage(appId, 640, 480, 'image3');
  });
}

/**
 * Rename test in thumbnail mode for Downloads.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
testcase.renameImageInThumbnailModeOnDownloads = function() {
  return renameImageInThumbnailMode('local', 'downloads');
};

/**
 * Rename test in thumbnail mode for Drive.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
testcase.renameImageInThumbnailModeOnDrive = function() {
  return renameImageInThumbnailMode('drive', 'drive');
};

/**
 * Delete all images test in thumbnail mode for Downloads.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
testcase.deleteAllImagesInThumbnailModeOnDownloads = function() {
  return deleteAllImagesInThumbnailMode('local', 'downloads');
};

/**
 * Delete all images test in thumbnail mode for Drive.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
testcase.deleteAllImagesInThumbnailModeOnDrive = function() {
  return deleteAllImagesInThumbnailMode('drive', 'drive');
};

/**
 * Empty space click unselects current selection in thumbnail mode for
 * Downloads.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
testcase.emptySpaceClickUnselectsInThumbnailModeOnDownloads = function() {
  return emptySpaceClickUnselectsInThumbnailMode('local', 'downloads');
};

/**
 * Empty space click unselects current selection in thumbnail mode for Drive.
 * @return {!Promise} Promise to be fulfilled with on success.
 */
testcase.emptySpaceClickUnselectsInThumbnailModeOnDrive = function() {
  return emptySpaceClickUnselectsInThumbnailMode('drive', 'drive');
};
