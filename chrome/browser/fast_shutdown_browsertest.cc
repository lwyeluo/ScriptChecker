// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/browser_test_utils.h"
#include "net/test/embedded_test_server/embedded_test_server.h"

using content::BrowserThread;

class FastShutdown : public InProcessBrowserTest {
 protected:
  FastShutdown() {
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    command_line->AppendSwitch(switches::kDisablePopupBlocking);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(FastShutdown);
};

// This tests for a previous error where uninstalling an onbeforeunload handler
// would enable fast shutdown even if an onunload handler still existed.
// Flaky on all platforms, http://crbug.com/89173
#if !defined(OS_CHROMEOS)  // ChromeOS opens tabs instead of windows for popups.
IN_PROC_BROWSER_TEST_F(FastShutdown, DISABLED_SlowTermination) {
  // Need to run these tests on http:// since we only allow cookies on that (and
  // https obviously).
  ASSERT_TRUE(embedded_test_server()->Start());
  // This page has an unload handler.
  GURL url = embedded_test_server()->GetURL("/fast_shutdown/on_unloader.html");
  EXPECT_EQ("", content::GetCookies(browser()->profile(), url));

  content::WindowedNotificationObserver window_observer(
      chrome::NOTIFICATION_BROWSER_OPENED,
      content::NotificationService::AllSources());
  ui_test_utils::NavigateToURLWithDisposition(
      browser(), url, WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui_test_utils::BROWSER_TEST_NONE);
  window_observer.Wait();

  // Close the new window, removing the one and only beforeunload handler.
  ASSERT_EQ(2u, chrome::GetTotalBrowserCount());
  chrome::CloseWindow(*(BrowserList::GetInstance()->begin() + 1));

  // Need to wait for the renderer process to shutdown to ensure that we got the
  // set cookies IPC.
  content::RenderProcessHostWatcher renderer_shutdown_observer(
      browser()->tab_strip_model()->GetActiveWebContents(),
      content::RenderProcessHostWatcher::WATCH_FOR_HOST_DESTRUCTION);
  // Close the tab. This should launch the unload handler, which sets a cookie
  // that's stored to disk.
  chrome::CloseTab(browser());
  renderer_shutdown_observer.Wait();

  EXPECT_EQ("unloaded=ohyeah", content::GetCookies(browser()->profile(), url));
}
#endif
