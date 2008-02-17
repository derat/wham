// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "desktop.h"

#include "anchor.h"
#include "drawing-engine.h"
#include "mock-x-window.h"
#include "util.h"
#include "window.h"
#include "x-server.h"
#include "x-window.h"

using namespace wham;

class DesktopTestSuite : public CxxTest::TestSuite {
 public:
  void setUp() {
    XServer::SetupTesting();
    desktop_.reset(new Desktop);
  }

  void testHide_Show() {
    Anchor* anchor1 = desktop_->CreateAnchor("test1", 10, 20);
    Anchor* anchor2 = desktop_->CreateAnchor("test2", 50, 60);

    // We'll just check that the anchors' titlebars are hidden or shown;
    // anchor_test has tests to make sure that we hide and show anchors'
    // active windows correctly.
    MockXWindow* xwin1 = dynamic_cast<MockXWindow*>(anchor1->titlebar());
    MockXWindow* xwin2 = dynamic_cast<MockXWindow*>(anchor2->titlebar());
    CHECK(xwin1);
    CHECK(xwin2);

    // Initially, both anchors should be mapped.
    TS_ASSERT(xwin1->mapped());
    TS_ASSERT(xwin2->mapped());

    // After hiding the desktop, both should be unmapped.
    desktop_->Hide();
    TS_ASSERT(!xwin1->mapped());
    TS_ASSERT(!xwin2->mapped());

    // And after showing the desktop, both anchors should be mapped again.
    desktop_->Show();
    TS_ASSERT(xwin1->mapped());
    TS_ASSERT(xwin2->mapped());
  }

  void testCreateAnchor() {
    TS_ASSERT_EQUALS(desktop_->anchors_.size(), 0U);
    TS_ASSERT_EQUALS(desktop_->active_anchor(), static_cast<Anchor*>(NULL));

    Anchor* anchor = desktop_->CreateAnchor("test", 10, 20);
    TS_ASSERT_EQUALS(anchor->name(), "test");
    TS_ASSERT_EQUALS(anchor->x(), 10);
    TS_ASSERT_EQUALS(anchor->y(), 20);
    CHECK(!desktop_->anchors_.empty());
    TS_ASSERT_EQUALS(desktop_->anchors_.size(), 1U);
    TS_ASSERT_EQUALS(desktop_->anchors_[0].get(), anchor);
    TS_ASSERT_EQUALS(desktop_->active_anchor(), anchor);

    Anchor* anchor2 = desktop_->CreateAnchor("test2", 30, 40);
    TS_ASSERT_EQUALS(anchor2->name(), "test2");
    TS_ASSERT_EQUALS(anchor2->x(), 30);
    TS_ASSERT_EQUALS(anchor2->y(), 40);
    CHECK(!desktop_->anchors_.empty());
    TS_ASSERT_EQUALS(desktop_->anchors_.size(), 2U);
    TS_ASSERT_EQUALS(desktop_->anchors_[0].get(), anchor);
    TS_ASSERT_EQUALS(desktop_->anchors_[1].get(), anchor2);
    TS_ASSERT_EQUALS(desktop_->active_anchor(), anchor);
  }

  void testAddWindow() {
    // FIXME
    /*
    wham::Window window(XWindow::Create(10, 10, 50, 50));;
    desktop_->AddWindow(&window);
    */
  }

  void testGetAnchorByTitlebar() {
    Anchor* anchor = desktop_->CreateAnchor("test", 10, 20);
    TS_ASSERT_EQUALS(desktop_->GetAnchorByTitlebar(anchor->titlebar()), anchor);

    // We should get NULL when we look up a window that's not a titlebar.
    XWindow* x_window = XWindow::Create(0, 0, 10, 10);
    TS_ASSERT_EQUALS(desktop_->GetAnchorByTitlebar(x_window),
                     static_cast<Anchor*>(NULL));
  }

  void testSetActiveAnchor() {
    Anchor* anchor = desktop_->CreateAnchor("test", 10, 20);
    Anchor* anchor2 = desktop_->CreateAnchor("test2", 30, 40);
    TS_ASSERT_EQUALS(desktop_->active_anchor(), anchor);
    desktop_->SetActiveAnchor(anchor2);
    TS_ASSERT_EQUALS(desktop_->active_anchor(), anchor2);
  }

  void testIsTitlebarWindow() {
    Anchor* anchor = desktop_->CreateAnchor("test", 10, 20);
    TS_ASSERT_EQUALS(desktop_->IsTitlebarWindow(anchor->titlebar()), true);

    XWindow* x_window = XWindow::Create(0, 0, 10, 10);
    TS_ASSERT_EQUALS(desktop_->IsTitlebarWindow(x_window), false);
  }

  void testGetNearestAnchor() {
    Anchor* anchor = desktop_->CreateAnchor("test", 10, 20);
    Anchor* anchor2 = desktop_->CreateAnchor("test2", 20, 20);
    Anchor* anchor3 = desktop_->CreateAnchor("test3", 30, 20);
    TS_ASSERT_EQUALS(desktop_->active_anchor(), anchor);
    TS_ASSERT_EQUALS(desktop_->GetNearestAnchor(Command::RIGHT), anchor2);

    desktop_->SetActiveAnchor(anchor2);
    TS_ASSERT_EQUALS(desktop_->GetNearestAnchor(Command::RIGHT), anchor3);
    TS_ASSERT_EQUALS(desktop_->GetNearestAnchor(Command::LEFT), anchor);
  }

  ref_ptr<Desktop> desktop_;
};
