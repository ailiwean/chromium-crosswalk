# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

try:
    import jsonresults
    from jsonresults import JsonResults
except ImportError:
    print "ERROR: Add the TestResultServer, google_appengine and yaml/lib directories to your PYTHONPATH"
    raise

# FIXME: Once we're on python 2.7, just use json directly.
try:
    from django.utils import simplejson
except:
    import json as simplejson

import unittest

FULL_RESULT_EXAMPLE = """ADD_RESULTS({
    "seconds_since_epoch": 1368146629,
    "tests": {
        "media": {
            "encrypted-media": {
                "encrypted-media-v2-events.html": {
                    "bugs": ["crbug.com/1234"],
                    "expected": "TIMEOUT",
                    "actual": "TIMEOUT",
                    "time": 6.0
                },
                "encrypted-media-v2-syntax.html": {
                    "expected": "TIMEOUT",
                    "actual": "TIMEOUT"
                }
            },
            "progress-events-generated-correctly.html": {
                "expected": "PASS FAIL IMAGE TIMEOUT CRASH MISSING",
                "actual": "TIMEOUT",
                "time": 6.0
            },
            "W3C": {
                "audio": {
                    "src": {
                        "src_removal_does_not_trigger_loadstart.html": {
                            "expected": "PASS",
                            "actual": "PASS",
                            "time": 3.5
                        }
                    }
                },
                "video": {
                    "src": {
                        "src_removal_does_not_trigger_loadstart.html": {
                            "expected": "PASS",
                            "actual": "PASS",
                            "time": 1.1
                        },
                        "notrun.html": {
                            "expected": "NOTRUN",
                            "actual": "SKIP",
                            "time": 1.1
                        }
                    }
                }
            },
            "unexpected-skip.html": {
                "expected": "PASS",
                "actual": "SKIP"
            },
            "media-document-audio-repaint.html": {
                "expected": "IMAGE",
                "image_diff_percent": 0,
                "actual": "IMAGE",
                "time": 0.1
            }
        }
    },
    "skipped": 2,
    "num_regressions": 0,
    "build_number": "3",
    "interrupted": false,
    "num_missing": 0,
    "uses_expectations_file": true,
    "layout_tests_dir": "\/tmp\/cr\/src\/third_party\/WebKit\/LayoutTests",
    "version": 3,
    "builder_name": "Webkit",
    "num_passes": 10,
    "pixel_tests_enabled": true,
    "blink_revision": "1234",
    "has_pretty_patch": true,
    "fixable": 25,
    "num_flaky": 0,
    "num_failures_by_type": {
        "CRASH": 3,
        "MISSING": 0,
        "TEXT": 3,
        "IMAGE": 1,
        "PASS": 10,
        "SKIP": 2,
        "TIMEOUT": 16,
        "IMAGE+TEXT": 0,
        "FAIL": 0,
        "AUDIO": 0
    },
    "has_wdiff": true,
    "chromium_revision": "5678"
});"""

JSON_RESULTS_OLD_TEMPLATE = (
    '{"Webkit":{'
    '"allFixableCount":[[TESTDATA_COUNT]],'
    '"blinkRevision":[[TESTDATA_WEBKITREVISION]],'
    '"webkitRevision":[[TESTDATA_WEBKITREVISION]],'
    '"buildNumbers":[[TESTDATA_BUILDNUMBERS]],'
    '"chromeRevision":[[TESTDATA_CHROMEREVISION]],'
    '"deferredCounts":[[TESTDATA_COUNTS]],'
    '"fixableCount":[[TESTDATA_COUNT]],'
    '"fixableCounts":[[TESTDATA_COUNTS]],'
    '"num_failures_by_type":{"AUDIO":[[TESTDATA_COUNT]],"CRASH":[[TESTDATA_COUNT]],"FAIL":[[TESTDATA_COUNT]],"IMAGE":[[TESTDATA_COUNT]],"IMAGE+TEXT":[[TESTDATA_COUNT]],"MISSING":[[TESTDATA_COUNT]],"PASS":[[TESTDATA_COUNT]],"SKIP":[[TESTDATA_COUNT]],"TEXT":[[TESTDATA_COUNT]],"TIMEOUT":[[TESTDATA_COUNT]]},'
    '"secondsSinceEpoch":[[TESTDATA_TIMES]],'
    '"tests":{[TESTDATA_TESTS]},'
    '"wontfixCounts":[[TESTDATA_COUNTS]]'
    '},'
    '"failure_map":{"A":"AUDIO","C":"CRASH","F":"TEXT","I":"IMAGE","N":"NO DATA","O":"MISSING","P":"PASS","T":"TIMEOUT","X":"SKIP","Y":"NOTRUN","Z":"IMAGE+TEXT"},'
    '"version":[VERSION]'
    '}')

