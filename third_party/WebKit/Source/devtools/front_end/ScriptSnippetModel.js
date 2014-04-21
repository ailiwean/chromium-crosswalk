/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @constructor
 * @extends {WebInspector.Object}
 * @param {!WebInspector.Workspace} workspace
 */
WebInspector.ScriptSnippetModel = function(workspace)
{
    this._workspace = workspace;
    /** @type {!Object.<string, !WebInspector.UISourceCode>} */
    this._uiSourceCodeForScriptId = {};
    /** @type {!Map.<!WebInspector.UISourceCode, !WebInspector.Script>} */
    this._scriptForUISourceCode = new Map();
    /** @type {!Object.<string, !WebInspector.UISourceCode>} */
    this._uiSourceCodeForSnippetId = {};
    /** @type {!Map.<!WebInspector.UISourceCode, string>} */
    this._snippetIdForUISourceCode = new Map();
    
    this._snippetStorage = new WebInspector.SnippetStorage("script", "Script snippet #");
    this._lastSnippetEvaluationIndexSetting = WebInspector.settings.createSetting("lastSnippetEvaluationIndex", 0);
    this._snippetScriptMapping = new WebInspector.SnippetScriptMapping(this);
    this._projectId = WebInspector.projectTypes.Snippets + ":";
    this._projectDelegate = new WebInspector.SnippetsProjectDelegate(workspace, this, this._projectId);
    this._project = this._workspace.project(this._projectId);
    this.reset();
    WebInspector.debuggerModel.addEventListener(WebInspector.DebuggerModel.Events.GlobalObjectCleared, this._debuggerReset, this);
}

