// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This module implements WebView (<webview>) as a custom element that wraps a
// BrowserPlugin object element. The object element is hidden within
// the shadow DOM of the WebView element.

var DocumentNatives = requireNative('document_natives');
var GuestViewInternal =
    require('binding').Binding.create('guestViewInternal').generate();
var IdGenerator = requireNative('id_generator');
// TODO(lazyboy): Rename this to WebViewInternal and call WebViewInternal
// something else.
var WebView = require('webViewInternal').WebView;
var WebViewEvents = require('webViewEvents').WebViewEvents;
var guestViewInternalNatives = requireNative('guest_view_internal');

// Attributes.
var WEB_VIEW_ATTRIBUTE_ALLOWTRANSPARENCY = 'allowtransparency';
var WEB_VIEW_ATTRIBUTE_AUTOSIZE = 'autosize';
var WEB_VIEW_ATTRIBUTE_MAXHEIGHT = 'maxheight';
var WEB_VIEW_ATTRIBUTE_MAXWIDTH = 'maxwidth';
var WEB_VIEW_ATTRIBUTE_MINHEIGHT = 'minheight';
var WEB_VIEW_ATTRIBUTE_MINWIDTH = 'minwidth';
var WEB_VIEW_ATTRIBUTE_PARTITION = 'partition';
var AUTO_SIZE_ATTRIBUTES = [
  WEB_VIEW_ATTRIBUTE_AUTOSIZE,
  WEB_VIEW_ATTRIBUTE_MAXHEIGHT,
  WEB_VIEW_ATTRIBUTE_MAXWIDTH,
  WEB_VIEW_ATTRIBUTE_MINHEIGHT,
  WEB_VIEW_ATTRIBUTE_MINWIDTH
];

// Error messages.
var ERROR_MSG_ALREADY_NAVIGATED =
    'The object has already navigated, so its partition cannot be changed.';
var ERROR_MSG_CANNOT_INJECT_SCRIPT = '<webview>: ' +
    'Script cannot be injected into content until the page has loaded.';
var ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE = '<webview>: ' +
    'contentWindow is not available at this time. It will become available ' +
    'when the page has finished loading.';
var ERROR_MSG_INVALID_PARTITION_ATTRIBUTE = 'Invalid partition attribute.';

// Represents the state of the storage partition.
function Partition() {
  this.validPartitionId = true;
  this.persistStorage = false;
  this.storagePartitionId = '';
}

Partition.prototype.toAttribute = function() {
  if (!this.validPartitionId) {
    return '';
  }
  return (this.persistStorage ? 'persist:' : '') + this.storagePartitionId;
};

Partition.prototype.fromAttribute = function(value, hasNavigated) {
  var result = {};
  if (hasNavigated) {
    result.error = ERROR_MSG_ALREADY_NAVIGATED;
    return result;
  }
  if (!value) {
    value = '';
  }

  var LEN = 'persist:'.length;
  if (value.substr(0, LEN) == 'persist:') {
    value = value.substr(LEN);
    if (!value) {
      this.validPartitionId = false;
      result.error = ERROR_MSG_INVALID_PARTITION_ATTRIBUTE;
      return result;
    }
    this.persistStorage = true;
  } else {
    this.persistStorage = false;
  }

  this.storagePartitionId = value;
  return result;
};

// Represents the internal state of the WebView node.
function WebViewInternal(webviewNode) {
  privates(webviewNode).internal = this;
  this.webviewNode = webviewNode;
  this.attached = false;
  this.pendingGuestCreation = false;
  this.elementAttached = false;

  this.beforeFirstNavigation = true;
  this.contentWindow = null;
  this.validPartitionId = true;
  // Used to save some state upon deferred attachment.
  // If <object> bindings is not available, we defer attachment.
  // This state contains whether or not the attachment request was for
  // newwindow.
  this.deferredAttachState = null;

  // on* Event handlers.
  this.on = {};

  this.browserPluginNode = this.createBrowserPluginNode();
  var shadowRoot = this.webviewNode.createShadowRoot();
  this.partition = new Partition();

  this.setupWebViewSrcAttributeMutationObserver();
  this.setupFocusPropagation();
  this.setupWebviewNodeProperties();

  this.viewInstanceId = IdGenerator.GetNextId();

  new WebViewEvents(this, this.viewInstanceId);

  shadowRoot.appendChild(this.browserPluginNode);
}

