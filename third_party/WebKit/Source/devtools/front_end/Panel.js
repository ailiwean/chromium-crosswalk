/*
 * Copyright (C) 2007, 2008 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @extends {WebInspector.View}
 * @implements {WebInspector.Searchable}
 * @constructor
 */
WebInspector.Panel = function(name)
{
    WebInspector.View.call(this);
    WebInspector.panels[name] = this;

    this.element.addStyleClass("panel");
    this.element.addStyleClass(name);
    this._panelName = name;

    this._shortcuts = /** !Object.<number, function(Event=):boolean> */ ({});

    WebInspector.settings[this._sidebarWidthSettingName()] = WebInspector.settings.createSetting(this._sidebarWidthSettingName(), undefined);
}

// Should by in sync with style declarations.
WebInspector.Panel.counterRightMargin = 25;

WebInspector.Panel._minimalSearchQuerySize = 3;

WebInspector.Panel.prototype = {
    get name()
    {
        return this._panelName;
    },

    reset: function()
    {
        this.searchCanceled();
    },

    defaultFocusedElement: function()
    {
        return this.sidebarTreeElement || this.element;
    },

    searchCanceled: function()
    {
        WebInspector.searchController.updateSearchMatchesCount(0, this);
    },

    /**
     * @param {string} query
     * @param {boolean} shouldJump
     */
    performSearch: function(query, shouldJump)
    {
        // Call searchCanceled since it will reset everything we need before doing a new search.
        this.searchCanceled();
    },

    /**
     * @return {number}
     */
    minimalSearchQuerySize: function()
    {
        return WebInspector.Panel._minimalSearchQuerySize;
    },

    jumpToNextSearchResult: function()
    {
    },

    jumpToPreviousSearchResult: function()
    {
    },

    /**
     * @override
     * @param {HTMLInputElement} input
     * @return {?Array.<string>}
     */
    buildSuggestions: function(input)
    {
        return null;
    },

    /**
     * @return {boolean}
     */
    canSearchAndReplace: function()
    {
        return false;
    },

    /**
     * @param {string} text
     */
    replaceSelectionWith: function(text)
    {
    },

    /**
     * @param {string} query
     * @param {string} text
     */
    replaceAllWith: function(query, text)
    {
    },

    /**
     * @return {boolean}
     */
    canFilter: function()
    {
        return false;
    },

    /**
     * @param {string} query
     */
    performFilter: function(query)
    {
    },

    /**
     * @return {boolean}
     */
    canSetFooterElement: function()
    {
        return false;
    },

    /**
     * @param {?Element} element
     */
    setFooterElement: function(element)
    {
    },

    /**
     * @param {Element=} parentElement
     * @param {string=} position
     * @param {number=} defaultWidth
     * @param {number=} defaultHeight
     */
    createSidebarView: function(parentElement, position, defaultWidth, defaultHeight)
    {
        if (this.splitView)
            return;

        if (!parentElement)
            parentElement = this.element;

        this.splitView = new WebInspector.SidebarView(position, this._sidebarWidthSettingName(), defaultWidth, defaultHeight);
        this.splitView.show(parentElement);
        this.splitView.addEventListener(WebInspector.SidebarView.EventTypes.Resized, this.sidebarResized.bind(this));

        this.sidebarElement = this.splitView.sidebarElement;
    },

    /**
     * @param {Element=} parentElement
     * @param {string=} position
     * @param {number=} defaultWidth
     */
    createSidebarViewWithTree: function(parentElement, position, defaultWidth)
    {
        if (this.splitView)
            return;

        this.createSidebarView(parentElement, position);

        this.sidebarTreeElement = document.createElement("ol");
        this.sidebarTreeElement.className = "sidebar-tree";
        this.splitView.sidebarElement.appendChild(this.sidebarTreeElement);
        this.splitView.sidebarElement.addStyleClass("sidebar");

        this.sidebarTree = new TreeOutline(this.sidebarTreeElement);
        this.sidebarTree.panel = this;
    },

    _sidebarWidthSettingName: function()
    {
        return this._panelName + "SidebarWidth";
    },

    // Should be implemented by ancestors.

    get statusBarItems()
    {
    },

    /**
     * @param {WebInspector.Event} event
     */
    sidebarResized: function(event)
    {
    },

    statusBarResized: function()
    {
    },

    /**
     * @param {Element} anchor
     * @return {boolean}
     */
    canShowAnchorLocation: function(anchor)
    {
        return false;
    },

    /**
     * @param {Element} anchor
     */
    showAnchorLocation: function(anchor)
    {
    },

    elementsToRestoreScrollPositionsFor: function()
    {
        return [];
    },

    /**
     * @param {KeyboardEvent} event
     */
    handleShortcut: function(event)
    {
        var shortcutKey = WebInspector.KeyboardShortcut.makeKeyFromEvent(event);
        var handler = this._shortcuts[shortcutKey];
        if (handler && handler(event))
            event.handled = true;
    },

    /**
     * @param {!Array.<!WebInspector.KeyboardShortcut.Descriptor>} keys
     * @param {function(Event=):boolean} handler
     */
    registerShortcuts: function(keys, handler)
    {
        for (var i = 0; i < keys.length; ++i)
            this._shortcuts[keys[i].key] = handler;
    },

    __proto__: WebInspector.View.prototype
}

/**
 * @constructor
 * @param {string} name
 * @param {string} title
 * @param {string=} className
 * @param {string=} scriptName
 * @param {WebInspector.Panel=} panel
 */
WebInspector.PanelDescriptor = function(name, title, className, scriptName, panel)
{
    this._name = name;
    this._title = title;
    this._className = className;
    this._scriptName = scriptName;
    this._panel = panel;
}

WebInspector.PanelDescriptor.prototype = {
    /**
     * @return {string}
     */
    name: function()
    {
        return this._name;
    },

    /**
     * @return {string}
     */
    title: function()
    {
        return this._title;
    },

    /**
     * @return {WebInspector.Panel}
     */
    panel: function()
    {
        if (this._panel)
            return this._panel;
        if (this._scriptName)
            loadScript(this._scriptName);
        this._panel = new WebInspector[this._className];
        return this._panel;
    },

    registerShortcuts: function() {}
}
