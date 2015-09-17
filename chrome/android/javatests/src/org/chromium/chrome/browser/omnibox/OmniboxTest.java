// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox;

import static org.chromium.chrome.test.util.OmniboxTestUtils.buildSuggestionMap;

import android.os.SystemClock;
import android.support.v4.view.ViewCompat;
import android.test.FlakyTest;
import android.test.suitebuilder.annotation.MediumTest;
import android.test.suitebuilder.annotation.SmallTest;
import android.text.Selection;
import android.util.Pair;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import org.chromium.base.ThreadUtils;
import org.chromium.base.test.util.EnormousTest;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.ScalableTimeout;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.omnibox.AutocompleteController.OnSuggestionsReceivedListener;
import org.chromium.chrome.browser.omnibox.LocationBarLayout.OmniboxSuggestionsList;
import org.chromium.chrome.browser.omnibox.OmniboxSuggestion.Type;
import org.chromium.chrome.test.ChromeActivityTestCaseBase;
import org.chromium.chrome.test.util.ChromeTabUtils;
import org.chromium.chrome.test.util.OmniboxTestUtils;
import org.chromium.chrome.test.util.OmniboxTestUtils.SuggestionsResult;
import org.chromium.chrome.test.util.OmniboxTestUtils.SuggestionsResultBuilder;
import org.chromium.chrome.test.util.OmniboxTestUtils.TestAutocompleteController;
import org.chromium.chrome.test.util.OmniboxTestUtils.TestSuggestionResultsBuilder;
import org.chromium.chrome.test.util.TestHttpServerClient;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;
import org.chromium.content.browser.test.util.KeyUtils;
import org.chromium.content.browser.test.util.UiUtils;
import org.chromium.ui.base.DeviceFormFactor;

import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Tests of the Omnibox.
 */
public class OmniboxTest extends ChromeActivityTestCaseBase<ChromeActivity> {

    public OmniboxTest() {
        super(ChromeActivity.class);
    }

