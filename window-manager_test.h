// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "window-manager.h"

#include "anchor.h"
#include "desktop.h"
#include "util.h"
#include "window.h"
#include "x-server.h"

using namespace wham;

class WindowManagerTestSuite : public CxxTest::TestSuite {
 public:
  void setUp() {
    XServer::SetupTesting();
  }

  void testCreateDesktop() {
    WindowManager wm;

    // We should start out with no desktops.
    TS_ASSERT_EQUALS(wm.desktops_.empty(), true);
    TS_ASSERT_EQUALS(wm.active_desktop_, static_cast<Desktop*>(NULL));

    // After inserting a desktop, we shouldn't automatically switch to it.
    Desktop* desktop1 = wm.CreateDesktop();
    TS_ASSERT_EQUALS(wm.desktops_.size(), 1U);
    TS_ASSERT_EQUALS(wm.active_desktop_, static_cast<Desktop*>(NULL));

    // Add a second desktop, which should be added to the end of the list.
    Desktop* desktop2 = wm.CreateDesktop();
    TS_ASSERT_EQUALS(wm.desktops_.size(), 2U);
    TS_ASSERT_EQUALS(wm.desktops_[0].get(), desktop1);
    TS_ASSERT_EQUALS(wm.desktops_[1].get(), desktop2);
    TS_ASSERT_EQUALS(wm.active_desktop_, static_cast<Desktop*>(NULL));

    // Add a third desktop.
    Desktop* desktop3 = wm.CreateDesktop();
    TS_ASSERT_EQUALS(wm.desktops_.size(), 3U);
    TS_ASSERT_EQUALS(wm.desktops_[0].get(), desktop1);
    TS_ASSERT_EQUALS(wm.desktops_[1].get(), desktop2);
    TS_ASSERT_EQUALS(wm.desktops_[2].get(), desktop3);
    TS_ASSERT_EQUALS(wm.active_desktop_, static_cast<Desktop*>(NULL));

    // Now switch to the second desktop.
    wm.SetActiveDesktop(desktop2);
    TS_ASSERT_EQUALS(wm.active_desktop_, desktop2);

    // If we create a desktop now, it should be appended after the active
    // (second) desktop.  We shouldn't switch to it automatically.
    Desktop* desktop4 = wm.CreateDesktop();
    TS_ASSERT_EQUALS(wm.desktops_.size(), 4U);
    TS_ASSERT_EQUALS(wm.desktops_[0].get(), desktop1);
    TS_ASSERT_EQUALS(wm.desktops_[1].get(), desktop2);
    TS_ASSERT_EQUALS(wm.desktops_[2].get(), desktop4);
    TS_ASSERT_EQUALS(wm.desktops_[3].get(), desktop3);
    TS_ASSERT_EQUALS(wm.active_desktop_, desktop2);
  }

  void testGetDesktopIndex() {
    WindowManager wm;
    TS_ASSERT_EQUALS(wm.GetDesktopIndex(NULL), -1);

    Desktop* desktop = wm.CreateDesktop();
    TS_ASSERT_EQUALS(wm.GetDesktopIndex(NULL), -1);
    TS_ASSERT_EQUALS(wm.GetDesktopIndex(desktop), 0);

    Desktop* desktop2 = wm.CreateDesktop();
    TS_ASSERT_EQUALS(wm.GetDesktopIndex(desktop), 0);
    TS_ASSERT_EQUALS(wm.GetDesktopIndex(desktop2), 1);
  }

  void testSetActiveDesktop() {
  }

  void testAttachTaggedWindowsSingleDesktop() {
    // Create two anchors on a single desktop, and add one window to each
    // of them.
    WindowManager wm;
    Desktop* desktop = wm.CreateDesktop();
    wm.SetActiveDesktop(desktop);
    Anchor* anchor1 = wm.active_desktop_->CreateAnchor("anchor1", 0, 0);
    Anchor* anchor2 = wm.active_desktop_->CreateAnchor("anchor2", 0, 0);

    XWindow* xwin1 = XWindow::Create(0, 0, 100, 100);
    wham::Window win1(xwin1);
    desktop->AddWindow(&win1);
    wm.ToggleWindowTag(&win1);

    XWindow* xwin2 = XWindow::Create(0, 0, 100, 100);
    wham::Window win2(xwin2);
    desktop->AddWindow(&win2);
    wm.ToggleWindowTag(&win2);

    TS_ASSERT_EQUALS(anchor1->windows().size(), 2U);
    TS_ASSERT(anchor2->windows().empty());
    TS_ASSERT_EQUALS(wm.tagged_windows_.size(), 2U);

    // Attach both windows to the second anchor.  The first window should
    // be removed from the first anchor when we do this.
    wm.AttachTaggedWindows(anchor2);
    TS_ASSERT(anchor1->windows().empty());
    TS_ASSERT_EQUALS(anchor2->windows().size(), 2U);
    TS_ASSERT(wm.tagged_windows_.empty());
  }

