// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @suppress {duplicate} */
var remoting = remoting || {};

(function(){

'use strict';

// TODO(kelvinp): Update the url to point to the telemetry front-end.
var LOGGING_URL = '';

/**
 * @constructor
 */
var BackgroundPage = function() {
  /** @private {remoting.AppLauncher} */
  this.appLauncher_ = null;
  /** @private {remoting.ActivationHandler} */
  this.activationHandler_ = null;
  /** @private {remoting.TelemetryEventWriter.Service} */
  this.telemetryService_ = null;
  /** @private */
  this.disposables_ = new base.Disposables();
  this.preInit_();
};

/**
 * Initialize members and globals that are valid throughout the entire lifetime
 * of the background page.
 *
 * @private
 */
BackgroundPage.prototype.preInit_ = function() {
  remoting.settings = new remoting.Settings();
  if (base.isAppsV2()) {
    remoting.identity = new remoting.Identity();
  } else {
    remoting.oauth2 = new remoting.OAuth2();
    var oauth2 = /** @type {*} */ (remoting.oauth2);
    remoting.identity = /** @type {remoting.Identity} */ (oauth2);
  }

  if (base.isAppsV2()) {
    this.appLauncher_ = new remoting.V2AppLauncher();
    this.activationHandler_ = new remoting.ActivationHandler(
        base.Ipc.getInstance(), this.appLauncher_);
    this.telemetryService_ = new remoting.TelemetryEventWriter.Service(
        base.Ipc.getInstance(),
        new remoting.XhrEventWriter(LOGGING_URL, chrome.storage.local,
                                    'pending-log-requests'));
    this.telemetryService_.init();
    this.disposables_.add(new base.EventHook(
        this.activationHandler_, remoting.ActivationHandler.Events.windowClosed,
        this.telemetryService_.unbindSession.bind(this.telemetryService_)));
  } else {
    this.appLauncher_ = new remoting.V1AppLauncher();
  }
};


window.addEventListener('load', function() {
  remoting.backgroundPage = new BackgroundPage();
}, false);

}());