WebViewInternal.prototype.createBrowserPluginNode = function() {
  // We create BrowserPlugin as a custom element in order to observe changes
  // to attributes synchronously.
  var browserPluginNode = new WebViewInternal.BrowserPlugin();
  privates(browserPluginNode).internal = this;
  return browserPluginNode;
};

WebViewInternal.prototype.getGuestInstanceId = function() {
  return this.guestInstanceId;
};

// Resets some state upon reattaching <webview> element to the DOM.
WebViewInternal.prototype.reset = function() {
  // If guestInstanceId is defined then the <webview> has navigated and has
  // already picked up a partition ID. Thus, we need to reset the initialization
  // state. However, it may be the case that beforeFirstNavigation is false BUT
  // guestInstanceId has yet to be initialized. This means that we have not
  // heard back from createGuest yet. We will not reset the flag in this case so
  // that we don't end up allocating a second guest.
  if (this.guestInstanceId) {
    GuestViewInternal.destroyGuest(this.guestInstanceId);
    this.guestInstanceId = undefined;
    this.beforeFirstNavigation = true;
    this.validPartitionId = true;
    this.partition.validPartitionId = true;
    this.contentWindow = null;
  }
  this.internalInstanceId = 0;
};

// Sets the <webview>.request property.
WebViewInternal.prototype.setRequestPropertyOnWebViewNode = function(request) {
  Object.defineProperty(
      this.webviewNode,
      'request',
      {
        value: request,
        enumerable: true
      }
  );
};

WebViewInternal.prototype.setupFocusPropagation = function() {
  if (!this.webviewNode.hasAttribute('tabIndex')) {
    // <webview> needs a tabIndex in order to be focusable.
    // TODO(fsamuel): It would be nice to avoid exposing a tabIndex attribute
    // to allow <webview> to be focusable.
    // See http://crbug.com/231664.
    this.webviewNode.setAttribute('tabIndex', -1);
  }
  this.webviewNode.addEventListener('focus', function(e) {
    // Focus the BrowserPlugin when the <webview> takes focus.
    this.browserPluginNode.focus();
  }.bind(this));
  this.webviewNode.addEventListener('blur', function(e) {
    // Blur the BrowserPlugin when the <webview> loses focus.
    this.browserPluginNode.blur();
  }.bind(this));
};

// Validation helper function for executeScript() and insertCSS().
WebViewInternal.prototype.validateExecuteCodeCall  = function() {
  if (!this.guestInstanceId) {
    throw new Error(ERROR_MSG_CANNOT_INJECT_SCRIPT);
  }
};

WebViewInternal.prototype.setupAutoSizeProperties = function() {
  $Array.forEach(AUTO_SIZE_ATTRIBUTES, function(attributeName) {
    this[attributeName] = this.webviewNode.getAttribute(attributeName);
    Object.defineProperty(this.webviewNode, attributeName, {
      get: function() {
        return this[attributeName];
      }.bind(this),
      set: function(value) {
        this.webviewNode.setAttribute(attributeName, value);
      }.bind(this),
      enumerable: true
    });
  }.bind(this), this);
};

WebViewInternal.prototype.setupWebviewNodeProperties = function() {
  this.setupAutoSizeProperties();

  Object.defineProperty(this.webviewNode,
                        WEB_VIEW_ATTRIBUTE_ALLOWTRANSPARENCY, {
    get: function() {
      return this.allowtransparency;
    }.bind(this),
    set: function(value) {
      this.webviewNode.setAttribute(WEB_VIEW_ATTRIBUTE_ALLOWTRANSPARENCY,
                                    value);
    }.bind(this),
    enumerable: true
  });

  // We cannot use {writable: true} property descriptor because we want a
  // dynamic getter value.
  Object.defineProperty(this.webviewNode, 'contentWindow', {
    get: function() {
      if (this.contentWindow) {
        return this.contentWindow;
      }
      window.console.error(ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE);
    }.bind(this),
    // No setter.
    enumerable: true
  });

  Object.defineProperty(this.webviewNode, 'name', {
    get: function() {
      return this.name;
    }.bind(this),
    set: function(value) {
      this.webviewNode.setAttribute('name', value);
    }.bind(this),
    enumerable: true
  });

  Object.defineProperty(this.webviewNode, 'partition', {
    get: function() {
      return this.partition.toAttribute();
    }.bind(this),
    set: function(value) {
      var result = this.partition.fromAttribute(value, this.hasNavigated());
      if (result.error) {
        throw result.error;
      }
      this.webviewNode.setAttribute('partition', value);
    }.bind(this),
    enumerable: true
  });

  this.src = this.webviewNode.getAttribute('src');
  Object.defineProperty(this.webviewNode, 'src', {
    get: function() {
      return this.src;
    }.bind(this),
    set: function(value) {
      this.webviewNode.setAttribute('src', value);
    }.bind(this),
    // No setter.
    enumerable: true
  });
};