JSON_RESULTS_TEMPLATE = (
    '{"Webkit":{'
    '"allFixableCount":[[TESTDATA_COUNT]],'
    '"blinkRevision":[[TESTDATA_WEBKITREVISION]],'
    '"buildNumbers":[[TESTDATA_BUILDNUMBERS]],'
    '"chromeRevision":[[TESTDATA_CHROMEREVISION]],'
    '"fixableCount":[[TESTDATA_COUNT]],'
    '"fixableCounts":[[TESTDATA_COUNTS]],'
    '"num_failures_by_type":{"AUDIO":[[TESTDATA_COUNT]],"CRASH":[[TESTDATA_COUNT]],"FAIL":[[TESTDATA_COUNT]],"IMAGE":[[TESTDATA_COUNT]],"IMAGE+TEXT":[[TESTDATA_COUNT]],"MISSING":[[TESTDATA_COUNT]],"PASS":[[TESTDATA_COUNT]],"SKIP":[[TESTDATA_COUNT]],"TEXT":[[TESTDATA_COUNT]],"TIMEOUT":[[TESTDATA_COUNT]]},'
    '"secondsSinceEpoch":[[TESTDATA_TIMES]],'
    '"tests":{[TESTDATA_TESTS]}'
    '},'
    '"failure_map":{"A":"AUDIO","C":"CRASH","F":"TEXT","I":"IMAGE","N":"NO DATA","O":"MISSING","P":"PASS","T":"TIMEOUT","X":"SKIP","Y":"NOTRUN","Z":"IMAGE+TEXT"},'
    '"version":[VERSION]'
    '}')

JSON_RESULTS_COUNTS_TEMPLATE = (
    '{'
    '"C":[TESTDATA],'
    '"F":[TESTDATA],'
    '"I":[TESTDATA],'
    '"O":[TESTDATA],'
    '"P":[TESTDATA],'
    '"T":[TESTDATA],'
    '"X":[TESTDATA],'
    '"Z":[TESTDATA]}')

JSON_RESULTS_DIRECTORY_TEMPLATE = '[[TESTDATA_DIRECTORY]]:{[TESTDATA_DATA]}'

JSON_RESULTS_TESTS_TEMPLATE = (
    '[[TESTDATA_TEST_NAME]]:{'
    '"results":[[TESTDATA_TEST_RESULTS]],'
    '"times":[[TESTDATA_TEST_TIMES]]}')

JSON_RESULTS_TEST_LIST_TEMPLATE = (
    '{"Webkit":{"tests":{[TESTDATA_TESTS]}}}')


