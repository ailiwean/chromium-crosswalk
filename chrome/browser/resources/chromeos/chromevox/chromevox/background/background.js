// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Script that runs on the background page.
 */

goog.provide('cvox.ChromeVoxBackground');

goog.require('cvox.AbstractEarcons');
goog.require('cvox.BrailleBackground');
goog.require('cvox.BrailleCaptionsBackground');
goog.require('cvox.ChromeVox');
goog.require('cvox.ChromeVoxEditableTextBase');
goog.require('cvox.ChromeVoxPrefs');
goog.require('cvox.CompositeTts');
goog.require('cvox.ConsoleTts');
goog.require('cvox.EarconsBackground');
goog.require('cvox.ExtensionBridge');
goog.require('cvox.HostFactory');
goog.require('cvox.InjectedScriptLoader');
goog.require('cvox.Msgs');
goog.require('cvox.NavBraille');
// TODO(dtseng): This is required to prevent Closure from stripping our export
// prefs on window.
goog.require('cvox.OptionsPage');
goog.require('cvox.PlatformFilter');
goog.require('cvox.PlatformUtil');
goog.require('cvox.QueueMode');
goog.require('cvox.TabsApiHandler');
goog.require('cvox.TtsBackground');


/**
 * This object manages the global and persistent state for ChromeVox.
 * It listens for messages from the content scripts on pages and
 * interprets them.
 * @constructor
 */
cvox.ChromeVoxBackground = function() {
};


/**
 * Initialize the background page: set up TTS and bridge listeners.
 */
cvox.ChromeVoxBackground.prototype.init = function() {
  // In the case of ChromeOS, only continue initialization if this instance of
  // ChromeVox is as we expect. This prevents ChromeVox from the webstore from
  // running.
  if (cvox.ChromeVox.isChromeOS &&
      chrome.i18n.getMessage('@@extension_id') !=
          'mndnfokpggljbaajbnioimlmbfngpief') {
    return;
  }

  cvox.ChromeVox.msgs = new cvox.Msgs();
  this.prefs = new cvox.ChromeVoxPrefs();
  this.readPrefs();

  var consoleTts = cvox.ConsoleTts.getInstance();
  consoleTts.setEnabled(true);

  /**
   * Chrome's actual TTS which knows and cares about pitch, volume, etc.
   * @type {cvox.TtsBackground}
   * @private
   */
  this.backgroundTts_ = new cvox.TtsBackground();

  /**
   * @type {cvox.TtsInterface}
   */
  this.tts = new cvox.CompositeTts()
      .add(this.backgroundTts_)
      .add(consoleTts);

  this.earcons = new cvox.EarconsBackground();
  this.addBridgeListener();

  /**
   * The actual Braille service.
   * @type {cvox.BrailleBackground}
   * @private
   */
  this.backgroundBraille_ = new cvox.BrailleBackground();

    this.tabsApiHandler_ = new cvox.TabsApiHandler(
      this.tts, this.backgroundBraille_, this.earcons);

  // Export globals on cvox.ChromeVox.
  cvox.ChromeVox.tts = this.tts;
  cvox.ChromeVox.braille = this.backgroundBraille_;
  cvox.ChromeVox.earcons = this.earcons;

  if (cvox.ChromeVox.isChromeOS &&
      chrome.accessibilityPrivate.onIntroduceChromeVox) {
    chrome.accessibilityPrivate.onIntroduceChromeVox.addListener(
        this.onIntroduceChromeVox);
  }

  // Set up a message passing system for goog.provide() calls from
  // within the content scripts.
  chrome.extension.onMessage.addListener(
      function(request, sender, callback) {
        if (request['srcFile']) {
          var srcFile = request['srcFile'];
          cvox.InjectedScriptLoader.fetchCode(
              [srcFile],
              function(code) {
                callback({'code': code[srcFile]});
              });
        }
        return true;
      });

  var self = this;

  // Inject the content script into all running tabs.
  chrome.windows.getAll({'populate': true}, function(windows) {
    for (var i = 0; i < windows.length; i++) {
      var tabs = windows[i].tabs;
      self.injectChromeVoxIntoTabs(tabs);
    }
  });

  if (localStorage['active'] == 'false') {
    // Warn the user when the browser first starts if ChromeVox is inactive.
    this.tts.speak(cvox.ChromeVox.msgs.getMsg('chromevox_inactive'),
                   cvox.QueueMode.QUEUE);
  } else if (cvox.PlatformUtil.matchesPlatform(cvox.PlatformFilter.WML)) {
    // Introductory message.
    this.tts.speak(cvox.ChromeVox.msgs.getMsg('chromevox_intro'),
                   cvox.QueueMode.QUEUE);
    cvox.ChromeVox.braille.write(cvox.NavBraille.fromText(
        cvox.ChromeVox.msgs.getMsg('intro_brl')));
  }
};