    private void clearUrlBar() {
        final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);
        assertNotNull(urlBar);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBar.setText("");
            }
        });
    }

    /**
     * Sanity check of Omnibox.  The problem in http://b/5021723 would
     * cause this to fail (hang or crash).
     */
    @EnormousTest
    @Feature({"Omnibox"})
    public void testSimpleUse() throws InterruptedException {
        typeInOmnibox("aaaaaaa", false);

        final LocationBarLayout locationBar =
                (LocationBarLayout) getActivity().findViewById(R.id.location_bar);
        OmniboxTestUtils.waitForOmniboxSuggestions(locationBar);

        ChromeTabUtils.waitForTabPageLoadStart(getActivity().getActivityTab(), new Runnable() {
            @Override
            public void run() {
                final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);
                KeyUtils.singleKeyEventView(getInstrumentation(), urlBar, KeyEvent.KEYCODE_ENTER);
            }
        }, ScalableTimeout.scaleTimeout(20));
    }

    /**
     * Test for checking whether soft input model switches with focus.
     */
    @MediumTest
    @Feature({"Omnibox"})
    public void testFocusChangingSoftInputMode() throws InterruptedException {
        final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);

        OmniboxTestUtils.toggleUrlBarFocus(urlBar, true);
        assertTrue("Soft input mode failed to switch on focus",
                CriteriaHelper.pollForCriteria(new Criteria() {
                    @Override
                    public boolean isSatisfied() {
                        return getActivity().getWindow().getAttributes().softInputMode
                                == WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN;
                    }
                }));

        OmniboxTestUtils.toggleUrlBarFocus(urlBar, false);
        assertTrue("Soft input mode failed to switch on unfocus",
                CriteriaHelper.pollForCriteria(new Criteria() {
                    @Override
                    public boolean isSatisfied() {
                        return getActivity().getWindow().getAttributes().softInputMode
                                == WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE;
                    }
                }));
    }

    /**
     * Tests that focusing a url bar starts a zero suggest request.
     */
    @MediumTest
    @Feature({"Omnibox"})
    public void testRequestZeroSuggestOnFocus() throws Exception {
        final LocationBarLayout locationBar =
                (LocationBarLayout) getActivity().findViewById(R.id.location_bar);
        final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);

        final TestAutocompleteController controller = new TestAutocompleteController(
                locationBar, null, null);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                locationBar.setAutocompleteController(controller);
            }
        });
        assertEquals("Should not have any zero suggest requests yet", 0,
                controller.numZeroSuggestRequests());

        OmniboxTestUtils.toggleUrlBarFocus(urlBar, true);

        assertTrue("Should have requested zero suggest on focusing",
                CriteriaHelper.pollForCriteria(new Criteria() {
                    @Override
                    public boolean isSatisfied() {
                        return controller.numZeroSuggestRequests() == 1;
                    }
                }));
    }

    /**
     * Tests that focusing a url bar starts a zero suggest request.
     */
    @MediumTest
    @Feature({"Omnibox"})
    public void testRequestZeroSuggestAfterDelete() throws InterruptedException {
        final LocationBarLayout locationBar =
                (LocationBarLayout) getActivity().findViewById(R.id.location_bar);
        final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);
        final ImageButton deleteButton = (ImageButton) getActivity().findViewById(
                R.id.delete_button);

        final OnSuggestionsReceivedListener emptySuggestionListener =
                new OnSuggestionsReceivedListener() {
                    @Override
                    public void onSuggestionsReceived(
                            List<OmniboxSuggestion> suggestions, String inlineAutocompleteText) {
                    }
                };

        final TestAutocompleteController controller = new TestAutocompleteController(
                locationBar, emptySuggestionListener,
                new HashMap<String, List<SuggestionsResult>>());

        OmniboxTestUtils.toggleUrlBarFocus(urlBar, true);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                locationBar.setAutocompleteController(controller);
                urlBar.setText("g");
            }
        });

        assertTrue("Should have drawn the delete button",
                CriteriaHelper.pollForCriteria(new Criteria() {
                    @Override
                    public boolean isSatisfied() {
                        return deleteButton.getWidth() > 0;
                    }
                }));

        // The click view below ends up clicking on the menu button underneath the delete button
        // for some time after the delete button appears. Wait for UI to settle down before
        // clicking.
        UiUtils.settleDownUI(getInstrumentation());

        singleClickView(deleteButton);

        assertTrue("Should have requested zero suggest results on url bar empty",
                CriteriaHelper.pollForCriteria(new Criteria() {
                    @Override
                    public boolean isSatisfied() {
                        return controller.numZeroSuggestRequests() == 1;
                    }
                }));
    }

    @MediumTest
    @Feature({"Omnibox"})
    public void testRequestZeroSuggestTypeAndBackspace() throws InterruptedException {
        final LocationBarLayout locationBar =
                (LocationBarLayout) getActivity().findViewById(R.id.location_bar);
        final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);

        final OnSuggestionsReceivedListener emptySuggestionListener =
                new OnSuggestionsReceivedListener() {
                    @Override
                    public void onSuggestionsReceived(List<OmniboxSuggestion> suggestions,
                            String inlineAutocompleteText) {
                    }
                };

        final TestAutocompleteController controller = new TestAutocompleteController(
                locationBar, emptySuggestionListener,
                new HashMap<String, List<SuggestionsResult>>());

        OmniboxTestUtils.toggleUrlBarFocus(urlBar, true);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                locationBar.setAutocompleteController(controller);
                urlBar.setText("g");
                urlBar.setSelection(1);
            }
        });

        assertEquals("No calls to zero suggest yet", 0, controller.numZeroSuggestRequests());
        KeyUtils.singleKeyEventView(getInstrumentation(), urlBar, KeyEvent.KEYCODE_DEL);
        assertTrue("Should have requested zero suggest results on url bar empty",
                CriteriaHelper.pollForCriteria(new Criteria() {
                    @Override
                    public boolean isSatisfied() {
                        return controller.numZeroSuggestRequests() == 1;
                    }
                }));
    }

    // Sanity check that no text is displayed in the omnibox when on the NTP page and that the hint
    // text is correct.
    @MediumTest
    @Feature({"Omnibox"})
    public void testDefaultText() throws InterruptedException {
        startMainActivityFromLauncher();

        final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);

        // Omnibox on NTP shows the hint text.
        assertNotNull(urlBar);
        assertEquals("Location bar has text.", "", urlBar.getText().toString());
        assertEquals("Location bar has incorrect hint.",
                getActivity().getResources().getString(R.string.search_or_type_url),
                urlBar.getHint().toString());

        // Type something in the omnibox.
        // Note that the TextView does not provide a way to test if the hint is showing, the API
        // documentation simply says it shows when the text is empty.
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBar.requestFocus();
                urlBar.setText("G");
            }
        });
        assertEquals("Location bar should have text.", "G", urlBar.getText().toString());
    }

    @MediumTest
    @Feature({"Omnibox", "Main"})
    public void testAutoCompleteAndCorrectionLandscape() throws ExecutionException,
            InterruptedException {
        // Default orientation for tablets is landscape. Default for phones is portrait.
        int requestedOrientation = 1;
        if (DeviceFormFactor.isTablet(getActivity())) {
            requestedOrientation = 0;
        }
        doTestAutoCompleteAndCorrectionForOrientation(requestedOrientation);
    }

    @MediumTest
    @Feature({"Omnibox", "Main"})
    public void testAutoCompleteAndCorrectionPortrait() throws ExecutionException,
            InterruptedException {
        // Default orientation for tablets is landscape. Default for phones is portrait.
        int requestedOrientation = 0;
        if (DeviceFormFactor.isTablet(getActivity())) {
            requestedOrientation = 1;
        }
        doTestAutoCompleteAndCorrectionForOrientation(requestedOrientation);
    }

    private void doTestAutoCompleteAndCorrectionForOrientation(
            int orientation) throws ExecutionException, InterruptedException {
        getActivity().setRequestedOrientation(orientation);
        UiUtils.settleDownUI(getInstrumentation());

        Map<String, List<SuggestionsResult>> suggestionsMap = buildSuggestionMap(
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("wiki")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_SUGGEST, "wikipedia", null)
                                .addGeneratedSuggestion(Type.SEARCH_SUGGEST, "wiki", null)
                                .addGeneratedSuggestion(Type.SEARCH_SUGGEST, "wikileaks", null)
                                .setAutocompleteText("pedia")),
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("onomatop")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_SUGGEST, "onomatopoeia", null)
                                .addGeneratedSuggestion(
                                        Type.SEARCH_SUGGEST, "onomatopoeia foo", null)
                                .setAutocompleteText("oeia")),
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("mispellled")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_SUGGEST, "misspelled", null)
                                .addGeneratedSuggestion(
                                        Type.SEARCH_SUGGEST, "misspelled words", null)
                                .setAutocompleteText(""))
        );
        checkAutocompleteText(suggestionsMap, "wiki", "wikipedia", 4, 9);
        checkAutocompleteText(suggestionsMap, "onomatop", "onomatopoeia", 8, 12);
        checkAutocompleteText(suggestionsMap, "mispellled", "mispellled", 10, 10);
    }

    /**
     * crbug.com/267901
     * @MediumTest
     * @Feature({"Omnibox"})
     */
    @FlakyTest
    public void testPerformSearchQuery() throws InterruptedException {
        UiUtils.settleDownUI(getInstrumentation());
        final String testQuery = "Test Query";

        ChromeTabUtils.waitForTabPageLoaded(getActivity().getActivityTab(), new Runnable() {
            @Override
            public void run() {
                ThreadUtils.runOnUiThreadBlocking(new Runnable() {
                    @Override
                    public void run() {
                        final LocationBarLayout locationBar =
                                (LocationBarLayout) getActivity().findViewById(R.id.location_bar);
                        locationBar.performSearchQueryForTest(testQuery);
                    }
                });
            }
        });

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                final UrlBar urlBar = (UrlBar) getActivity().findViewById(R.id.url_bar);
                String currentUrl = urlBar.getText().toString();
                assertEquals(testQuery, currentUrl);
            }
        });
    }

    @MediumTest
    @Feature({"Omnibox"})
    public void testDuplicateAutocompleteTextResults()
            throws InterruptedException, ExecutionException {
        Map<String, List<SuggestionsResult>> suggestionsMap = buildSuggestionMap(
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("test")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testing", null)
                                .setAutocompleteText("ing"))
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testz", null)
                                .setAutocompleteText("ing"))
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testblarg", null)
                                .setAutocompleteText("ing")));
        checkAutocompleteText(suggestionsMap, "test", "testing", 4, 7);
    }

    @MediumTest
    @Feature({"Omnibox"})
    public void testGrowingAutocompleteTextResults()
            throws InterruptedException, ExecutionException {
        Map<String, List<SuggestionsResult>> suggestionsMap = buildSuggestionMap(
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("test")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testing", null)
                                .setAutocompleteText("i"))
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testz", null)
                                .setAutocompleteText("in"))
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testblarg", null)
                                .setAutocompleteText("ing for the win")));
        checkAutocompleteText(suggestionsMap, "test", "testing for the win", 4, 19);
    }

    @MediumTest
    @Feature({"Omnibox"})
    public void testShrinkingAutocompleteTextResults()
            throws InterruptedException, ExecutionException {
        Map<String, List<SuggestionsResult>> suggestionsMap = buildSuggestionMap(
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("test")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testing", null)
                                .setAutocompleteText("ing is awesome"))
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testz", null)
                                .setAutocompleteText("ing is hard"))
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "test", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "testblarg", null)
                                .setAutocompleteText("ingz")));
        checkAutocompleteText(suggestionsMap, "test", "testingz", 4, 8);
    }

    private void checkAutocompleteText(
            Map<String, List<SuggestionsResult>> suggestionsMap,
            final String textToType, final String expectedAutocompleteText,
            final int expectedAutocompleteStart, final int expectedAutocompleteEnd)
            throws InterruptedException, ExecutionException {

        final TextView urlBarView = (TextView) getActivity().findViewById(R.id.url_bar);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBarView.requestFocus();
                urlBarView.setText("");
            }
        });

        final LocationBarLayout locationBar =
                ((LocationBarLayout) getActivity().findViewById(R.id.location_bar));

        final Object suggestionsProcessedSignal = new Object();
        final AtomicInteger suggestionsLeft = new AtomicInteger(
                suggestionsMap.get(textToType).size());
        OnSuggestionsReceivedListener suggestionsListener = new OnSuggestionsReceivedListener() {
            @Override
            public void onSuggestionsReceived(
                    List<OmniboxSuggestion> suggestions,
                    String inlineAutocompleteText) {
                locationBar.onSuggestionsReceived(suggestions, inlineAutocompleteText);
                synchronized (suggestionsProcessedSignal) {
                    int remaining = suggestionsLeft.decrementAndGet();
                    if (remaining == 0) {
                        suggestionsProcessedSignal.notifyAll();
                    } else if (remaining < 0) {
                        fail("Unexpected suggestions received");
                    }
                }
            }
        };
        final TestAutocompleteController controller = new TestAutocompleteController(
                locationBar, suggestionsListener, suggestionsMap);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                locationBar.setAutocompleteController(controller);
            }
        });

        KeyUtils.typeTextIntoView(getInstrumentation(), urlBarView, textToType);

        synchronized (suggestionsProcessedSignal) {
            long endTime = SystemClock.uptimeMillis() + 3000;
            while (suggestionsLeft.get() != 0) {
                long waitTime = endTime - SystemClock.uptimeMillis();
                if (waitTime <= 0) break;
                suggestionsProcessedSignal.wait(waitTime);
            }
        }

        CharSequence urlText = ThreadUtils.runOnUiThreadBlocking(new Callable<CharSequence>() {
            @Override
            public CharSequence call() throws Exception {
                return urlBarView.getText();
            }
        });
        assertEquals("URL Bar text not autocompleted as expected.",
                expectedAutocompleteText, urlText.toString());
        assertEquals(expectedAutocompleteStart, Selection.getSelectionStart(urlText));
        assertEquals(expectedAutocompleteEnd, Selection.getSelectionEnd(urlText));
    }

    /**
     * The following test is a basic way to assess how much instant slows down typing in the
     * omnibox. It is meant to be run manually for investigation purposes.
     * When instant was enabled for all suggestions (including searched), I would get a 40% increase
     * in the average time on this test. With instant off, it was almost identical.
     * Marking the test disabled so it is not picked up by our test runner, as it is supposed to be
     * run manually.
     */
    public void manualTestTypingPerformance() throws InterruptedException {
        final String text = "searching for pizza";
        // Type 10 times something on the omnibox and get the average time with and without instant.
        long instantAverage = 0;
        long noInstantAverage = 0;

        for (int i = 0; i < 2; ++i) {
            boolean instantOn = (i == 1);
            setAllowPrerender(instantOn);

            for (int j = 0; j < 10; ++j) {
                long before = System.currentTimeMillis();
                typeInOmnibox(text, true);
                if (instantOn) {
                    instantAverage += System.currentTimeMillis() - before;
                } else {
                    noInstantAverage += System.currentTimeMillis() - before;
                }
                clearUrlBar();
                getInstrumentation().waitForIdleSync();
            }
        }
        instantAverage /= 10;
        noInstantAverage /= 10;
        System.err.println("******************************************************************");
        System.err.println("**** Instant average=" + instantAverage);
        System.err.println("**** No instant average=" + noInstantAverage);
        System.err.println("******************************************************************");
    }

    /**
     * crbug.com/414353
     * Test to verify security-icon "lock or globe" on visiting http and secured Urls.
     * @EnormousTest
     */
    @FlakyTest
    public void testSecurityIcon() throws InterruptedException {
        final String testUrl = TestHttpServerClient.getUrl(
                "chrome/test/data/android/omnibox/one.html");
        final String securedExternalUrl = "https://www.google.com";

        ImageView navigationButton = (ImageView)
                getActivity().findViewById(R.id.navigation_button);
        ImageButton securityButton = (ImageButton)
                getActivity().findViewById(R.id.security_button);

        loadUrl(testUrl);
        final LocationBarLayout locationBar =
                (LocationBarLayout) getActivity().findViewById(R.id.location_bar);
        boolean securityIcon = locationBar.isSecurityButtonShown();
        assertFalse("Omnibox should not have a Security icon", securityIcon);
        assertEquals("navigation_button with wrong resource-id",
                R.id.navigation_button, navigationButton.getId());
        assertTrue(navigationButton.isShown());
        assertFalse(securityButton.isShown());

        loadUrl(securedExternalUrl);
        securityIcon = locationBar.isSecurityButtonShown();
        assertTrue("Omnibox should have a Security icon", securityIcon);
        assertEquals("security_button with wrong resource-id",
                R.id.security_button, securityButton.getId());
        assertTrue(securityButton.isShown());
        assertFalse(navigationButton.isShown());
    }

    @SmallTest
    public void testSplitPathFromUrlDisplayText() {
        verifySplitUrlAndPath("", null, LocationBarLayout.splitPathFromUrlDisplayText(""));
        verifySplitUrlAndPath(
                "https:", null, LocationBarLayout.splitPathFromUrlDisplayText("https:"));
        verifySplitUrlAndPath("about:blank", null,
                LocationBarLayout.splitPathFromUrlDisplayText("about:blank"));

        verifySplitUrlAndPath("chrome://flags", null,
                LocationBarLayout.splitPathFromUrlDisplayText("chrome://flags"));
        verifySplitUrlAndPath("chrome://flags", "/?egads",
                LocationBarLayout.splitPathFromUrlDisplayText("chrome://flags/?egads"));

        verifySplitUrlAndPath("www.google.com", null,
                LocationBarLayout.splitPathFromUrlDisplayText("www.google.com"));
        verifySplitUrlAndPath("www.google.com", null,
                LocationBarLayout.splitPathFromUrlDisplayText("www.google.com/"));
        verifySplitUrlAndPath("www.google.com", "/?q=blah",
                LocationBarLayout.splitPathFromUrlDisplayText("www.google.com/?q=blah"));

        verifySplitUrlAndPath("https://www.google.com", null,
                LocationBarLayout.splitPathFromUrlDisplayText("https://www.google.com"));
        verifySplitUrlAndPath("https://www.google.com", null,
                LocationBarLayout.splitPathFromUrlDisplayText("https://www.google.com/"));
        verifySplitUrlAndPath("https://www.google.com", "/?q=blah",
                LocationBarLayout.splitPathFromUrlDisplayText("https://www.google.com/?q=blah"));

        // crbug.com/414990
        String testUrl = "https://disneyworld.disney.go.com/special-offers/"
                + "?CMP=KNC-WDW_FY15_DOM_Q1RO_BR_Gold_SpOffer|G|4141300.RR.AM.01.47"
                + "&keyword_id=s6JyxRifG_dm|walt%20disney%20world|37174067873|e|1540wwa14043";
        verifySplitUrlAndPath("https://disneyworld.disney.go.com",
                "/special-offers/?CMP=KNC-WDW_FY15_DOM_Q1RO_BR_Gold_SpOffer|G|4141300.RR.AM.01.47"
                        + "&keyword_id=s6JyxRifG_dm|walt%20disney%20world|37174067873|e|"
                        + "1540wwa14043",
                LocationBarLayout.splitPathFromUrlDisplayText(testUrl));

        // crbug.com/415387
        verifySplitUrlAndPath("ftp://example.com", "/ftp.html",
                LocationBarLayout.splitPathFromUrlDisplayText("ftp://example.com/ftp.html"));

        // crbug.com/447416
        verifySplitUrlAndPath("file:///dev/blah", null,
                LocationBarLayout.splitPathFromUrlDisplayText("file:///dev/blah"));
        verifySplitUrlAndPath("javascript:window.alert('hello');", null,
                LocationBarLayout.splitPathFromUrlDisplayText("javascript:window.alert('hello');"));
        verifySplitUrlAndPath("data:text/html;charset=utf-8,Page%201", null,
                LocationBarLayout.splitPathFromUrlDisplayText(
                        "data:text/html;charset=utf-8,Page%201"));
    }

    private void verifySplitUrlAndPath(
            String expectedPrePath, String expectedPostPath, Pair<String, String> actualValues) {
        assertEquals(expectedPrePath, actualValues.first);
        assertEquals(expectedPostPath, actualValues.second);
    }

    @MediumTest
    @Feature({"Omnibox"})
    public void testSuggestionDirectionSwitching() throws InterruptedException {
        final TextView urlBarView = (TextView) getActivity().findViewById(R.id.url_bar);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBarView.requestFocus();
                urlBarView.setText("");
            }
        });

        final LocationBarLayout locationBar =
                ((LocationBarLayout) getActivity().findViewById(R.id.location_bar));

        Map<String, List<SuggestionsResult>> suggestionsMap = buildSuggestionMap(
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("ل")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "للك", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "www.test.com", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "للكتا", null)),
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("للك")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "للكتاب", null)),
                new TestSuggestionResultsBuilder()
                        .setTextShownFor("f")
                        .addSuggestions(new SuggestionsResultBuilder()
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "f", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "fa", null)
                                .addGeneratedSuggestion(Type.SEARCH_HISTORY, "fac", null)));
        final TestAutocompleteController controller = new TestAutocompleteController(
                locationBar, locationBar, suggestionsMap);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                locationBar.setAutocompleteController(controller);
            }
        });

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBarView.setText("ل");
            }
        });
        verifyOmniboxSuggestionAlignment(locationBar, 3, View.LAYOUT_DIRECTION_RTL);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBarView.setText("للك");
            }
        });
        verifyOmniboxSuggestionAlignment(locationBar, 1, View.LAYOUT_DIRECTION_RTL);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                urlBarView.setText("f");
            }
        });
        verifyOmniboxSuggestionAlignment(locationBar, 3, View.LAYOUT_DIRECTION_LTR);
    }

    private void verifyOmniboxSuggestionAlignment(
            final LocationBarLayout locationBar, final int expectedSuggestionCount,
            final int expectedLayoutDirection) throws InterruptedException {
        assertTrue(OmniboxTestUtils.waitForOmniboxSuggestions(
                locationBar, expectedSuggestionCount));
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                OmniboxSuggestionsList suggestionsList = locationBar.getSuggestionList();
                assertEquals(expectedSuggestionCount, suggestionsList.getChildCount());
                for (int i = 0; i < suggestionsList.getChildCount(); i++) {
                    SuggestionView suggestionView = (SuggestionView) suggestionsList.getChildAt(i);
                    assertEquals(
                            String.format(
                                    Locale.getDefault(),
                                    "Incorrect layout direction of suggestion at index %d",
                                    i),
                            expectedLayoutDirection,
                            ViewCompat.getLayoutDirection(suggestionView));
                }
            }
        });
    }

    @Override
    public void startMainActivity() throws InterruptedException {
        if (getName().equals("testsplitPathFromUrlDisplayText")
                || getName().equals("testDefaultText")) {
            return;
        }
        startMainActivityOnBlankPage();
    }
}
