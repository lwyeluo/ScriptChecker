// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import static junit.framework.Assert.assertTrue;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.core.IsCollectionContaining.hasItems;
import static org.junit.Assert.assertFalse;

import android.support.test.filters.SmallTest;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestRule;
import org.junit.runner.RunWith;

import org.chromium.base.CommandLine;
import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.chrome.browser.util.FeatureUtilities;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.chrome.test.util.browser.ChromeHome;
import org.chromium.chrome.test.util.browser.Features;
import org.chromium.chrome.test.util.browser.Features.DisableFeatures;
import org.chromium.chrome.test.util.browser.Features.EnableFeatures;
import org.chromium.ui.base.DeviceFormFactor;

import java.util.Arrays;
import java.util.List;

/**
 * Tests for {@link Features}.
 */
@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add(ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE)
public class FeaturesAnnotationsTest {
    @Rule
    public TestRule mChromeHomeProcessor = new ChromeHome.Processor();

    @Rule
    public TestRule mFeaturesProcessor = new Features.InstrumentationProcessor();

    @Rule
    public ChromeTabbedActivityTestRule mActivityRule = new ChromeTabbedActivityTestRule();

    /**
     * Tests that {@link EnableFeatures} and {@link DisableFeatures} can alter the flags registered
     * on {@link CommandLine}.
     */
    @Test
    @SmallTest
    @EnableFeatures("One")
    @DisableFeatures("Two")
    public void testFeaturesSetExistingFlags() throws InterruptedException {
        mActivityRule.startMainActivityOnBlankPage();
        List<String> finalEnabledList = getArgsList(true);

        assertThat(finalEnabledList, hasItems("One"));
        assertThat(finalEnabledList.size(), equalTo(1));

        List<String> finalDisabledList = getArgsList(false);
        assertThat(finalDisabledList, hasItems("Two"));
        assertThat(finalDisabledList.size(), equalTo(1));
    }

    /**
     * Tests the compatibility between {@link EnableFeatures} and other rules. {@link ChromeHome}
     * here explicitly calls {@link Features#enable(String...)}, so its feature should also be added
     * to the set of registered flags.
     */
    @DisabledTest(message = "https://crbug.com/805160")
    @Test
    @SmallTest
    @ChromeHome.Enable
    @EnableFeatures("One")
    public void testFeaturesIncludeValuesSetFromOtherRules() throws InterruptedException {
        mActivityRule.startMainActivityOnBlankPage();

        List<String> finalEnabledList = getArgsList(true);
        assertThat(finalEnabledList, hasItems("One", ChromeFeatureList.CHROME_HOME));
        assertTrue(ChromeFeatureList.isEnabled(ChromeFeatureList.CHROME_HOME));
    }

    /**
     * Tests the compatibility between the legacy {@link CommandLineFlags} annotation usage for
     * features and the new dedicated annotations.
     *
     * If a feature is already present in the command line, it's should not be removed nor alter
     * the current feature list.
     */
    @Test
    @SmallTest
    @CommandLineFlags.Add("enable-features=One,Two,Three")
    @EnableFeatures("Two")
    public void testFeaturesDoNotRemoveExistingFlags() throws InterruptedException {
        mActivityRule.startMainActivityOnBlankPage();
        List<String> finalEnabledList = getArgsList(true);

        assertThat(finalEnabledList, hasItems("One", "Two", "Three"));
        assertThat(finalEnabledList.size(), equalTo(3));
    }

    /**
     * Tests the compatibility between the legacy {@link CommandLineFlags} annotation usage for
     * features and the new dedicated annotations.
     *
     * New features should be added to the existing command line, without removing the current ones.
     */
    @Test
    @SmallTest
    @CommandLineFlags.Add("enable-features=One,Two,Three")
    @EnableFeatures({"Three", "Four"})
    public void testFeaturesAddToExistingFlags() throws InterruptedException {
        mActivityRule.startMainActivityOnBlankPage();
        List<String> finalEnabledList = getArgsList(true);

        assertThat(finalEnabledList, hasItems("Four"));
        assertThat(finalEnabledList.size(), equalTo(4));
    }

    @DisabledTest(message = "https://crbug.com/805160")
    @Test
    @SmallTest
    @ChromeHome.Enable
    public void testChromeHomeSkipping() {
        assertFalse("The test should only run on phones.", DeviceFormFactor.isTablet());
        assertTrue("ChromeHome should be enabled.", FeatureUtilities.isChromeHomeEnabled());
    }

    private static List<String> getArgsList(boolean enabled) {
        String switchName = enabled ? "enable-features" : "disable-features";
        return Arrays.asList(CommandLine.getInstance().getSwitchValue(switchName).split(","));
    }
}