/**
 * Inject ChromeVox into a tab.
 * @param {Array<Tab>} tabs The tab where ChromeVox scripts should be injected.
 */
cvox.ChromeVoxBackground.prototype.injectChromeVoxIntoTabs = function(tabs) {
  var listOfFiles;

  // These lists of files must match the content_scripts section in
  // the manifest files.
  if (COMPILED) {
    listOfFiles = ['chromeVoxChromePageScript.js'];
  } else {
    listOfFiles = [
        'closure/closure_preinit.js',
        'closure/base.js',
        'deps.js',
        'chromevox/injected/loader.js'];
  }

  var stageTwo = function(code) {
    for (var i = 0, tab; tab = tabs[i]; i++) {
      window.console.log('Injecting into ' + tab.id, tab);
      var sawError = false;

      /**
       * A helper function which executes code.
       * @param {string} code The code to execute.
       */
      var executeScript = goog.bind(function(code) {
        chrome.tabs.executeScript(
            tab.id,
            {'code': code,
             'allFrames': true},
            goog.bind(function() {
              if (!chrome.extension.lastError) {
                return;
              }
              if (sawError) {
                return;
              }
              sawError = true;
              console.error('Could not inject into tab', tab);
              this.tts.speak('Error starting ChromeVox for ' +
                  tab.title + ', ' + tab.url, cvox.QueueMode.QUEUE);
            }, this));
      }, this);

      // There is a scenario where two copies of the content script can get
      // loaded into the same tab on browser startup - one automatically and one
      // because the background page injects the content script into every tab
      // on startup. To work around potential bugs resulting from this,
      // ChromeVox exports a global function called disableChromeVox() that can
      // be used here to disable any existing running instance before we inject
      // a new instance of the content script into this tab.
      //
      // It's harmless if there wasn't a copy of ChromeVox already running.
      //
      // Also, set some variables so that Closure deps work correctly and so
      // that ChromeVox knows not to announce feedback as if a page just loaded.
      executeScript('try { window.disableChromeVox(); } catch(e) { }\n' +
          'window.INJECTED_AFTER_LOAD = true;\n' +
          'window.CLOSURE_NO_DEPS = true\n');

      // Now inject the ChromeVox content script code into the tab.
      listOfFiles.forEach(function(file) { executeScript(code[file]); });
    }
  };

  // We use fetchCode instead of chrome.extensions.executeFile because
  // executeFile doesn't propagate the file name to the content script
  // which means that script is not visible in Dev Tools.
  cvox.InjectedScriptLoader.fetchCode(listOfFiles, stageTwo);
};


/**
 * Called when a TTS message is received from a page content script.
 * @param {Object} msg The TTS message.
 */