class JsonResultsTest(unittest.TestCase):
    def setUp(self):
        self._builder = "Webkit"

    def test_strip_prefix_suffix(self):
        json = "['contents']"
        self.assertEqual(JsonResults._strip_prefix_suffix("ADD_RESULTS(" + json + ");"), json)
        self.assertEqual(JsonResults._strip_prefix_suffix(json), json)

    def _make_test_json(self, test_data, json_string=JSON_RESULTS_TEMPLATE):
        if not test_data:
            return ""

        builds = test_data["builds"]
        tests = test_data["tests"]
        if not builds or not tests:
            return ""

        counts = []
        build_numbers = []
        webkit_revision = []
        chrome_revision = []
        times = []
        for build in builds:
            counts.append(JSON_RESULTS_COUNTS_TEMPLATE.replace("[TESTDATA]", build))
            build_numbers.append("1000%s" % build)
            webkit_revision.append("2000%s" % build)
            chrome_revision.append("3000%s" % build)
            times.append("100000%s000" % build)

        json_string = json_string.replace("[TESTDATA_COUNTS]", ",".join(counts))
        json_string = json_string.replace("[TESTDATA_COUNT]", ",".join(builds))
        json_string = json_string.replace("[TESTDATA_BUILDNUMBERS]", ",".join(build_numbers))
        json_string = json_string.replace("[TESTDATA_WEBKITREVISION]", ",".join(webkit_revision))
        json_string = json_string.replace("[TESTDATA_CHROMEREVISION]", ",".join(chrome_revision))
        json_string = json_string.replace("[TESTDATA_TIMES]", ",".join(times))

        version = str(test_data["version"]) if "version" in test_data else "4"
        json_string = json_string.replace("[VERSION]", version)
        json_string = json_string.replace("{[TESTDATA_TESTS]}", simplejson.dumps(tests, separators=(',', ':'), sort_keys=True))
        return json_string

    def _test_merge(self, aggregated_data, incremental_data, expected_data, max_builds=jsonresults.JSON_RESULTS_MAX_BUILDS):
        aggregated_results = self._make_test_json(aggregated_data)
        incremental_results = self._make_test_json(incremental_data)
        merged_results = JsonResults.merge(self._builder, aggregated_results, incremental_results, is_full_results_format=False, num_runs=max_builds, sort_keys=True)

        if expected_data:
            expected_results = self._make_test_json(expected_data)
            self.assertEqual(merged_results, expected_results)
        else:
            self.assertFalse(merged_results)

    def _test_get_test_list(self, input_data, expected_data):
        input_results = self._make_test_json(input_data)
        expected_results = JSON_RESULTS_TEST_LIST_TEMPLATE.replace("{[TESTDATA_TESTS]}", simplejson.dumps(expected_data, separators=(',', ':')))
        actual_results = JsonResults.get_test_list(self._builder, input_results)
        self.assertEqual(actual_results, expected_results)

    def test_merge_with_empty_aggregated_results(self):
        incremental_data = {
            "builds": ["2", "1"],
            "tests": {
                "001.html": {
                    "results": [[200, "F"]],
                    "times": [[200, 0]],
                }
            }
        }
        incremental_results = self._make_test_json(incremental_data)
        aggregated_results = ""
        merged_results = JsonResults.merge(self._builder, aggregated_results, incremental_results, is_full_results_format=False, num_runs=jsonresults.JSON_RESULTS_MAX_BUILDS, sort_keys=True)
        self.assertEqual(merged_results, incremental_results)

    def test_old_keys_deleted(self):
        aggregated_results = self._make_test_json({
            "builds": ["2", "1"],
            "tests": {
                "001.html": {
                    "results": [[100, "F"]],
                    "times": [[100, 0]],
                }
            }
        }, json_string=JSON_RESULTS_OLD_TEMPLATE)
        incremental_results = self._make_test_json({
            "builds": ["3"],
            "tests": {
                "001.html": {
                    "results": [[1, "F"]],
                    "times": [[1, 0]],
                }
            }
        })
        merged_results = JsonResults.merge(self._builder, aggregated_results, incremental_results, is_full_results_format=False, num_runs=200, sort_keys=True)
        self.assertEqual(merged_results, self._make_test_json({
            "builds": ["3", "2", "1"],
            "tests": {
                "001.html": {
                    "results": [[101, "F"]],
                    "times": [[101, 0]],
                }
            }
        }))

    def test_merge_full_results_format(self):
        expected_incremental_results = ('{"Webkit":{'
            '"allFixableCount":[35],'
            '"blinkRevision":["1234"],'
            '"buildNumbers":["3"],'
            '"chromeRevision":["5678"],'
            '"fixableCount":[25],'
            '"fixableCounts":[{"A":0,"C":3,"F":3,"I":1,"O":0,"P":10,"T":16,"X":2,"Z":0}],'
            '"num_failures_by_type":{"AUDIO":[0],"CRASH":[3],"IMAGE":[1],"IMAGE+TEXT":[0],"MISSING":[0],"PASS":[10],"SKIP":[2],"TEXT":[3],"TIMEOUT":[16]},'
            '"secondsSinceEpoch":[1368146629],'
            '"tests":{'
                '"media":{'
                    '"W3C":{'
                        '"audio":{'
                            '"src":{'
                                '"src_removal_does_not_trigger_loadstart.html":{'
                                    '"results":[[1,"P"]],'
                                    '"times":[[1,4]]'
                                '}'
                            '}'
                        '}'
                    '},'
                    '"encrypted-media":{'
                        '"encrypted-media-v2-events.html":{'
                            '"bugs":["crbug.com/1234"],'
                            '"expected":"TIMEOUT",'
                            '"results":[[1,"T"]],'
                            '"times":[[1,6]]'
                        '},'
                        '"encrypted-media-v2-syntax.html":{'
                            '"expected":"TIMEOUT",'
                            '"results":[[1,"T"]],'
                            '"times":[[1,0]]'
                        '}'
                    '},'
                    '"media-document-audio-repaint.html":{'
                        '"expected":"IMAGE",'
                        '"results":[[1,"I"]],'
                        '"times":[[1,0]]'
                    '},'
                    '"progress-events-generated-correctly.html":{'
                        '"expected":"PASS FAIL IMAGE TIMEOUT CRASH MISSING",'
                        '"results":[[1,"T"]],'
                        '"times":[[1,6]]'
                    '}'
                '}'
            '}'
        '},'
        '"failure_map":{"A":"AUDIO","C":"CRASH","F":"TEXT","I":"IMAGE","N":"NO DATA","O":"MISSING","P":"PASS","T":"TIMEOUT","X":"SKIP","Y":"NOTRUN","Z":"IMAGE+TEXT"},'
        '"version":4}')

        aggregated_results = ""
        merged_results = JsonResults.merge("Webkit", aggregated_results, FULL_RESULT_EXAMPLE, is_full_results_format=True, num_runs=jsonresults.JSON_RESULTS_MAX_BUILDS, sort_keys=True)
        self.assertEqual(merged_results, expected_incremental_results)

    def test_merge_null_incremental_results(self):
        # Empty incremental results json.
        # Nothing to merge.
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}},
            # Incremental results
            None,
            # Expect no merge happens.
            None)

    def test_merge_empty_incremental_results(self):
        # No actual incremental test results (only prefix and suffix) to merge.
        # Nothing to merge.
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}},
            # Incremental results
            {"builds": [],
             "tests": {}},
            # Expected no merge happens.
            None)

    def test_merge_empty_aggregated_results(self):
        # No existing aggregated results.
        # Merged results == new incremental results.
        self._test_merge(
            # Aggregated results
            None,
            # Incremental results

            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}},
            # Expected result
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}})

    def test_merge_duplicate_build_number(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[100, "F"]],
                           "times": [[100, 0]]}}},
            # Incremental results
            {"builds": ["2"],
             "tests": {"001.html": {
                           "results": [[1, "F"]],
                           "times": [[1, 0]]}}},
            # Expected results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[100, "F"]],
                           "times": [[100, 0]]}}})

    def test_merge_incremental_single_test_single_run_same_result(self):
        # Incremental results has the latest build and same test results for
        # that run.
        # Insert the incremental results at the first place and sum number
        # of runs for "F" (200 + 1) to get merged results.
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"F"]],
                           "times": [[1,0]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[201,"F"]],
                           "times": [[201,0]]}}})

    def test_merge_single_test_single_run_different_result(self):
        # Incremental results has the latest build but different test results
        # for that run.
        # Insert the incremental results at the first place.
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1, "I"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[1,"I"],[200,"F"]],
                           "times": [[1,1],[200,0]]}}})

    def test_merge_single_test_single_run_result_changed(self):
        # Incremental results has the latest build but results which differ from
        # the latest result (but are the same as an older result).
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"],[10,"I"]],
                           "times": [[200,0],[10,1]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"I"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[1,"I"],[200,"F"],[10,"I"]],
                           "times": [[1,1],[200,0],[10,1]]}}})

    def test_merge_multiple_tests_single_run(self):
        # All tests have incremental updates.
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]},
                       "002.html": {
                           "results": [[100,"I"]],
                           "times": [[100,1]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"F"]],
                           "times": [[1,0]]},
                       "002.html": {
                           "results": [[1,"I"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[201,"F"]],
                           "times": [[201,0]]},
                       "002.html": {
                           "results": [[101,"I"]],
                           "times": [[101,1]]}}})

    def test_merge_multiple_tests_single_run_one_no_result(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]},
                       "002.html": {
                           "results": [[100,"I"]],
                           "times": [[100,1]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"002.html": {
                           "results": [[1,"I"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[1,"N"],[200,"F"]],
                           "times": [[201,0]]},
                       "002.html": {
                           "results": [[101,"I"]],
                           "times": [[101,1]]}}})

    def test_merge_single_test_multiple_runs(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]}}},
            # Incremental results
            {"builds": ["4", "3"],
             "tests": {"001.html": {
                           "results": [[2, "I"]],
                           "times": [[2,2]]}}},
            # Expected results
            {"builds": ["4", "3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[2,"I"],[200,"F"]],
                           "times": [[2,2],[200,0]]}}})

    def test_merge_multiple_tests_multiple_runs(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"F"]],
                           "times": [[200,0]]},
                       "002.html": {
                           "results": [[10,"Z"]],
                           "times": [[10,0]]}}},
            # Incremental results
            {"builds": ["4", "3"],
             "tests": {"001.html": {
                           "results": [[2, "I"]],
                           "times": [[2,2]]},
                       "002.html": {
                           "results": [[1,"C"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["4", "3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[2,"I"],[200,"F"]],
                           "times": [[2,2],[200,0]]},
                       "002.html": {
                           "results": [[1,"C"],[10,"Z"]],
                           "times": [[1,1],[10,0]]}}})

    def test_merge_incremental_result_older_build(self):
        # Test the build in incremental results is older than the most recent
        # build in aggregated results.
        self._test_merge(
            # Aggregated results
            {"builds": ["3", "1"],
             "tests": {"001.html": {
                           "results": [[5,"F"]],
                           "times": [[5,0]]}}},
            # Incremental results
            {"builds": ["2"],
             "tests": {"001.html": {
                           "results": [[1, "F"]],
                           "times": [[1,0]]}}},
            # Expected no merge happens.
            {"builds": ["2", "3", "1"],
             "tests": {"001.html": {
                           "results": [[6,"F"]],
                           "times": [[6,0]]}}})

    def test_merge_incremental_result_same_build(self):
        # Test the build in incremental results is same as the build in
        # aggregated results.
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[5,"F"]],
                           "times": [[5,0]]}}},
            # Incremental results
            {"builds": ["3", "2"],
             "tests": {"001.html": {
                           "results": [[2, "F"]],
                           "times": [[2,0]]}}},
            # Expected no merge happens.
            {"builds": ["3", "2", "2", "1"],
             "tests": {"001.html": {
                           "results": [[7,"F"]],
                           "times": [[7,0]]}}})

    def test_merge_remove_new_test(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[199, "F"]],
                           "times": [[199, 0]]},
                       }},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1, "F"]],
                           "times": [[1, 0]]},
                       "002.html": {
                           "results": [[1, "P"]],
                           "times": [[1, 0]]},
                       "notrun.html": {
                           "results": [[1, "Y"]],
                           "times": [[1, 0]]},
                       "003.html": {
                           "results": [[1, "N"]],
                           "times": [[1, 0]]},
                        }},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[200, "F"]],
                           "times": [[200, 0]]},
                       }},
            max_builds=200)

    def test_merge_remove_test(self):
        self._test_merge(
            # Aggregated results
            {
                "builds": ["2", "1"],
                "tests": {
                    "directory": {
                        "directory": {
                            "001.html": {
                                "results": [[200, "P"]],
                                "times": [[200, 0]]
                            }
                        }
                    },
                    "002.html": {
                        "results": [[10, "F"]],
                        "times": [[10, 0]]
                    },
                    "003.html": {
                        "results": [[190, 'P'], [9, 'N'], [1,"F"]],
                        "times": [[200, 0]]
                    },
                }
            },
            # Incremental results
            {
                "builds": ["3"],
                "tests": {
                    "directory": {
                        "directory": {
                            "001.html": {
                                "results": [[1, "P"]],
                                "times": [[1, 0]]
                            }
                        }
                    },
                    "002.html": {
                        "results": [[1, "P"]],
                        "times": [[1, 0]]
                    },
                    "003.html": {
                        "results": [[1, "P"]],
                        "times": [[1, 0]]
                    },
                }
            },
            # Expected results
            {
                "builds": ["3", "2", "1"],
                "tests": {
                    "002.html": {
                        "results": [[1, "P"],[10, "F"]],
                        "times": [[11, 0]]
                    }
                }
            },
            max_builds=200)

    def test_merge_updates_expected(self):
        self._test_merge(
            # Aggregated results
            {
                "builds": ["2", "1"],
                "tests": {
                    "directory": {
                        "directory": {
                            "001.html": {
                                "expected": "FAIL",
                                "results": [[200, "P"]],
                                "times": [[200, 0]]
                            }
                        }
                    },
                    "002.html": {
                        "bugs": ["crbug.com/1234"],
                        "expected": "FAIL",
                        "results": [[10, "F"]],
                        "times": [[10, 0]]
                    },
                    "003.html": {
                        "expected": "FAIL",
                        "results": [[190, 'P'], [9, 'N'], [1,"F"]],
                        "times": [[200, 0]]
                    },
                    "004.html": {
                        "results": [[199, 'P'], [1,"F"]],
                        "times": [[200, 0]]
                    },
                }
            },
            # Incremental results
            {
                "builds": ["3"],
                "tests": {
                    "002.html": {
                        "expected": "PASS",
                        "results": [[1, "P"]],
                        "times": [[1, 0]]
                    },
                    "003.html": {
                        "expected": "TIMEOUT",
                        "results": [[1, "P"]],
                        "times": [[1, 0]]
                    },
                    "004.html": {
                        "bugs": ["crbug.com/1234"],
                        "results": [[1, "P"]],
                        "times": [[1, 0]]
                    },
                }
            },
            # Expected results
            {
                "builds": ["3", "2", "1"],
                "tests": {
                    "002.html": {
                        "results": [[1, "P"],[10, "F"]],
                        "times": [[11, 0]]
                    },
                    "003.html": {
                        "expected": "TIMEOUT",
                        "results": [[191, 'P'], [9, 'N']],
                        "times": [[200, 0]]
                    },
                    "004.html": {
                        "bugs": ["crbug.com/1234"],
                        "results": [[200, 'P']],
                        "times": [[200, 0]]
                    },
                }
            },
            max_builds=200)


    def test_merge_keep_test_with_all_pass_but_slow_time(self):
        # Do not remove test where all run pass but max running time >= 5 seconds
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[200,"P"]],
                           "times": [[200,jsonresults.JSON_RESULTS_MIN_TIME]]},
                       "002.html": {
                           "results": [[10,"F"]],
                           "times": [[10,0]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"P"]],
                           "times": [[1,1]]},
                       "002.html": {
                           "results": [[1,"P"]],
                           "times": [[1,0]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[201,"P"]],
                           "times": [[1,1],[200,jsonresults.JSON_RESULTS_MIN_TIME]]},
                       "002.html": {
                           "results": [[1,"P"],[10,"F"]],
                           "times": [[11,0]]}}})

    def test_merge_prune_extra_results(self):
        # Remove items from test results and times that exceed the max number
        # of builds to track.
        max_builds = jsonresults.JSON_RESULTS_MAX_BUILDS
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[max_builds,"F"],[1,"I"]],
                           "times": [[max_builds,0],[1,1]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"T"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[1,"T"],[max_builds,"F"]],
                           "times": [[1,1],[max_builds,0]]}}})

    def test_merge_prune_extra_results_small(self):
        # Remove items from test results and times that exceed the max number
        # of builds to track, using smaller threshold.
        max_builds = jsonresults.JSON_RESULTS_MAX_BUILDS_SMALL
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[max_builds,"F"],[1,"I"]],
                           "times": [[max_builds,0],[1,1]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"T"]],
                           "times": [[1,1]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[1,"T"],[max_builds,"F"]],
                           "times": [[1,1],[max_builds,0]]}}},
            int(max_builds))

    def test_merge_prune_extra_results_with_new_result_of_same_type(self):
        # Test that merging in a new result of the same type as the last result
        # causes old results to fall off.
        max_builds = jsonresults.JSON_RESULTS_MAX_BUILDS_SMALL
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"001.html": {
                           "results": [[max_builds,"F"],[1,"N"]],
                           "times": [[max_builds,0],[1,1]]}}},
            # Incremental results
            {"builds": ["3"],
             "tests": {"001.html": {
                           "results": [[1,"F"]],
                           "times": [[1,0]]}}},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"001.html": {
                           "results": [[max_builds,"F"]],
                           "times": [[max_builds,0]]}}},
            int(max_builds))

    def test_merge_build_directory_hierarchy(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"bar": {"baz": {
                           "003.html": {
                                "results": [[25,"F"]],
                                "times": [[25,0]]}}},
                       "foo": {
                           "001.html": {
                                "results": [[50,"F"]],
                                "times": [[50,0]]},
                           "002.html": {
                                "results": [[100,"I"]],
                                "times": [[100,0]]}}},
              "version": 4},
            # Incremental results
            {"builds": ["3"],
             "tests": {"baz": {
                           "004.html": {
                               "results": [[1,"I"]],
                               "times": [[1,0]]}},
                       "foo": {
                           "001.html": {
                               "results": [[1,"F"]],
                               "times": [[1,0]]},
                           "002.html": {
                               "results": [[1,"I"]],
                               "times": [[1,0]]}}},
             "version": 4},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"bar": {"baz": {
                           "003.html": {
                               "results": [[1,"N"],[25,"F"]],
                               "times": [[26,0]]}}},
                       "baz": {
                           "004.html": {
                               "results": [[1,"I"]],
                               "times": [[1,0]]}},
                       "foo": {
                           "001.html": {
                               "results": [[51,"F"]],
                               "times": [[51,0]]},
                           "002.html": {
                               "results": [[101,"I"]],
                               "times": [[101,0]]}}},
             "version": 4})

    # FIXME(aboxhall): Add some tests for xhtml/svg test results.

    def test_get_test_name_list(self):
        # Get test name list only. Don't include non-test-list data and
        # of test result details.
        # FIXME: This also tests a temporary bug in the data where directory-level
        # results have a results and times values. Once that bug is fixed,
        # remove this test-case and assert we don't ever hit it.
        self._test_get_test_list(
            # Input results
            {"builds": ["3", "2", "1"],
             "tests": {"foo": {
                           "001.html": {
                               "results": [[200,"P"]],
                               "times": [[200,0]]},
                           "results": [[1,"N"]],
                           "times": [[1,0]]},
                       "002.html": {
                           "results": [[10,"F"]],
                           "times": [[10,0]]}}},
            # Expected results
            {"foo": {"001.html":{}}, "002.html":{}})

    def test_gtest(self):
        self._test_merge(
            # Aggregated results
            {"builds": ["2", "1"],
             "tests": {"foo.bar": {
                           "results": [[50,"F"]],
                           "times": [[50,0]]},
                       "foo.bar2": {
                           "results": [[100,"I"]],
                           "times": [[100,0]]},
                       },
             "version": 3},
            # Incremental results
            {"builds": ["3"],
             "tests": {"foo.bar2": {
                           "results": [[1,"I"]],
                           "times": [[1,0]]},
                       "foo.bar3": {
                           "results": [[1,"F"]],
                           "times": [[1,0]]},
                       },
             "version": 4},
            # Expected results
            {"builds": ["3", "2", "1"],
             "tests": {"foo.bar": {
                           "results": [[1, "N"], [50,"F"]],
                           "times": [[51,0]]},
                       "foo.bar2": {
                           "results": [[101,"I"]],
                           "times": [[101,0]]},
                       "foo.bar3": {
                           "results": [[1,"F"]],
                           "times": [[1,0]]},
                       },
             "version": 4})

if __name__ == '__main__':
    unittest.main()