// The purpose of this mutation observer is to catch assignment to the src
// attribute without any changes to its value. This is useful in the case
// where the webview guest has crashed and navigating to the same address
// spawns off a new process.
WebViewInternal.prototype.setupWebViewSrcAttributeMutationObserver =
    function() {
  this.srcAndPartitionObserver = new MutationObserver(function(mutations) {
    $Array.forEach(mutations, function(mutation) {
      var oldValue = mutation.oldValue;
      var newValue = this.webviewNode.getAttribute(mutation.attributeName);
      if (oldValue != newValue) {
        return;
      }
      this.handleWebviewAttributeMutation(
          mutation.attributeName, oldValue, newValue);
    }.bind(this));
  }.bind(this));
  var params = {
    attributes: true,
    attributeOldValue: true,
    attributeFilter: ['src', 'partition']
  };
  this.srcAndPartitionObserver.observe(this.webviewNode, params);
};

// This observer monitors mutations to attributes of the <webview> and
// updates the BrowserPlugin properties accordingly. In turn, updating
// a BrowserPlugin property will update the corresponding BrowserPlugin
// attribute, if necessary. See BrowserPlugin::UpdateDOMAttribute for more
// details.
WebViewInternal.prototype.handleWebviewAttributeMutation =
      function(name, oldValue, newValue) {
  if (AUTO_SIZE_ATTRIBUTES.indexOf(name) > -1) {
    this[name] = newValue;
    if (!this.guestInstanceId) {
      return;
    }
    // Convert autosize attribute to boolean.
    var autosize = this.webviewNode.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE);
    GuestViewInternal.setAutoSize(this.guestInstanceId, {
      'enableAutoSize': autosize,
      'min': {
        'width': parseInt(this.minwidth || 0),
        'height': parseInt(this.minheight || 0)
      },
      'max': {
        'width': parseInt(this.maxwidth || 0),
        'height': parseInt(this.maxheight || 0)
      }
    });
    return;
  } else if (name == WEB_VIEW_ATTRIBUTE_ALLOWTRANSPARENCY) {
    // We treat null attribute (attribute removed) and the empty string as
    // one case.
    oldValue = oldValue || '';
    newValue = newValue || '';

    if (oldValue === newValue) {
      return;
    }
    this.allowtransparency = newValue != '';

    if (!this.guestInstanceId) {
      return;
    }

    WebView.setAllowTransparency(this.guestInstanceId, this.allowtransparency);
    return;
  } else if (name == 'name') {
    // We treat null attribute (attribute removed) and the empty string as
    // one case.
    oldValue = oldValue || '';
    newValue = newValue || '';

    if (oldValue === newValue) {
      return;
    }
    this.name = newValue;
    if (!this.guestInstanceId) {
      return;
    }
    WebView.setName(this.guestInstanceId, newValue);
    return;
  } else if (name == 'src') {
    // We treat null attribute (attribute removed) and the empty string as
    // one case.
    oldValue = oldValue || '';
    newValue = newValue || '';
    // Once we have navigated, we don't allow clearing the src attribute.
    // Once <webview> enters a navigated state, it cannot be return back to a
    // placeholder state.
    if (newValue == '' && oldValue != '') {
      // src attribute changes normally initiate a navigation. We suppress
      // the next src attribute handler call to avoid reloading the page
      // on every guest-initiated navigation.
      this.ignoreNextSrcAttributeChange = true;
      this.webviewNode.setAttribute('src', oldValue);
      return;
    }
    this.src = newValue;
    if (this.ignoreNextSrcAttributeChange) {
      // Don't allow the src mutation observer to see this change.
      this.srcAndPartitionObserver.takeRecords();
      this.ignoreNextSrcAttributeChange = false;
      return;
    }
    var result = {};
    this.parseSrcAttribute(result);

    if (result.error) {
      throw result.error;
    }
  } else if (name == 'partition') {
    // Note that throwing error here won't synchronously propagate.
    this.partition.fromAttribute(newValue, this.hasNavigated());
  }
};

