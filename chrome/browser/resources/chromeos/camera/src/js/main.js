// Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * Namespace for the Camera app.
 */
var cca = cca || {};

/**
 * Creates the Camera App main object.
 * @param {number} aspectRatio Aspect ratio of app window when launched.
 * @constructor
 */
cca.App = function(aspectRatio) {
  /**
   * @type {cca.App.ViewsStack}
   * @private
   */
  this.viewsStack_ = new cca.App.ViewsStack();

  /**
   * @type {cca.Router}
   * @private
   */
  this.router_ = new cca.Router(
      this.navigateById_.bind(this),
      this.viewsStack_.pop.bind(this.viewsStack_));

  /**
   * @type {cca.views.Camera}
   * @private
   */
  this.cameraView_ = null;

  /**
   * @type {cca.views.Browser}
   * @private
   */
  this.browserView_ = null;

  /**
   * @type {cca.views.Dialog}
   * @private
   */
  this.dialogView_ = null;

  /**
   * @type {?number}
   * @private
   */
  this.resizeWindowTimeout_ = null;

  /**
   * @type {number}
   * @private
   */
  this.aspectRatio_ = aspectRatio;

  // End of properties. Seal the object.
  Object.seal(this);

  // Handle key presses to make the Camera app accessible via the keyboard.
  document.body.addEventListener('keydown', this.onKeyPressed_.bind(this));

  // Handle window resize.
  window.addEventListener('resize', this.onWindowResize_.bind(this, null));

  // Set the localized window title.
  document.title = chrome.i18n.getMessage('name');
};

/**
 * Creates a stack of views.
 * @constructor
 */
cca.App.ViewsStack = function() {
  /**
   * Stack with the views as well as return callbacks.
   * @type {Array.<Object>}
   * @private
   */
  this.stack_ = [];

  // No more properties. Seal the object.
  Object.seal(this);
};

cca.App.ViewsStack.prototype = {
  get current() {
    return this.stack_.length ? this.stack_[this.stack_.length - 1].view : null;
  },
  get all() {
    return this.stack_.map((entry) => entry.view);
  },
};

/**
 * Adds the view on the stack and hence makes it the current one. Optionally,
 * passes the arguments to the view.
 * @param {cca.View} view View to be pushed on top of the stack.
 * @param {Object=} opt_arguments Optional arguments.
 * @param {function(Object=)} opt_callback Optional result callback to be called
 *     when the view is closed.
 */
cca.App.ViewsStack.prototype.push = function(
    view, opt_arguments, opt_callback) {
  if (!view)
    return;
  if (this.current)
    this.current.inactivate();

  this.stack_.push({
    view: view,
    callback: opt_callback || function(result) {},
  });
  view.onResize();
  view.enter(opt_arguments);
  view.activate();
};

/**
 * Removes the current view from the stack and hence makes the previous one
 * the current one. Calls the callback passed to the previous view's navigate()
 * method with the result.
 * @param {Object=} opt_result Optional result. If not passed, then undefined
 *     will be passed to the callback.
 */
cca.App.ViewsStack.prototype.pop = function(opt_result) {
  var entry = this.stack_.pop();
  entry.view.inactivate();
  entry.view.leave();

  if (this.current)
    this.current.activate();
  if (entry.callback)
    entry.callback(opt_result);
};

/**
 * Starts the app by initializing views and showing the camera view.
 */
cca.App.prototype.start = function() {
  var model = new cca.models.Gallery();
  this.cameraView_ = new cca.views.Camera(
      this.router_, model, this.onWindowResize_.bind(this));
  this.browserView_ = new cca.views.Browser(this.router_, model);
  this.dialogView_ = new cca.views.Dialog(this.router_);

  var promptMigrate = () => {
    return new Promise((resolve, reject) => {
      this.router_.navigate(cca.Router.ViewIdentifier.DIALOG, {
        type: cca.views.Dialog.Type.ALERT,
        message: chrome.i18n.getMessage('migratePicturesMsg'),
      }, (result) => {
        if (!result.isPositive) {
          var error = new Error('Did not acknowledge migrate-prompt.');
          error.exitApp = true;
          reject(error);
          return;
        }
        resolve();
      });
    });
  };
  cca.models.FileSystem.initialize(promptMigrate).then(() => {
    // Prepare the views and model, and then make the app ready.
    this.cameraView_.prepare();
    this.browserView_.prepare();
    model.load([this.cameraView_.galleryButton, this.browserView_]);

    document.querySelectorAll('[tabindex]').forEach(
        (element) => cca.util.makeUnfocusableByMouse(element));
    cca.util.setupElementsAria();
    cca.tooltip.setup();
    this.router_.navigate(cca.Router.ViewIdentifier.CAMERA);
  }).catch((error) => {
    console.error(error);
    if (error && error.exitApp) {
      chrome.app.window.current().close();
      return;
    }
    cca.App.onError('filesystem-failure', 'errorMsgFileSystemFailed');
  });
};

