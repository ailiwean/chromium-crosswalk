// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {

'use strict';

/** @param {Event} event */
function onClick(event) {
  var element = /** @type {HTMLElement} */ (event.target);
  if (element.classList.contains('toggle-licence')) {
    element.classList.toggle('hide-licence');
  }
}

document.addEventListener('click', onClick, false);

})();