WebViewInternal.prototype.handleBrowserPluginAttributeMutation =
    function(name, oldValue, newValue) {
  if (name == 'internalinstanceid' && !oldValue && !!newValue) {
    this.browserPluginNode.removeAttribute('internalinstanceid');
    this.internalInstanceId = parseInt(newValue);

    if (!!this.guestInstanceId && this.guestInstanceId != 0) {
      var isNewWindow = this.deferredAttachState ?
          this.deferredAttachState.isNewWindow : false;
      var params = this.buildAttachParams(isNewWindow);
      guestViewInternalNatives.AttachGuest(
          this.internalInstanceId,
          this.guestInstanceId,
          params,
          function(w) {
            this.contentWindow = w;
          }.bind(this)
      );
    }

    return;
  }
};

WebViewInternal.prototype.onSizeChanged = function(webViewEvent) {
  var newWidth = webViewEvent.newWidth;
  var newHeight = webViewEvent.newHeight;

  var node = this.webviewNode;

  var width = node.offsetWidth;
  var height = node.offsetHeight;

  // Check the current bounds to make sure we do not resize <webview>
  // outside of current constraints.
  var maxWidth;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MAXWIDTH) &&
      node[WEB_VIEW_ATTRIBUTE_MAXWIDTH]) {
    maxWidth = node[WEB_VIEW_ATTRIBUTE_MAXWIDTH];
  } else {
    maxWidth = width;
  }

  var minWidth;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MINWIDTH) &&
      node[WEB_VIEW_ATTRIBUTE_MINWIDTH]) {
    minWidth = node[WEB_VIEW_ATTRIBUTE_MINWIDTH];
  } else {
    minWidth = width;
  }
  if (minWidth > maxWidth) {
    minWidth = maxWidth;
  }

  var maxHeight;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MAXHEIGHT) &&
      node[WEB_VIEW_ATTRIBUTE_MAXHEIGHT]) {
    maxHeight = node[WEB_VIEW_ATTRIBUTE_MAXHEIGHT];
  } else {
    maxHeight = height;
  }

  var minHeight;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MINHEIGHT) &&
      node[WEB_VIEW_ATTRIBUTE_MINHEIGHT]) {
    minHeight = node[WEB_VIEW_ATTRIBUTE_MINHEIGHT];
  } else {
    minHeight = height;
  }
  if (minHeight > maxHeight) {
    minHeight = maxHeight;
  }

  if (!this.webviewNode.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE) ||
      (newWidth >= minWidth &&
       newWidth <= maxWidth &&
       newHeight >= minHeight &&
       newHeight <= maxHeight)) {
    node.style.width = newWidth + 'px';
    node.style.height = newHeight + 'px';
    // Only fire the DOM event if the size of the <webview> has actually
    // changed.
    this.dispatchEvent(webViewEvent);
  }
};

// Returns if <object> is in the render tree.
WebViewInternal.prototype.isPluginInRenderTree = function() {
  return !!this.internalInstanceId && this.internalInstanceId != 0;
};

WebViewInternal.prototype.hasNavigated = function() {
  return !this.beforeFirstNavigation;
};

WebViewInternal.prototype.parseSrcAttribute = function(result) {
  if (!this.partition.validPartitionId) {
    result.error = ERROR_MSG_INVALID_PARTITION_ATTRIBUTE;
    return;
  }
  this.src = this.webviewNode.getAttribute('src');

  if (!this.src) {
    return;
  }

  if (this.guestInstanceId == undefined) {
    if (this.beforeFirstNavigation) {
      this.beforeFirstNavigation = false;
      this.createGuest();
    }
    return;
  }

  // Navigate to |this.src|.
  WebView.navigate(this.guestInstanceId, this.src);
};