/**
 * Switches the view using a router's view identifier.
 * @param {cca.Router.ViewIdentifier} viewIdentifier View identifier.
 * @param {Object=} opt_arguments Optional arguments for the view.
 * @param {function(Object=)} opt_callback Optional result callback to be called
 *     when the view is closed.
 * @private
 */
cca.App.prototype.navigateById_ = function(
    viewIdentifier, opt_arguments, opt_callback) {
  switch (viewIdentifier) {
    case cca.Router.ViewIdentifier.CAMERA:
      this.viewsStack_.push(this.cameraView_, opt_arguments, opt_callback);
      break;
    case cca.Router.ViewIdentifier.BROWSER:
      this.viewsStack_.push(this.browserView_, opt_arguments, opt_callback);
      break;
    case cca.Router.ViewIdentifier.DIALOG:
      this.viewsStack_.push(this.dialogView_, opt_arguments, opt_callback);
      break;
  }
};

/**
 * Resizes the window to match the last known aspect ratio if applicable.
 * @private
 */
cca.App.prototype.resizeByAspectRatio_ = function() {
  // Don't update window size if it's maximized or fullscreen.
  if (cca.util.isWindowFullSize()) {
    return;
  }

  // Keep the width fixed and calculate the height by the aspect ratio.
  // TODO(yuli): Update min-width for resizing at portrait orientation.
  var appWindow = chrome.app.window.current();
  var inner = appWindow.innerBounds;
  var innerW = inner.minWidth;
  var innerH = Math.round(innerW / this.aspectRatio_);

  // Limit window resizing capability by setting min-height. Don't limit
  // max-height here as it may disable maximize/fullscreen capabilities.
  // TODO(yuli): Revise if there is an alternative fix.
  inner.minHeight = innerH;

  inner.width = innerW;
  inner.height = innerH;
};

/**
 * Handles resizing window/views for size or aspect ratio changes.
 * @param {number=} aspectRatio Aspect ratio changed.
 * @private
 */
cca.App.prototype.onWindowResize_ = function(aspectRatio) {
  if (this.resizeWindowTimeout_) {
    clearTimeout(this.resizeWindowTimeout_);
    this.resizeWindowTimeout_ = null;
  }
  if (aspectRatio) {
    // Update window size immediately for changed aspect ratio.
    this.aspectRatio_ = aspectRatio;
    this.resizeByAspectRatio_();
  } else {
    // Don't further resize window during resizing for smooth UX.
    this.resizeWindowTimeout_ = setTimeout(() => {
      this.resizeWindowTimeout_ = null;
      this.resizeByAspectRatio_();
    }, 500);
  }

  // Resize all stacked views rather than just the current-view to avoid
  // camera-preview not being resized if a dialog or settings' menu is shown on
  // top of the camera-view.
  this.viewsStack_.all.forEach((view) => {
    view.onResize();
  });
};

/**
 * Handles pressed keys.
 * @param {Event} event Key press event.
 * @private
 */
cca.App.prototype.onKeyPressed_ = function(event) {
  cca.tooltip.hide(); // Hide shown tooltip on any keypress.
  if (cca.util.getShortcutIdentifier(event) == 'BrowserBack') {
    chrome.app.window.current().minimize();
    return;
  }

  var currentView = this.viewsStack_.current;
  if (currentView && !document.body.classList.contains('has-error')) {
    currentView.onKeyPressed(event);
  }
};

/**
 * Shows an error message.
 * @param {string} identifier Identifier of the error.
 * @param {string} message Message for the error.
 */
cca.App.onError = function(identifier, message) {
  // TODO(yuli): Implement error-identifier to look up messages/hints and handle
  // multiple errors. Make 'error' a view to block buttons on other views.
  document.body.classList.add('has-error');
  // Use setTimeout to wait for error-view to be visible by screen reader.
  setTimeout(() => {
    document.querySelector('#error-msg').textContent =
        chrome.i18n.getMessage(message) || message;
  }, 0);
};

/**
 * Removes the error message when an error goes away.
 * @param {string} identifier Identifier of the error.
 */
cca.App.onErrorRecovered = function(identifier) {
  document.body.classList.remove('has-error');
  document.querySelector('#error-msg').textContent = '';
};

/**
 * @type {cca.App} Singleton of the Camera object.
 * @private
 */
cca.App.instance_ = null;

/**
 * Creates the Camera object and starts screen capturing.
 */
document.addEventListener('DOMContentLoaded', () => {
  var appWindow = chrome.app.window.current();
  if (!cca.App.instance_) {
    var inner = appWindow.innerBounds;
    cca.App.instance_ = new cca.App(inner.width / inner.height);
  }
  cca.App.instance_.start();
  appWindow.show();
});