cvox.ChromeVoxBackground.prototype.onTtsMessage = function(msg) {
  if (msg['action'] == 'speak') {
    this.tts.speak(msg['text'],
                   /** cvox.QueueMode */msg['queueMode'],
                   msg['properties']);
  } else if (msg['action'] == 'stop') {
    this.tts.stop();
  } else if (msg['action'] == 'increaseOrDecrease') {
    this.tts.increaseOrDecreaseProperty(msg['property'], msg['increase']);
    var property = msg['property'];
    var engine = this.backgroundTts_;
    var valueAsPercent = Math.round(
        this.backgroundTts_.propertyToPercentage(property) * 100);
    var announcement;
    switch (msg['property']) {
    case cvox.AbstractTts.RATE:
      announcement = cvox.ChromeVox.msgs.getMsg('announce_rate',
                                                [valueAsPercent]);
      break;
    case cvox.AbstractTts.PITCH:
      announcement = cvox.ChromeVox.msgs.getMsg('announce_pitch',
                                                [valueAsPercent]);
      break;
    case cvox.AbstractTts.VOLUME:
      announcement = cvox.ChromeVox.msgs.getMsg('announce_volume',
                                                [valueAsPercent]);
      break;
    }
    if (announcement) {
      this.tts.speak(announcement,
                     cvox.QueueMode.FLUSH,
                     cvox.AbstractTts.PERSONALITY_ANNOTATION);
    }
  } else if (msg['action'] == 'cyclePunctuationEcho') {
    this.tts.speak(cvox.ChromeVox.msgs.getMsg(
            this.backgroundTts_.cyclePunctuationEcho()),
                   cvox.QueueMode.FLUSH);
  }
};


/**
 * Called when an earcon message is received from a page content script.
 * @param {Object} msg The earcon message.
 */
cvox.ChromeVoxBackground.prototype.onEarconMessage = function(msg) {
  if (msg.action == 'play') {
    this.earcons.playEarcon(msg.earcon);
  }
};


/**
 * Listen for connections from our content script bridges, and dispatch the
 * messages to the proper destination.
 */
cvox.ChromeVoxBackground.prototype.addBridgeListener = function() {
  cvox.ExtensionBridge.addMessageListener(goog.bind(function(msg, port) {
    var target = msg['target'];
    var action = msg['action'];

    switch (target) {
    case 'OpenTab':
      var destination = {url: msg['url']};
      chrome.tabs.create(destination);
      break;
    case 'KbExplorer':
      var explorerPage = {url: 'chromevox/background/kbexplorer.html'};
      chrome.tabs.create(explorerPage);
      break;
    case 'HelpDocs':
      var helpPage = {url: 'http://chromevox.com/tutorial/index.html'};
      chrome.tabs.create(helpPage);
      break;
    case 'Options':
      if (action == 'open') {
        var optionsPage = {url: 'chromevox/background/options.html'};
        chrome.tabs.create(optionsPage);
      }
      break;
    case 'Data':
      if (action == 'getHistory') {
        var results = {};
        chrome.history.search({text: '', maxResults: 25}, function(items) {
          items.forEach(function(item) {
            if (item.url) {
              results[item.url] = true;
            }
          });
          port.postMessage({
            'history': results
          });
        });
      }
      break;
    case 'Prefs':
      if (action == 'getPrefs') {
        this.prefs.sendPrefsToPort(port);
      } else if (action == 'setPref') {
        if (msg['pref'] == 'active' &&
            msg['value'] != cvox.ChromeVox.isActive) {
          if (cvox.ChromeVox.isActive) {
            this.tts.speak(cvox.ChromeVox.msgs.getMsg('chromevox_inactive'),
                           cvox.QueueMode.FLUSH);
            chrome.accessibilityPrivate.setNativeAccessibilityEnabled(
                true);
          } else {
            chrome.accessibilityPrivate.setNativeAccessibilityEnabled(
                false);
          }
        } else if (msg['pref'] == 'earcons') {
          this.earcons.enabled = msg['value'];
        } else if (msg['pref'] == 'sticky' && msg['announce']) {
          if (msg['value']) {
            this.tts.speak(cvox.ChromeVox.msgs.getMsg('sticky_mode_enabled'),
                           cvox.QueueMode.QUEUE);
          } else {
            this.tts.speak(
                cvox.ChromeVox.msgs.getMsg('sticky_mode_disabled'),
                cvox.QueueMode.QUEUE);
          }
        } else if (msg['pref'] == 'typingEcho' && msg['announce']) {
          var announce = '';
          switch (msg['value']) {
            case cvox.TypingEcho.CHARACTER:
              announce = cvox.ChromeVox.msgs.getMsg('character_echo');
              break;
            case cvox.TypingEcho.WORD:
              announce = cvox.ChromeVox.msgs.getMsg('word_echo');
              break;
            case cvox.TypingEcho.CHARACTER_AND_WORD:
              announce = cvox.ChromeVox.msgs.getMsg('character_and_word_echo');
              break;
            case cvox.TypingEcho.NONE:
              announce = cvox.ChromeVox.msgs.getMsg('none_echo');
              break;
            default:
              break;
          }
          if (announce) {
            this.tts.speak(announce, cvox.QueueMode.QUEUE);
          }
        } else if (msg['pref'] == 'brailleCaptions') {
          cvox.BrailleCaptionsBackground.setActive(msg['value']);
        }
        this.prefs.setPref(msg['pref'], msg['value']);
        this.readPrefs();
      }
      break;
    case 'Math':
      // TODO (sorge): Put the change of styles etc. here!
      if (msg['action'] == 'getDomains') {
        port.postMessage({'message': 'DOMAINS_STYLES',
                          'domains': this.backgroundTts_.mathmap.allDomains,
                          'styles': this.backgroundTts_.mathmap.allStyles});
      }
      break;
    case 'TTS':
      if (msg['startCallbackId'] != undefined) {
        msg['properties']['startCallback'] = function(opt_cleanupOnly) {
          port.postMessage({'message': 'TTS_CALLBACK',
                            'cleanupOnly': opt_cleanupOnly,
                            'id': msg['startCallbackId']});
        };
      }
      if (msg['endCallbackId'] != undefined) {
        msg['properties']['endCallback'] = function(opt_cleanupOnly) {
          port.postMessage({'message': 'TTS_CALLBACK',
                            'cleanupOnly': opt_cleanupOnly,
                            'id': msg['endCallbackId']});
        };
      }
      try {
        this.onTtsMessage(msg);
      } catch (err) {
        console.log(err);
      }
      break;
    case 'EARCON':
      this.onEarconMessage(msg);
      break;
    case 'BRAILLE':
      try {
        this.backgroundBraille_.onBrailleMessage(msg);
      } catch (err) {
        console.log(err);
      }
      break;
    }
  }, this));
};


