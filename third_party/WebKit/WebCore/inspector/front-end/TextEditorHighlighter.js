/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

WebInspector.TextEditorHighlighter = function(textModel, damageCallback)
{
    this._textModel = textModel;
    this._tokenizer = WebInspector.SourceTokenizer.Registry.getInstance().getTokenizer("text/html");
    this._damageCallback = damageCallback;
    this._lastHighlightedLine = 0;
}

WebInspector.TextEditorHighlighter.prototype = {
    set mimeType(mimeType)
    {
        var tokenizer = WebInspector.SourceTokenizer.Registry.getInstance().getTokenizer(mimeType);
        if (tokenizer) {
            this._tokenizer = tokenizer;
            this._tokenizerCondition = this._tokenizer.initialCondition;
        }
    },

    highlight: function(endLine)
    {
        // First check if we have work to do.
        if (endLine <= this._lastHighlightedLine)
            return;

        this._requestedEndLine = endLine;

        if (this._highlightTimer) {
            // There is a timer scheduled, it will catch the new job based on the new endLine set.
            return;
        }

        // Do small highlight synchronously. This will provide instant highlight on PageUp / PageDown, gentle scrolling.
        var toLine = Math.min(this._lastHighlightedLine + 200, endLine);
        this._highlightInChunks(this._lastHighlightedLine, toLine);

        // Schedule tail highlight if necessary.
        if (endLine > toLine)
            this._highlightTimer = setTimeout(this._highlightInChunks.bind(this, toLine, endLine), 100);
    },

    _highlightInChunks: function(startLine, endLine)
    {
        delete this._highlightTimer;

        // First we always check if we have work to do. Could be that user scrolled back and we can quit.
        if (this._requestedEndLine <= this._lastHighlightedLine)
            return;

        if (this._requestedEndLine !== endLine) {
            // User keeps updating the job in between of our timer ticks. Just reschedule self, don't eat CPU (they must be scrolling).
            this._highlightTimer = setTimeout(this._highlightInChunks.bind(this, startLine, this._requestedEndLine), 100);
            return;
        }

        // Highlight 500 lines chunk.
        var toLine = Math.min(startLine + 500, this._requestedEndLine);
        this._highlightLines(startLine, toLine);

        // Schedule tail highlight if necessary.
        if (toLine < this._requestedEndLine)
            this._highlightTimer = setTimeout(this._highlightInChunks.bind(this, toLine, this._requestedEndLine), 10);
    },

    _highlightLines: function(startLine, endLine)
    {
        // Tokenizer is stateless and reused accross viewers, restore its condition before highlight and save it after.
        this._tokenizer.condition = this._tokenizerCondition;
        for (var i = startLine; i < endLine; ++i)
            this._highlightLine(i);
        this._lastHighlightedLine = endLine;
        this._tokenizerCondition = this._tokenizer.condition;

        this._damageCallback(startLine, endLine);
    },

    _highlightLine: function(lineNumber) {
        var line = this._textModel.line(lineNumber);
        var attributes = {};
        this._tokenizer.line = line;
        var column = 0;
        do {
            var newColumn = this._tokenizer.nextToken(column);
            var tokenType = this._tokenizer.tokenType;
            if (tokenType)
                attributes[column] = { length: newColumn - column, tokenType: tokenType, subTokenizer: this._tokenizer.subTokenizer };
            column = newColumn;
        } while (column < line.length)
        this._textModel.setAttribute(lineNumber, "highlight", attributes);
    }
}