WebViewInternal.prototype.parseAttributes = function() {
  if (!this.elementAttached) {
    return;
  }
  var hasNavigated = this.hasNavigated();
  var attributeValue = this.webviewNode.getAttribute('partition');
  var result = this.partition.fromAttribute(attributeValue, hasNavigated);
  this.parseSrcAttribute(result);
};

WebViewInternal.prototype.createGuest = function() {
  if (this.pendingGuestCreation) {
    return;
  }
  var storagePartitionId =
      this.webviewNode.getAttribute(WEB_VIEW_ATTRIBUTE_PARTITION) ||
      this.webviewNode[WEB_VIEW_ATTRIBUTE_PARTITION];
  var params = {
    'storagePartitionId': storagePartitionId
  };
  GuestViewInternal.createGuest(
      'webview',
      params,
      function(guestInstanceId) {
        this.pendingGuestCreation = false;
        if (!this.elementAttached) {
          GuestViewInternal.destroyGuest(guestInstanceId);
          return;
        }
        this.attachWindow(guestInstanceId, false);
      }.bind(this)
  );
  this.pendingGuestCreation = true;
};

WebViewInternal.prototype.onFrameNameChanged = function(name) {
  this.name = name || '';
  if (this.name === '') {
    this.webviewNode.removeAttribute('name');
  } else {
    this.webviewNode.setAttribute('name', this.name);
  }
};

WebViewInternal.prototype.dispatchEvent = function(webViewEvent) {
  return this.webviewNode.dispatchEvent(webViewEvent);
};

// Adds an 'on<event>' property on the webview, which can be used to set/unset
// an event handler.
WebViewInternal.prototype.setupEventProperty = function(eventName) {
  var propertyName = 'on' + eventName.toLowerCase();
  Object.defineProperty(this.webviewNode, propertyName, {
    get: function() {
      return this.on[propertyName];
    }.bind(this),
    set: function(value) {
      if (this.on[propertyName])
        this.webviewNode.removeEventListener(eventName, this.on[propertyName]);
      this.on[propertyName] = value;
      if (value)
        this.webviewNode.addEventListener(eventName, value);
    }.bind(this),
    enumerable: true
  });
};

// Updates state upon loadcommit.
WebViewInternal.prototype.onLoadCommit = function(
    baseUrlForDataUrl, currentEntryIndex, entryCount,
    processId, url, isTopLevel) {
  this.baseUrlForDataUrl = baseUrlForDataUrl;
  this.currentEntryIndex = currentEntryIndex;
  this.entryCount = entryCount;
  this.processId = processId;
  var oldValue = this.webviewNode.getAttribute('src');
  var newValue = url;
  if (isTopLevel && (oldValue != newValue)) {
    // Touching the src attribute triggers a navigation. To avoid
    // triggering a page reload on every guest-initiated navigation,
    // we use the flag ignoreNextSrcAttributeChange here.
    this.ignoreNextSrcAttributeChange = true;
    this.webviewNode.setAttribute('src', newValue);
  }
};

WebViewInternal.prototype.onAttach = function(storagePartitionId) {
  this.webviewNode.setAttribute('partition', storagePartitionId);
  this.partition.fromAttribute(storagePartitionId, this.hasNavigated());
};

WebViewInternal.prototype.buildAttachParams = function(isNewWindow) {
  var params = {
    'allowtransparency': this.allowtransparency || false,
    'autosize': this.webviewNode.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE),
    'instanceId': this.viewInstanceId,
    'maxheight': parseInt(this.maxheight || 0),
    'maxwidth': parseInt(this.maxwidth || 0),
    'minheight': parseInt(this.minheight || 0),
    'minwidth': parseInt(this.minwidth || 0),
    'name': this.name,
    // We don't need to navigate new window from here.
    'src': isNewWindow ? undefined : this.src,
    // If we have a partition from the opener, that will also be already
    // set via this.onAttach().
    'storagePartitionId': this.partition.toAttribute(),
    'userAgentOverride': this.userAgentOverride
  };
  return params;
};