/**
 * Read and apply preferences that affect the background context.
 */
cvox.ChromeVoxBackground.prototype.readPrefs = function() {
  var prefs = this.prefs.getPrefs();
  cvox.ChromeVoxEditableTextBase.useIBeamCursor =
      (prefs['useIBeamCursor'] == 'true');
  cvox.ChromeVox.isActive =
      (prefs['active'] == 'true' || cvox.ChromeVox.isChromeOS);
  cvox.ChromeVox.isStickyPrefOn = (prefs['sticky'] == 'true');
};

/**
 * Checks if we are currently in an incognito window.
 * @return {boolean} True if incognito or not within a tab context, false
 * otherwise.
 * @private
 */
cvox.ChromeVoxBackground.prototype.isIncognito_ = function() {
  var incognito = false;
  chrome.tabs.getCurrent(function(tab) {
    // Tab is null if not called from a tab context. In that case, also consider
    // it incognito.
    incognito = tab ? tab.incognito : true;
  });
  return incognito;
};


/**
 * Handles the onIntroduceChromeVox event.
 */
cvox.ChromeVoxBackground.prototype.onIntroduceChromeVox = function() {
  cvox.ChromeVox.tts.speak(cvox.ChromeVox.msgs.getMsg('chromevox_intro'),
                           cvox.QueueMode.QUEUE,
                           {doNotInterrupt: true});
  cvox.ChromeVox.braille.write(cvox.NavBraille.fromText(
      cvox.ChromeVox.msgs.getMsg('intro_brl')));
};


// Create the background page object and export a function window['speak']
// so that other background pages can access it. Also export the prefs object
// for access by the options page.
(function() {
  var background = new cvox.ChromeVoxBackground();
  background.init();
  window['speak'] = goog.bind(background.tts.speak, background.tts);

  // Export the prefs object for access by the options page.
  window['prefs'] = background.prefs;

  // Export the braille translator manager for access by the options page.
  window['braille_translator_manager'] =
      background.backgroundBraille_.getTranslatorManager();

  // Export injection for ChromeVox Next.
  cvox.ChromeVox.injectChromeVoxIntoTabs =
      background.injectChromeVoxIntoTabs.bind(background);
})();
