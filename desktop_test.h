// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "desktop.h"

#include "anchor.h"
#include "util.h"
#include "x.h"

using namespace wham;

class DesktopTestSuite : public CxxTest::TestSuite {
 public:
  void setUp() {
    x_server_.reset(new XServer);
    x_server_->Init();
    XWindow::SetTesting(true);
    desktop_.reset(new Desktop);
  }

  void testDesktop_CreateAnchor() {
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

  void testDesktop_AddWindow() {
  }

  ref_ptr<XServer> x_server_;
  ref_ptr<Desktop> desktop_;
};