WebInspector.ScriptSnippetModel.prototype = {
    /**
     * @return {!WebInspector.SnippetScriptMapping}
     */
    get scriptMapping()
    {
        return this._snippetScriptMapping;
    },

    /**
     * @return {!WebInspector.Project}
     */
    project: function()
    {
        return this._project;
    },

    _loadSnippets: function()
    {
        var snippets = this._snippetStorage.snippets();
        for (var i = 0; i < snippets.length; ++i)
            this._addScriptSnippet(snippets[i]);
    },

    /**
     * @param {string} content
     * @return {string}
     */
    createScriptSnippet: function(content)
    {
        var snippet = this._snippetStorage.createSnippet();
        snippet.content = content;
        return this._addScriptSnippet(snippet);
    },

    /**
     * @param {!WebInspector.Snippet} snippet
     * @return {string}
     */
    _addScriptSnippet: function(snippet)
    {
        var path = this._projectDelegate.addSnippet(snippet.name, new WebInspector.SnippetContentProvider(snippet));
        var uiSourceCode = this._workspace.uiSourceCode(this._projectId, path);
        if (!uiSourceCode) {
            console.assert(uiSourceCode);
            return "";
        }
        uiSourceCode.addEventListener(WebInspector.UISourceCode.Events.WorkingCopyChanged, this._workingCopyChanged, this);
        this._snippetIdForUISourceCode.put(uiSourceCode, snippet.id);
        var breakpointLocations = this._removeBreakpoints(uiSourceCode);
        uiSourceCode.setSourceMapping(this._snippetScriptMapping);
        this._restoreBreakpoints(uiSourceCode, breakpointLocations);
        this._uiSourceCodeForSnippetId[snippet.id] = uiSourceCode;
        return path;
    },

    /**
     * @param {!WebInspector.Event} event
     */
    _workingCopyChanged: function(event)
    {
        var uiSourceCode = /** @type {!WebInspector.UISourceCode} */ (event.target);
        this._scriptSnippetEdited(uiSourceCode);
    },

    /**
     * @param {string} path
     */
    deleteScriptSnippet: function(path)
    {
        var uiSourceCode = this._workspace.uiSourceCode(this._projectId, path);
        if (!uiSourceCode)
            return;
        var snippetId = this._snippetIdForUISourceCode.get(uiSourceCode) || "";
        var snippet = this._snippetStorage.snippetForId(snippetId);
        this._snippetStorage.deleteSnippet(snippet);
        this._removeBreakpoints(uiSourceCode);
        this._releaseSnippetScript(uiSourceCode);
        delete this._uiSourceCodeForSnippetId[snippet.id];
        this._snippetIdForUISourceCode.remove(uiSourceCode);
        this._projectDelegate.removeFile(snippet.name);
    },

    /**
     * @param {string} name
     * @param {string} newName
     * @param {function(boolean, string=)} callback
     */
    renameScriptSnippet: function(name, newName, callback)
    {
        newName = newName.trim();
        if (!newName || newName.indexOf("/") !== -1 || name === newName || this._snippetStorage.snippetForName(newName)) {
            callback(false);
            return;
        }
        var snippet = this._snippetStorage.snippetForName(name);
        console.assert(snippet, "Snippet '" + name + "' was not found.");
        var uiSourceCode = this._uiSourceCodeForSnippetId[snippet.id];
        console.assert(uiSourceCode, "No uiSourceCode was found for snippet '" + name + "'.");

        var breakpointLocations = this._removeBreakpoints(uiSourceCode);
        snippet.name = newName;
        this._restoreBreakpoints(uiSourceCode, breakpointLocations);
        callback(true, newName);
    },

    /**
     * @param {string} name
     * @param {string} newContent
     */
    _setScriptSnippetContent: function(name, newContent)
    {
        var snippet = this._snippetStorage.snippetForName(name);
        snippet.content = newContent;
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     */
    _scriptSnippetEdited: function(uiSourceCode)
    {
        var script = this._scriptForUISourceCode.get(uiSourceCode);
        if (!script)
            return;

        var breakpointLocations = this._removeBreakpoints(uiSourceCode);
        this._releaseSnippetScript(uiSourceCode);
        this._restoreBreakpoints(uiSourceCode, breakpointLocations);
        var scriptUISourceCode = script.rawLocationToUILocation(0, 0).uiSourceCode;
        if (scriptUISourceCode)
            this._restoreBreakpoints(scriptUISourceCode, breakpointLocations);
    },

    /**
     * @param {string} snippetId
     * @return {number}
     */
    _nextEvaluationIndex: function(snippetId)
    {
        var evaluationIndex = this._lastSnippetEvaluationIndexSetting.get() + 1;
        this._lastSnippetEvaluationIndexSetting.set(evaluationIndex);
        return evaluationIndex;
    },

    /**
     * @param {!WebInspector.ExecutionContext} executionContext
     * @param {!WebInspector.UISourceCode} uiSourceCode
     */
    evaluateScriptSnippet: function(executionContext, uiSourceCode)
    {
        var breakpointLocations = this._removeBreakpoints(uiSourceCode);
        this._releaseSnippetScript(uiSourceCode);
        this._restoreBreakpoints(uiSourceCode, breakpointLocations);
        var snippetId = this._snippetIdForUISourceCode.get(uiSourceCode) || "";
        var evaluationIndex = this._nextEvaluationIndex(snippetId);
        uiSourceCode._evaluationIndex = evaluationIndex;
        var evaluationUrl = this._evaluationSourceURL(uiSourceCode);
        var expression = uiSourceCode.workingCopy();
        
        WebInspector.console.show();
        var target = executionContext.target();
        target.debuggerAgent().compileScript(expression, evaluationUrl, executionContext.id, compileCallback.bind(this, target));

        /**
         * @param {!WebInspector.Target} target
         * @param {?string} error
         * @param {string=} scriptId
         * @param {string=} syntaxErrorMessage
         * @this {WebInspector.ScriptSnippetModel}
         */
        function compileCallback(target, error, scriptId, syntaxErrorMessage)
        {
            if (!uiSourceCode || uiSourceCode._evaluationIndex !== evaluationIndex)
                return;

            if (error) {
                console.error(error);
                return;
            }

            if (!scriptId) {
                var consoleMessage = new WebInspector.ConsoleMessage(
                        target,
                        WebInspector.ConsoleMessage.MessageSource.JS,
                        WebInspector.ConsoleMessage.MessageLevel.Error,
                        syntaxErrorMessage || "");
                target.consoleModel.addMessage(consoleMessage);
                return;
            }

            var breakpointLocations = this._removeBreakpoints(uiSourceCode);
            this._restoreBreakpoints(uiSourceCode, breakpointLocations);

            this._runScript(scriptId, executionContext);
        }
    },

    /**
     * @param {!DebuggerAgent.ScriptId} scriptId
     * @param {!WebInspector.ExecutionContext} executionContext
     */
    _runScript: function(scriptId, executionContext)
    {
        var target = executionContext.target();
        target.debuggerAgent().runScript(scriptId, executionContext.id, "console", false, runCallback.bind(this, target));

        /**
         * @param {!WebInspector.Target} target
         * @param {?string} error
         * @param {?RuntimeAgent.RemoteObject} result
         * @param {boolean=} wasThrown
         * @this {WebInspector.ScriptSnippetModel}
         */
        function runCallback(target, error, result, wasThrown)
        {
            if (error) {
                console.error(error);
                return;
            }

            this._printRunScriptResult(target, result, wasThrown);
        }
    },

    /**
     * @param {!WebInspector.Target} target
     * @param {?RuntimeAgent.RemoteObject} result
     * @param {boolean=} wasThrown
     */
    _printRunScriptResult: function(target, result, wasThrown)
    {
        var level = (wasThrown ? WebInspector.ConsoleMessage.MessageLevel.Error : WebInspector.ConsoleMessage.MessageLevel.Log);
        var message = new WebInspector.ConsoleMessage(target,
            WebInspector.ConsoleMessage.MessageSource.JS,
            level,
            "",
            undefined,
            undefined,
            undefined,
            undefined,
            undefined,
            [result]);
        target.consoleModel.addMessage(message);
    },

    /**
     * @param {!WebInspector.DebuggerModel.Location} rawLocation
     * @return {?WebInspector.UILocation}
     */
    _rawLocationToUILocation: function(rawLocation)
    {
        var uiSourceCode = this._uiSourceCodeForScriptId[rawLocation.scriptId];
        if (!uiSourceCode)
            return null;
        return uiSourceCode.uiLocation(rawLocation.lineNumber, rawLocation.columnNumber || 0);
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     * @param {number} lineNumber
     * @param {number} columnNumber
     * @return {?WebInspector.DebuggerModel.Location}
     */
    _uiLocationToRawLocation: function(uiSourceCode, lineNumber, columnNumber)
    {
        var script = this._scriptForUISourceCode.get(uiSourceCode);
        if (!script)
            return null;

        return WebInspector.debuggerModel.createRawLocation(script, lineNumber, columnNumber);
    },

    /**
     * @param {!WebInspector.Script} script
     */
    _addScript: function(script)
    {
        var snippetId = this._snippetIdForSourceURL(script.sourceURL);
        if (!snippetId)
            return;
        var uiSourceCode = this._uiSourceCodeForSnippetId[snippetId];

        if (!uiSourceCode || this._evaluationSourceURL(uiSourceCode) !== script.sourceURL)
            return;

        console.assert(!this._scriptForUISourceCode.get(uiSourceCode));
        this._uiSourceCodeForScriptId[script.scriptId] = uiSourceCode;
        this._scriptForUISourceCode.put(uiSourceCode, script);
        script.pushSourceMapping(this._snippetScriptMapping);
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     * @return {!Array.<!Object>}
     */
    _removeBreakpoints: function(uiSourceCode)
    {
        var breakpointLocations = WebInspector.breakpointManager.breakpointLocationsForUISourceCode(uiSourceCode);
        for (var i = 0; i < breakpointLocations.length; ++i)
            breakpointLocations[i].breakpoint.remove();
        return breakpointLocations;
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     * @param {!Array.<!Object>} breakpointLocations
     */
    _restoreBreakpoints: function(uiSourceCode, breakpointLocations)
    {
        for (var i = 0; i < breakpointLocations.length; ++i) {
            var uiLocation = breakpointLocations[i].uiLocation;
            var breakpoint = breakpointLocations[i].breakpoint;
            WebInspector.breakpointManager.setBreakpoint(uiSourceCode, uiLocation.lineNumber, uiLocation.columnNumber, breakpoint.condition(), breakpoint.enabled());
        }
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     */
    _releaseSnippetScript: function(uiSourceCode)
    {
        var script = this._scriptForUISourceCode.get(uiSourceCode);
        if (!script)
            return null;

        delete this._uiSourceCodeForScriptId[script.scriptId];
        this._scriptForUISourceCode.remove(uiSourceCode);
        delete uiSourceCode._evaluationIndex;
    },

    _debuggerReset: function()
    {
        for (var snippetId in this._uiSourceCodeForSnippetId) {
            var uiSourceCode = this._uiSourceCodeForSnippetId[snippetId];
            this._releaseSnippetScript(uiSourceCode);
        }
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     * @return {string}
     */
    _evaluationSourceURL: function(uiSourceCode)
    {
        var evaluationSuffix = "_" + uiSourceCode._evaluationIndex;
        var snippetId = this._snippetIdForUISourceCode.get(uiSourceCode);
        return WebInspector.Script.snippetSourceURLPrefix + snippetId + evaluationSuffix;
    },

    /**
     * @param {string} sourceURL
     * @return {?string}
     */
    _snippetIdForSourceURL: function(sourceURL)
    {
        var snippetPrefix = WebInspector.Script.snippetSourceURLPrefix;
        if (!sourceURL.startsWith(snippetPrefix))
            return null;
        var splitURL = sourceURL.substring(snippetPrefix.length).split("_");
        var snippetId = splitURL[0];
        return snippetId;
    },

    reset: function()
    {
        /** @type {!Object.<string, !WebInspector.UISourceCode>} */
        this._uiSourceCodeForScriptId = {};
        this._scriptForUISourceCode = new Map();
        /** @type {!Object.<string, !WebInspector.UISourceCode>} */
        this._uiSourceCodeForSnippetId = {};
        this._snippetIdForUISourceCode = new Map();
        this._projectDelegate.reset();
        this._loadSnippets();
    },

    __proto__: WebInspector.Object.prototype
}

/**
 * @constructor
 * @implements {WebInspector.ScriptSourceMapping}
 * @param {!WebInspector.ScriptSnippetModel} scriptSnippetModel
 */
WebInspector.SnippetScriptMapping = function(scriptSnippetModel)
{
    this._scriptSnippetModel = scriptSnippetModel;
}

WebInspector.SnippetScriptMapping.prototype = {
    /**
     * @param {!WebInspector.RawLocation} rawLocation
     * @return {?WebInspector.UILocation}
     */
    rawLocationToUILocation: function(rawLocation)
    {
        var debuggerModelLocation = /** @type {!WebInspector.DebuggerModel.Location} */(rawLocation);
        return this._scriptSnippetModel._rawLocationToUILocation(debuggerModelLocation);
    },

    /**
     * @param {!WebInspector.UISourceCode} uiSourceCode
     * @param {number} lineNumber
     * @param {number} columnNumber
     * @return {?WebInspector.DebuggerModel.Location}
     */
    uiLocationToRawLocation: function(uiSourceCode, lineNumber, columnNumber)
    {
        return this._scriptSnippetModel._uiLocationToRawLocation(uiSourceCode, lineNumber, columnNumber);
    },

    /**
     * @param {string} sourceURL
     * @return {?string}
     */
    snippetIdForSourceURL: function(sourceURL)
    {
        return this._scriptSnippetModel._snippetIdForSourceURL(sourceURL);
    },

    /**
     * @param {!WebInspector.Script} script
     */
    addScript: function(script)
    {
        this._scriptSnippetModel._addScript(script);
    },

    /**
     * @return {boolean}
     */
    isIdentity: function()
    {
        return false;
    },
}

/**
 * @constructor
 * @implements {WebInspector.ContentProvider}
 * @param {!WebInspector.Snippet} snippet
 */
WebInspector.SnippetContentProvider = function(snippet)
{
    this._snippet = snippet;
}

WebInspector.SnippetContentProvider.prototype = {
    /**
     * @return {string}
     */
    contentURL: function()
    {
        return "";
    },

    /**
     * @return {!WebInspector.ResourceType}
     */
    contentType: function()
    {
        return WebInspector.resourceTypes.Script;
    },

    /**
     * @param {function(?string)} callback
     */
    requestContent: function(callback)
    {
        callback(this._snippet.content);
    },

    /**
     * @param {string} query
     * @param {boolean} caseSensitive
     * @param {boolean} isRegex
     * @param {function(!Array.<!WebInspector.ContentProvider.SearchMatch>)} callback
     */
    searchInContent: function(query, caseSensitive, isRegex, callback)
    {
        /**
         * @this {WebInspector.SnippetContentProvider}
         */
        function performSearch()
        {
            callback(WebInspector.ContentProvider.performSearchInContent(this._snippet.content, query, caseSensitive, isRegex));
        }

        // searchInContent should call back later.
        window.setTimeout(performSearch.bind(this), 0);
    }
}

/**
 * @constructor
 * @extends {WebInspector.ContentProviderBasedProjectDelegate}
 * @param {!WebInspector.Workspace} workspace
 * @param {!WebInspector.ScriptSnippetModel} model
 * @param {string} id
 */
WebInspector.SnippetsProjectDelegate = function(workspace, model, id)
{
    WebInspector.ContentProviderBasedProjectDelegate.call(this, workspace, id, WebInspector.projectTypes.Snippets);
    this._model = model;
}

WebInspector.SnippetsProjectDelegate.prototype = {
    /**
     * @param {string} name
     * @param {!WebInspector.ContentProvider} contentProvider
     * @return {string}
     */
    addSnippet: function(name, contentProvider)
    {
        return this.addContentProvider("", name, name, contentProvider, true);
    },

    /**
     * @return {boolean}
     */
    canSetFileContent: function()
    {
        return true;
    },

    /**
     * @param {string} path
     * @param {string} newContent
     * @param {function(?string)} callback
     */
    setFileContent: function(path, newContent, callback)
    {
        this._model._setScriptSnippetContent(path, newContent);
        callback("");
    },

    /**
     * @return {boolean}
     */
    canRename: function()
    {
        return true;
    },

    /**
     * @param {string} path
     * @param {string} newName
     * @param {function(boolean, string=)} callback
     */
    performRename: function(path, newName, callback)
    {
        this._model.renameScriptSnippet(path, newName, callback);
    },

    /**
     * @param {string} path
     * @param {?string} name
     * @param {string} content
     * @param {function(?string)} callback
     */
    createFile: function(path, name, content, callback)
    {
        var filePath = this._model.createScriptSnippet(content);
        callback(filePath);
    },

    /**
     * @param {string} path
     */
    deleteFile: function(path)
    {
        this._model.deleteScriptSnippet(path);
    },

    __proto__: WebInspector.ContentProviderBasedProjectDelegate.prototype
}

/**
 * @type {!WebInspector.ScriptSnippetModel}
 */
WebInspector.scriptSnippetModel;
