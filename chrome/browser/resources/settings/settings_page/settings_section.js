// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * 'cr-settings-section' shows a paper material themed section with a header
 * which shows its page title and icon.
 *
 * Example:
 *
 *    <cr-settings-section page-title="[[pageTitle]]" icon="[[icon]]">
 *      <!-- Insert your section controls here -->
 *    </cr-settings-section>
 *
 * @group Chrome Settings Elements
 * @element cr-settings-section
 */
Polymer({
  is: 'cr-settings-section',

  behaviors: [
    Polymer.NeonAnimationRunnerBehavior,
  ],

  properties: {
    /**
     * The current active route.
     */
    currentRoute: {
      type: Object,
      observer: 'currentRouteChanged_',
    },

    /**
     * The section is expanded to a full-page view when this property matches
     * currentRoute.section.
     */
    section: {
      type: String,
    },

    /**
     * Title for the page header and navigation menu.
     */
    pageTitle: String,

    /**
     * Name of the 'iron-icon' to show.
     */
    icon: String,

    /**
     * Container that determines the sizing of expanded sections.
     */
    expandContainer: {
      type: Object,
      notify: true,
    },

    animationConfig: {
      value: function() {
        return {
          expand: {
            name: 'expand-card-animation',
            node: this,
          },
          collapse: {
            name: 'collapse-card-animation',
            node: this,
          }
        };
      },
    },
  },

  /** @private */
  expanded_: false,

  /** @private */
  currentRouteChanged_: function() {
    var expanded = this.currentRoute.section == this.section;

    if (expanded == this.expanded_)
      return;

    this.expanded_ = expanded;
    this.playAnimation(expanded ? 'expand' : 'collapse');
  },
});

Polymer({
  is: 'expand-card-animation',

  behaviors: [
    Polymer.NeonAnimationBehavior
  ],

  configure: function(config) {
    var node = config.node;
    var containerRect = node.expandContainer.getBoundingClientRect();
    var nodeRect = node.getBoundingClientRect();

    // Save section's original height.
    node.unexpandedHeight = nodeRect.height;

    var headerHeight = node.$.header.getBoundingClientRect().height;
    var newTop = containerRect.top - headerHeight;
    var newHeight = containerRect.height + headerHeight;

    node.style.position = 'fixed';
    node.style.zIndex = '1';

    this._effect = new KeyframeEffect(node, [
      {'top': nodeRect.top + 'px', 'height': nodeRect.height + 'px'},
      {'top': newTop + 'px', 'height': newHeight + 'px'},
    ], this.timingFromConfig(config));
    return this._effect;
  },

  complete: function(config) {
    config.node.style.position = 'absolute';
    config.node.style.top =
        -config.node.$.header.getBoundingClientRect().height + 'px';
    config.node.style.bottom = 0;
  }
});

Polymer({
  is: 'collapse-card-animation',

  behaviors: [
    Polymer.NeonAnimationBehavior
  ],

  configure: function(config) {
    var node = config.node;

    var oldRect = node.getBoundingClientRect();

    // Temporarily set position to static to determine new height.
    node.style.position = '';
    var newTop = node.getBoundingClientRect().top;

    // TODO(tommycli): This value is undefined when the user navigates to a
    // subpage directly by URL instead of from the settings root. Find a better
    // method than using 200 as a dummy height.
    var newHeight = node.unexpandedHeight || 200;

    node.style.position = 'fixed';

    this._effect = new KeyframeEffect(node, [
      {'top': oldRect.top + 'px', 'height': oldRect.height + 'px'},
      {'top': newTop + 'px', 'height': newHeight + 'px'},
    ], this.timingFromConfig(config));
    return this._effect;
  },

  complete: function(config) {
    config.node.style.position = '';
    config.node.style.zIndex = '0';
  }
});
