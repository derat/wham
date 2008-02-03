// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "anchor.h"

#include "config.h"
#include "drawing-engine.h"
#include "util.h"
#include "window.h"
#include "x.h"

using namespace wham;

class AnchorTestSuite : public CxxTest::TestSuite {
 public:
  void setUp() {
    XServer::SetupTesting();
  }

  void testConstructor() {
    Anchor anchor("test", 10, 20);
    TS_ASSERT_EQUALS(anchor.name(), "test");
    TS_ASSERT_EQUALS(anchor.x(), 10);
    TS_ASSERT_EQUALS(anchor.y(), 20);
    TS_ASSERT_EQUALS(anchor.gravity(), Anchor::TOP_LEFT);
  }

  void testUpdateTitlebarPosition() {
    int x = 10, y = 20;
    Anchor anchor("test", x, y);
    anchor.titlebar_->Resize(100, 15);

    anchor.gravity_ = Anchor::TOP_LEFT;
    anchor.UpdateTitlebarPosition();
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);

    anchor.gravity_ = Anchor::BOTTOM_LEFT;
    anchor.UpdateTitlebarPosition();
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(),
                     y - static_cast<int>(anchor.titlebar_->height()));

    anchor.gravity_ = Anchor::TOP_RIGHT;
    anchor.UpdateTitlebarPosition();
    TS_ASSERT_EQUALS(anchor.titlebar_->x(),
                     x - static_cast<int>(anchor.titlebar_->width()));
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);

    anchor.gravity_ = Anchor::BOTTOM_RIGHT;
    anchor.UpdateTitlebarPosition();
    TS_ASSERT_EQUALS(anchor.titlebar_->x(),
                     x - static_cast<int>(anchor.titlebar_->width()));
    TS_ASSERT_EQUALS(anchor.titlebar_->y(),
                     y - static_cast<int>(anchor.titlebar_->height()));
  }

  void testUpdateWindowPosition() {
    int x = 10, y = 20;
    Anchor anchor("test", x, y);
    anchor.titlebar_->Resize(100, 15);

    XWindow* x_window = XWindow::Create(50, 60, 640, 480);
    wham::Window window(x_window);
    anchor.AddWindow(&window);

    int border = static_cast<int>(Config::Get()->window_border);

    anchor.gravity_ = Anchor::TOP_LEFT;
    anchor.UpdateWindowPosition(&window);
    TS_ASSERT_EQUALS(window.x(), x);
    TS_ASSERT_EQUALS(window.y(),
                     y + static_cast<int>(anchor.titlebar_->height()));

    anchor.gravity_ = Anchor::BOTTOM_LEFT;
    anchor.UpdateWindowPosition(&window);
    TS_ASSERT_EQUALS(window.x(), x);
    TS_ASSERT_EQUALS(window.y(),
                     y - static_cast<int>(anchor.titlebar_->height()) -
                       static_cast<int>(window.height()) - 2 * border);

    anchor.gravity_ = Anchor::TOP_RIGHT;
    anchor.UpdateWindowPosition(&window);
    TS_ASSERT_EQUALS(window.x(),
                     x - static_cast<int>(window.width()) - 2 * border);
    TS_ASSERT_EQUALS(window.y(),
                     y + static_cast<int>(anchor.titlebar_->height()));

    anchor.gravity_ = Anchor::BOTTOM_RIGHT;
    anchor.UpdateWindowPosition(&window);
    TS_ASSERT_EQUALS(window.x(),
                     x - static_cast<int>(window.width()) - 2 * border);
    TS_ASSERT_EQUALS(window.y(),
                     y - static_cast<int>(anchor.titlebar_->height()) -
                       static_cast<int>(window.height()) - 2 * border);
  }

  ref_ptr<XServer> x_server_;
};
