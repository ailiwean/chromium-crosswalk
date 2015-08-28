// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element for displaying and modifying a list of cellular
 * access points.
 */
(function() {
'use strict';

Polymer({
  is: 'network-apnlist',

  properties: {
    /**
     * The current state for the network matching |guid|.
     * @type {?CrOnc.NetworkStateProperties}
     */
    networkState: {
      type: Object,
      value: null,
      observer: 'networkStateChanged_'
    },

    /**
     * The CrOnc.APNProperties.AccessPointName value of the selected APN.
     */
    selectedApn: {
      type: String,
      value: ''
    },

    /**
     * Selectable list of APN dictionaries for the UI. Includes an entry
     * corresponding to |otherApn| (see below).
     * @type {!Array<!CrOnc.APNProperties>}
     */
    apnSelectList: {
      type: Array,
      value: function() { return []; }
    },

    /**
     * The user settable properties for a new ('other') APN. The values for
     * AccessPointName, Username, and Password will be set to the currently
     * active APN if it does not match an existing list entry.
     * @type {?CrOnc.APNProperties}
     */
    otherApn: {
      type: Object,
      value: null
    },

    /**
     * Array of property names to pass to the Other APN property list.
     * @type {!Array<string>}
     */
    otherApnFields_: {
      type: Array,
      value: function() {
        return ['AccessPointName', 'Username', 'Password'];
      },
      readOnly: true
    },

    /**
     * Array of edit types to pass to the Other APN property list.
     */
    otherApnEditTypes_: {
      type: Object,
      value: function() {
        return {
          'AccessPointName': 'String',
          'Username': 'String',
          'Password': 'String'
        };
      },
      readOnly: true
    },
  },

  /** @const */ DefaultAccessPointName: 'none',

  /**
   * Polymer networkState changed method.
   */
  networkStateChanged_: function() {
    if (!this.networkState || !this.networkState.Cellular)
      return;

    var activeApn = null;
    var cellular = this.networkState.Cellular;
    if (cellular.APN && cellular.APN.AccessPointName)
      activeApn = cellular.APN;
    else if (cellular.LastGoodAPN && cellular.LastGoodAPN.AccessPointName)
      activeApn = cellular.LastGoodAPN;
    this.setApnSelectList_(activeApn);
  },

  /**
   * Sets the list of selectable APNs for the UI. Appends an 'Other' entry
   * (see comments for |otherApn| above).
   * @param {?CrOnc.APNProperties} activeApn The currently active APN value.
   * @private
   */
  setApnSelectList_: function(activeApn) {
    // Copy the list of APNs from this.networkState.
    var result = this.getApnList_().slice();

    // Test whether |activeApn| is in the current APN list in this.networkState.
    var activeApnInList = activeApn && result.some(
        function(a) { return a.AccessPointName == activeApn.AccessPointName; });

    // If |activeApn| is specified and not in the list, use the active
    // properties for 'other'. Otherwise use any existing 'other' properties.
    var otherApnProperties =
        (activeApn && !activeApnInList) ? activeApn : this.otherApn;
    var otherApn = this.createApnObject_(otherApnProperties);

    // Always use 'Other' for the name of custom APN entries (the name does
    // not get saved).
    otherApn.Name = 'Other';

    // If no 'active' or 'other' AccessPointName was provided, use the default.
    otherApn.AccessPointName =
        otherApn.AccessPointName || this.DefaultAccessPointName;

    // Save the 'other' properties.
    this.otherApn = otherApn;

    // Append 'other' to the end of the list of APNs.
    result.push(otherApn);

    this.set('apnSelectList', result);
    this.set(
        'selectedApn',
        (activeApn && activeApn.AccessPointName) || otherApn.AccessPointName);
  },

  /**
   * @param {?CrOnc.APNProperties=} apnProperties
   * @return {!CrOnc.APNProperties} A new APN object with properties from
   *     |apnProperties| if provided.
   * @private
   */
  createApnObject_: function(apnProperties) {
    var newApn = {AccessPointName: ''};
    if (apnProperties)
      Object.assign(newApn, apnProperties);
    return newApn;
  },

  /**
   * @return {!Array<!CrOnc.APNProperties>} The list of APN properties in
   *     |networkState| or an empty list if the property is not set.
   * @private
   */
  getApnList_: function() {
    var apnList = /** @type {Array<!CrOnc.APNProperties>|undefined} */(
        CrOnc.getActiveValue(this.networkState, 'Cellular.APNList'));
    return apnList || [];
  },

  /**
   * We need to update the select value after the dom-repeat template updates:
   * 1. Rebuilding the template options resets the select value property.
   * 2. The template update occurs after any property changed events.
   * TODO(stevenjb): Remove once we use cr-dropdown-menu which (hopefully)
   * won't require this.
   * @private
   */
  onSelectApnUpdated_: function() {
    this.$.selectApn.value = this.selectedApn;
  },

  /**
   * Event triggered when the selectApn selection changes.
   * @param {Event} event The select node change event.
   * @private
   */
  onSelectApnChange_: function(event) {
    var selectedApn = event.target.value;
    // When selecting 'Other', don't set a change event unless a valid
    // non-default value has been set for Other.
    if (this.isOtherSelected_(this.networkState, selectedApn) &&
        (!this.otherApn || !this.otherApn.AccessPointName ||
         this.otherApn.AccessPointName == this.DefaultAccessPointName)) {
      return;
    }
    this.sendApnChange_(selectedApn);
  },

  /**
   * Event triggered when any 'Other' APN network property changes.
   * @param {!{detail: {field: string, value: string}}} event
   * @private
   */
  onOtherApnChange_: function(event) {
    this.set('otherApn.' + event.detail.field, event.detail.value);
    // Don't send a change event for 'Other' until the 'Save' button is clicked.
  },

  /**
   * Event triggered when the Other APN 'Save' button is clicked.
   * @param {Event} event
   * @private
   */
  onSaveOther_: function(event) {
    this.sendApnChange_(this.selectedApn);
  },

  /**
   * Send the apn-change event.
   * @param {string} selectedApn
   * @private
   */
  sendApnChange_: function(selectedApn) {
    var apnList = this.getApnList_();
    var apn = this.findApnInList(apnList, selectedApn);
    if (apn == undefined) {
      apn = this.createApnObject_();
      if (this.otherApn) {
        apn.AccessPointName = this.otherApn.AccessPointName;
        apn.Username = this.otherApn.Username;
        apn.Password = this.otherApn.Password;
      }
    }
    this.fire('apn-change', {field: 'APN', value: apn});
  },

  /**
   * @param {?CrOnc.NetworkStateProperties} networkState
   * @param {string} selectedApn
   * @return {boolean} True if the 'other' APN is currently selected.
   * @private
   */
  isOtherSelected_: function(networkState, selectedApn) {
    if (!networkState || !networkState.Cellular)
      return false;
    var apnList = this.getApnList_();
    var apn = this.findApnInList(apnList, selectedApn);
    return apn == undefined;
  },

  /**
   * @param {!CrOnc.APNProperties} apn
   * @return {string} The most descriptive name for the access point.
   * @private
   */
  apnDesc_: function(apn) {
    return apn.LocalizedName || apn.Name || apn.AccessPointName;
  },

  /**
   * @param {!Array<!CrOnc.APNProperties>} apnList
   * @param {string} accessPointName
   * @return {CrOnc.APNProperties|undefined} The entry in |apnList| matching
   *     |accessPointName| if it exists, or undefined.
   * @private
   */
  findApnInList: function(apnList, accessPointName) {
    for (let a of apnList) {
      if (a.AccessPointName == accessPointName)
        return a;
    }
    return undefined;
  }
});
})();