WebViewInternal.prototype.attachWindow = function(guestInstanceId,
                                                  isNewWindow) {
  this.guestInstanceId = guestInstanceId;
  var params = this.buildAttachParams(isNewWindow);

  if (!this.isPluginInRenderTree()) {
    this.deferredAttachState = {isNewWindow: isNewWindow};
    return true;
  }

  this.deferredAttachState = null;
  return guestViewInternalNatives.AttachGuest(
      this.internalInstanceId,
      this.guestInstanceId,
      params, function(w) {
        this.contentWindow = w;
      }.bind(this)
  );
};

// -----------------------------------------------------------------------------
// Public-facing API methods.


// Navigates to the previous history entry.
WebViewInternal.prototype.back = function(callback) {
  return this.go(-1, callback);
};

// Returns whether there is a previous history entry to navigate to.
WebViewInternal.prototype.canGoBack = function() {
  return this.entryCount > 1 && this.currentEntryIndex > 0;
};

// Returns whether there is a subsequent history entry to navigate to.
WebViewInternal.prototype.canGoForward = function() {
  return this.currentEntryIndex >= 0 &&
      this.currentEntryIndex < (this.entryCount - 1);
};

// Clears browsing data for the WebView partition.
WebViewInternal.prototype.clearData = function() {
  if (!this.guestInstanceId) {
    return;
  }
  var args = $Array.concat([this.guestInstanceId], $Array.slice(arguments));
  $Function.apply(WebView.clearData, null, args);
};

// Injects JavaScript code into the guest page.
WebViewInternal.prototype.executeScript = function(var_args) {
  this.validateExecuteCodeCall();
  var webviewSrc = this.src;
  if (this.baseUrlForDataUrl != '') {
    webviewSrc = this.baseUrlForDataUrl;
  }
  var args = $Array.concat([this.guestInstanceId, webviewSrc],
                           $Array.slice(arguments));
  $Function.apply(WebView.executeScript, null, args);
};

// Initiates a find-in-page request.
WebViewInternal.prototype.find = function(search_text, options, callback) {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.find(this.guestInstanceId, search_text, options, callback);
};

// Navigates to the subsequent history entry.
WebViewInternal.prototype.forward = function(callback) {
  return this.go(1, callback);
};

// Returns Chrome's internal process ID for the guest web page's current
// process.
WebViewInternal.prototype.getProcessId = function() {
  return this.processId;
};

// Returns the user agent string used by the webview for guest page requests.
WebViewInternal.prototype.getUserAgent = function() {
  return this.userAgentOverride || navigator.userAgent;
};

// Gets the current zoom factor.
WebViewInternal.prototype.getZoom = function(callback) {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.getZoom(this.guestInstanceId, callback);
};

// Navigates to a history entry using a history index relative to the current
// navigation.
WebViewInternal.prototype.go = function(relativeIndex, callback) {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.go(this.guestInstanceId, relativeIndex, callback);
};

// Injects CSS into the guest page.
WebViewInternal.prototype.insertCSS = function(var_args) {
  this.validateExecuteCodeCall();
  var webviewSrc = this.src;
  if (this.baseUrlForDataUrl != '') {
    webviewSrc = this.baseUrlForDataUrl;
  }
  var args = $Array.concat([this.guestInstanceId, webviewSrc],
                           $Array.slice(arguments));
  $Function.apply(WebView.insertCSS, null, args);
};

// Indicates whether or not the webview's user agent string has been overridden.
WebViewInternal.prototype.isUserAgentOverridden = function() {
  return !!this.userAgentOverride &&
      this.userAgentOverride != navigator.userAgent;
};

// Prints the contents of the webview.
WebViewInternal.prototype.print = function() {
  this.executeScript({code: 'window.print();'});
};

// Reloads the current top-level page.
WebViewInternal.prototype.reload = function() {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.reload(this.guestInstanceId);
};

// Override the user agent string used by the webview for guest page requests.
WebViewInternal.prototype.setUserAgentOverride = function(userAgentOverride) {
  this.userAgentOverride = userAgentOverride;
  if (!this.guestInstanceId) {
    // If we are not attached yet, then we will pick up the user agent on
    // attachment.
    return;
  }
  WebView.overrideUserAgent(this.guestInstanceId, userAgentOverride);
};

