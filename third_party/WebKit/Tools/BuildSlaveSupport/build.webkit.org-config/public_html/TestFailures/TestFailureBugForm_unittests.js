/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

(function() {

module('TestFailureBugForm');

function MockBuilder(name) {
    this.name = name;
}

function createTestForm(testerName, failingBuildName, passingBuildName, failingTests) {
    var mockBugzilla = {};
    mockBugzilla.baseURL = '[BUGZILLA BASE URL]';

    var mockTrac = {};
    mockTrac.changesetURL = function(revisionNumber) {
        return '[CHANGESET URL r' + revisionNumber + ']';
    }
    mockTrac.logURL = function(path, startRevision, endRevision) {
        return '[LOG URL ' + path + ', r' + startRevision + ', r' + endRevision + ']';
    }

    var mockBuildbot = {};
    mockBuildbot.parseBuildName = function(buildName) {
        var match = /(\d+)/.exec(buildName);
        return {
            revision: parseInt(match[1], 10),
            buildNumber: parseInt(match[2], 10),
        };
    };

    var mockBuilder = {};
    mockBuilder.name = testerName;
    mockBuilder.buildbot = mockBuildbot;
    mockBuilder.resultsPageURL = function(buildName) {
        return '[RESULTS PAGE URL ' + this.name + ', ' + buildName + ']';
    }

    return new TestFailureBugForm(mockBugzilla, mockTrac, mockBuilder, failingBuildName, passingBuildName, failingTests);
}

test('component and keywords are set', 2, function() {
    var form = createTestForm('Windows 7 Release (Tests)', 'r10 (5)', 'r8 (2)', ['css1/basic/class_as_selector.html']);

    equal(form.component, BugzillaConstants.Component.ToolsTests);
    deepEqual(form.keywords.split(', '), [BugzillaConstants.Keyword.LayoutTestFailure, BugzillaConstants.Keyword.MakingBotsRed, BugzillaConstants.Keyword.Regression]);
});

const testers = {
    'GTK Linux 32-bit Release': {
        operatingSystem: '',
        platform: '',
    },
    'Leopard Intel Release (Tests)': {
        operatingSystem: BugzillaConstants.OperatingSystem.Leopard,
        platform: BugzillaConstants.Platform.Macintosh,
    },
    'SnowLeopard Intel Release (Tests)': {
        operatingSystem: BugzillaConstants.OperatingSystem.SnowLeopard,
        platform: BugzillaConstants.Platform.Macintosh,
    },
    'Windows 7 Release (Tests)': {
        operatingSystem: BugzillaConstants.OperatingSystem.Windows7,
        platform: BugzillaConstants.Platform.PC,
    },
    'Windows XP Debug (Tests)': {
        operatingSystem: BugzillaConstants.OperatingSystem.WindowsXP,
        platform: BugzillaConstants.Platform.PC,
    },
};

test('operating system is deduced', 5, function() {
    for (var name in testers) {
        var form = createTestForm(name, 'r10 (5)', 'r8 (2)', ['css1/basic/class_as_selector.html']);
        equal(form.operatingSystem, testers[name].operatingSystem);
    }
});

test('platform is deduced', 5, function() {
    for (var name in testers) {
        var form = createTestForm(name, 'r10 (5)', 'r8 (2)', ['css1/basic/class_as_selector.html']);
        equal(form.platform, testers[name].platform);
    }
});

const testCases = [
    {
        failingBuildName: 'r10 (5)',
        failingTests: [
            'css1/basic/class_as_selector.html',
        ],
        expectedDescription: 'css1/basic/class_as_selector.html has been failing on Windows 7 Release (Tests) since at least r10 <[CHANGESET URL r10]>.\n\n[RESULTS PAGE URL Windows 7 Release (Tests), r10 (5)] failed\n',
        expectedTitle: 'REGRESSION (r10): css1/basic/class_as_selector.html failing on Windows 7 Release (Tests)',
    },
    {
        failingBuildName: 'r10 (5)',
        passingBuildName: 'r9 (3)',
        failingTests: [
            'css1/basic/class_as_selector.html',
        ],
        expectedDescription: 'css1/basic/class_as_selector.html started failing on Windows 7 Release (Tests) in r10 <[CHANGESET URL r10]>.\n\n[RESULTS PAGE URL Windows 7 Release (Tests), r9 (3)] passed\n[RESULTS PAGE URL Windows 7 Release (Tests), r10 (5)] failed\n',
        expectedTitle: 'REGRESSION (r10): css1/basic/class_as_selector.html failing on Windows 7 Release (Tests)',
    },
    {
        failingBuildName: 'r10 (5)',
        passingBuildName: 'r8 (2)',
        failingTests: [
            'css1/basic/class_as_selector.html',
        ],
        expectedDescription: 'css1/basic/class_as_selector.html started failing on Windows 7 Release (Tests) between r9 and r10 (inclusive).\n\n[LOG URL trunk, r9, r10]\n\n[RESULTS PAGE URL Windows 7 Release (Tests), r8 (2)] passed\n[RESULTS PAGE URL Windows 7 Release (Tests), r10 (5)] failed\n',
        expectedTitle: 'REGRESSION (r8-r10): css1/basic/class_as_selector.html failing on Windows 7 Release (Tests)',
    },
    {
        failingBuildName: 'r10 (5)',
        passingBuildName: 'r8 (2)',
        failingTests: [
            'css1/basic/class_as_selector.html',
            'fast/css/ex-after-font-variant.html',
        ],
        expectedDescription: 'css1/basic/class_as_selector.html and fast/css/ex-after-font-variant.html started failing on Windows 7 Release (Tests) between r9 and r10 (inclusive).\n\n[LOG URL trunk, r9, r10]\n\n[RESULTS PAGE URL Windows 7 Release (Tests), r8 (2)] passed\n[RESULTS PAGE URL Windows 7 Release (Tests), r10 (5)] failed\n',
        expectedTitle: 'REGRESSION (r8-r10): css1/basic/class_as_selector.html, fast/css/ex-after-font-variant.html failing on Windows 7 Release (Tests)',
    },
    {
        failingBuildName: 'r10 (5)',
        passingBuildName: 'r8 (2)',
        failingTests: [
            'css1/basic/class_as_selector1.html',
            'css1/basic/class_as_selector2.html',
            'css1/basic/class_as_selector3.html',
            'css1/basic/class_as_selector4.html',
            'css1/basic/class_as_selector5.html',
            'css1/basic/class_as_selector6.html',
            'css1/basic/class_as_selector7.html',
            'css1/basic/class_as_selector8.html',
        ],
        expectedDescription: 'The following tests started failing on Windows 7 Release (Tests) between r9 and r10 (inclusive):\n\n    css1/basic/class_as_selector1.html\n    css1/basic/class_as_selector2.html\n    css1/basic/class_as_selector3.html\n    css1/basic/class_as_selector4.html\n    css1/basic/class_as_selector5.html\n    css1/basic/class_as_selector6.html\n    css1/basic/class_as_selector7.html\n    css1/basic/class_as_selector8.html\n\n[LOG URL trunk, r9, r10]\n\n[RESULTS PAGE URL Windows 7 Release (Tests), r8 (2)] passed\n[RESULTS PAGE URL Windows 7 Release (Tests), r10 (5)] failed\n',
        expectedTitle: 'REGRESSION (r8-r10): 8 css1/basic tests failing on Windows 7 Release (Tests)',
    },
    {
        failingBuildName: 'r10 (5)',
        passingBuildName: 'r8 (2)',
        failingTests: [
            'css1/basic/class_as_selector1.html',
            'css1/basic/class_as_selector2.html',
            'css1/basic/class_as_selector3.html',
            'css1/basic/class_as_selector4.html',
            'css1/basic/class_as_selector5.html',
            'css1/basic/class_as_selector6.html',
            'css1/basic/class_as_selector7.html',
            'css1/basic/class_as_selector8.html',
            'css1/class_as_selector9.html',
        ],
        expectedTitle: 'REGRESSION (r8-r10): 9 css1 tests failing on Windows 7 Release (Tests)',
    },
    {
        failingBuildName: 'r10 (5)',
        passingBuildName: 'r8 (2)',
        failingTests: [
            'css1/basic/class_as_selector1.html',
            'css1/basic/class_as_selector2.html',
            'css1/basic/class_as_selector3.html',
            'css1/basic/class_as_selector4.html',
            'css1/basic/class_as_selector5.html',
            'css1/basic/class_as_selector6.html',
            'css1/basic/class_as_selector7.html',
            'css1/basic/class_as_selector8.html',
            'css1/class_as_selector9.html',
            'fast/css/ex-after-font-variant.html',
        ],
        expectedTitle: 'REGRESSION (r8-r10): 10 tests failing on Windows 7 Release (Tests)',
    },
];

test('titles', 7, function() {
    for (var i = 0; i < testCases.length; ++i) {
        var form = createTestForm('Windows 7 Release (Tests)', testCases[i].failingBuildName, testCases[i].passingBuildName, testCases[i].failingTests);
        equal(form.title, testCases[i].expectedTitle);
    }
});

test('descriptions', 5, function() {
    for (var i = 0; i < testCases.length; ++i) {
        if (!('expectedDescription' in testCases[i]))
            continue;
        var form = createTestForm('Windows 7 Release (Tests)', testCases[i].failingBuildName, testCases[i].passingBuildName, testCases[i].failingTests);
        equal(form.description, testCases[i].expectedDescription);
    }
});

})();