  void testAttachTaggedWindowsMultipleDesktops() {
    // Create two desktops, with two anchors on the first and one on the
    // second.
    WindowManager wm;
    Desktop* desktop1 = wm.CreateDesktop();
    Desktop* desktop2 = wm.CreateDesktop();
    wm.SetActiveDesktop(desktop1);
    Anchor* anchor1 = desktop1->CreateAnchor("anchor1", 0, 0);
    Anchor* anchor2 = desktop1->CreateAnchor("anchor2", 0, 0);
    Anchor* anchor3 = desktop2->CreateAnchor("anchor2", 0, 0);

    // Add win1 to anchor2 on desktop1.
    XWindow* xwin1 = XWindow::Create(0, 0, 100, 100);
    wham::Window win1(xwin1);
    desktop1->AddWindowToAnchor(&win1, anchor2);
    wm.ToggleWindowTag(&win1);

    // Add win2 to anchor3 on desktop2.
    XWindow* xwin2 = XWindow::Create(0, 0, 100, 100);
    wham::Window win2(xwin2);
    desktop2->AddWindow(&win2);
    wm.ToggleWindowTag(&win2);

    TS_ASSERT(anchor1->windows().empty());
    TS_ASSERT_EQUALS(anchor2->windows().size(), 1U);
    TS_ASSERT_EQUALS(anchor3->windows().size(), 1U);
    TS_ASSERT_EQUALS(wm.tagged_windows_.size(), 2U);

    // Attach both windows to anchor1.  win1 should be moved from anchor2
    // to anchor1, but win2 should still be present on desktop2 as well.
    wm.AttachTaggedWindows(anchor1);
    TS_ASSERT_EQUALS(anchor1->windows().size(), 2U);
    TS_ASSERT(anchor2->windows().empty());
    TS_ASSERT_EQUALS(anchor3->windows().size(), 1U);
    TS_ASSERT(wm.tagged_windows_.empty());
  }

  void testToggleWindowTag() {
    // We should start out with no tagged windows.
    WindowManager wm;
    Desktop* desktop = wm.CreateDesktop();
    wm.SetActiveDesktop(desktop);
    wm.active_desktop_->CreateAnchor("anchor1", 0, 0);
    TS_ASSERT_EQUALS(wm.tagged_windows_.empty(), true);

    // Create and tag a single window.
    XWindow* xwin1 = XWindow::Create(0, 0, 100, 100);
    wham::Window win1(xwin1);
    desktop->AddWindow(&win1);
    TS_ASSERT_EQUALS(win1.tagged(), false);
    wm.ToggleWindowTag(&win1);
    TS_ASSERT_EQUALS(wm.tagged_windows_.size(), 1U);
    TS_ASSERT(wm.tagged_windows_.find(&win1) != wm.tagged_windows_.end());
    TS_ASSERT_EQUALS(win1.tagged(), true);

    // Now create and tag a second one.
    XWindow* xwin2 = XWindow::Create(0, 0, 100, 100);
    wham::Window win2(xwin2);
    desktop->AddWindow(&win2);
    TS_ASSERT_EQUALS(win2.tagged(), false);
    wm.ToggleWindowTag(&win2);
    TS_ASSERT_EQUALS(wm.tagged_windows_.size(), 2U);
    TS_ASSERT(wm.tagged_windows_.find(&win1) != wm.tagged_windows_.end());
    TS_ASSERT(wm.tagged_windows_.find(&win2) != wm.tagged_windows_.end());
    TS_ASSERT_EQUALS(win1.tagged(), true);
    TS_ASSERT_EQUALS(win2.tagged(), true);

    // After toggling the tag on the first window, only the second one
    // should be tagged.
    wm.ToggleWindowTag(&win1);
    TS_ASSERT_EQUALS(wm.tagged_windows_.size(), 1U);
    TS_ASSERT(wm.tagged_windows_.find(&win2) != wm.tagged_windows_.end());
    TS_ASSERT_EQUALS(win1.tagged(), false);
    TS_ASSERT_EQUALS(win2.tagged(), true);

    // After toggling the tag on the second window, no windows should be
    // tagged.
    wm.ToggleWindowTag(&win2);
    TS_ASSERT_EQUALS(wm.tagged_windows_.empty(), true);
    TS_ASSERT_EQUALS(win2.tagged(), false);
  }

  void testGetActiveWindow() {
    WindowManager wm;
    wm.SetActiveDesktop(wm.CreateDesktop());
    TS_ASSERT_EQUALS(wm.GetActiveWindow(), static_cast<wham::Window*>(NULL));

    wm.active_desktop_->CreateAnchor("test", 0, 0);
    TS_ASSERT_EQUALS(wm.GetActiveWindow(), static_cast<wham::Window*>(NULL));

    XWindow* xwin = XWindow::Create(0, 0, 100, 100);
    wham::Window window(xwin);
    Anchor* anchor = wm.active_desktop_->active_anchor();
    anchor->AddWindow(&window);
    TS_ASSERT_EQUALS(wm.GetActiveWindow(), &window);
  }
};