// Changes the zoom factor of the page.
WebViewInternal.prototype.setZoom = function(zoomFactor, callback) {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.setZoom(this.guestInstanceId, zoomFactor, callback);
};

// Stops loading the current navigation if one is in progress.
WebViewInternal.prototype.stop = function() {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.stop(this.guestInstanceId);
};

// Ends the current find session.
WebViewInternal.prototype.stopFinding = function(action) {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.stopFinding(this.guestInstanceId, action);
};

// Forcibly kills the guest web page's renderer process.
WebViewInternal.prototype.terminate = function() {
  if (!this.guestInstanceId) {
    return;
  }
  WebView.terminate(this.guestInstanceId);
};

// -----------------------------------------------------------------------------

// Registers browser plugin <object> custom element.
function registerBrowserPluginElement() {
  var proto = Object.create(HTMLObjectElement.prototype);

  proto.createdCallback = function() {
    this.setAttribute('type', 'application/browser-plugin');
    this.setAttribute('id', 'browser-plugin-' + IdGenerator.GetNextId());
    // The <object> node fills in the <webview> container.
    this.style.width = '100%';
    this.style.height = '100%';
  };

  proto.attributeChangedCallback = function(name, oldValue, newValue) {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.handleBrowserPluginAttributeMutation(name, oldValue, newValue);
  };

  proto.attachedCallback = function() {
    // Load the plugin immediately.
    var unused = this.nonExistentAttribute;
  };

  WebViewInternal.BrowserPlugin =
      DocumentNatives.RegisterElement('browserplugin', {extends: 'object',
                                                        prototype: proto});

  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

// Registers <webview> custom element.
function registerWebViewElement() {
  var proto = Object.create(HTMLElement.prototype);

  proto.createdCallback = function() {
    new WebViewInternal(this);
  };

  proto.attributeChangedCallback = function(name, oldValue, newValue) {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.handleWebviewAttributeMutation(name, oldValue, newValue);
  };

  proto.detachedCallback = function() {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.elementAttached = false;
    internal.reset();
  };

  proto.attachedCallback = function() {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    if (!internal.elementAttached) {
      internal.elementAttached = true;
      internal.parseAttributes();
    }
  };

  // Public-facing API methods.
  var methods = [
    'back',
    'canGoBack',
    'canGoForward',
    'clearData',
    'executeScript',
    'find',
    'forward',
    'getProcessId',
    'getUserAgent',
    'getZoom',
    'go',
    'insertCSS',
    'isUserAgentOverridden',
    'print',
    'reload',
    'setUserAgentOverride',
    'setZoom',
    'stop',
    'stopFinding',
    'terminate'
  ];

  // Add the experimental API methods, if available.
  var experimentalMethods =
      WebViewInternal.maybeGetExperimentalAPIs();
  methods = $Array.concat(methods, experimentalMethods);

  // Forward proto.foo* method calls to WebViewInternal.foo*.
  var createHandler = function(m) {
    return function(var_args) {
      var internal = privates(this).internal;
      return $Function.apply(internal[m], internal, arguments);
    };
  };
  for (var i = 0; methods[i]; ++i) {
    proto[methods[i]] = createHandler(methods[i]);
  }

  window.WebView =
      DocumentNatives.RegisterElement('webview', {prototype: proto});

  // Delete the callbacks so developers cannot call them and produce unexpected
  // behavior.
  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

var useCapture = true;
window.addEventListener('readystatechange', function listener(event) {
  if (document.readyState == 'loading')
    return;

  registerBrowserPluginElement();
  registerWebViewElement();
  window.removeEventListener(event.type, listener, useCapture);
}, useCapture);

// Implemented when the ChromeWebView API is available.
WebViewInternal.prototype.maybeGetChromeWebViewEvents = function() {};

// Implemented when the experimental WebView API is available.
WebViewInternal.maybeGetExperimentalAPIs = function() {};
WebViewInternal.prototype.maybeGetExperimentalEvents = function() {};
WebViewInternal.prototype.setupExperimentalContextMenus = function() {};

// Exports.
exports.WebView = WebView;
exports.WebViewInternal = WebViewInternal;
