// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "anchor.h"

#include "config.h"
#include "drawing-engine.h"
#include "util.h"
#include "window.h"
#include "x-server.h"
#include "x-window.h"

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

  void testSetName() {
    Anchor anchor("test", 10, 20);
    TS_ASSERT_EQUALS(anchor.name(), "test");
    anchor.SetName("foo");
    TS_ASSERT_EQUALS(anchor.name(), "foo");
  }

  void testAddWindow_RemoveWindow() {
    Anchor anchor("test", 10, 20);
    TS_ASSERT(anchor.windows().empty());
    TS_ASSERT_EQUALS(anchor.active_window(), static_cast<wham::Window*>(NULL));

    wham::Window window(XWindow::Create(50, 60, 640, 480));
    anchor.AddWindow(&window);
    TS_ASSERT_EQUALS(anchor.windows().size(), 1U);
    TS_ASSERT_EQUALS(anchor.active_window(), &window);

    wham::Window window2(XWindow::Create(50, 60, 640, 480));
    anchor.AddWindow(&window2);
    TS_ASSERT_EQUALS(anchor.windows().size(), 2U);
    // The first window should still be active.
    TS_ASSERT_EQUALS(anchor.active_window(), &window);

    anchor.RemoveWindow(&window);
    TS_ASSERT_EQUALS(anchor.windows().size(), 1U);
    // The second window should be active now.
    TS_ASSERT_EQUALS(anchor.active_window(), &window2);

    anchor.RemoveWindow(&window2);
    TS_ASSERT(anchor.windows().empty());
    TS_ASSERT_EQUALS(anchor.active_window(), static_cast<wham::Window*>(NULL));
  }

  void testMove() {
    Anchor anchor("test", 10, 20);
    wham::Window window(XWindow::Create(50, 60, 640, 480));
    anchor.AddWindow(&window);

    int x = 100, y = 200;
    anchor.Move(x, y);
    TS_ASSERT_EQUALS(anchor.x(), x);
    TS_ASSERT_EQUALS(anchor.y(), y);
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);
    TS_ASSERT_EQUALS(window.x(), x);
    TS_ASSERT_EQUALS(window.y(),
                     y + static_cast<int>(anchor.titlebar_->height()));
  }

  void testSetActive() {
    // FIXME: write this
  }

  void testSetGravity() {
    int x = 10, y = 20;
    Anchor anchor("test", x, y);
    anchor.titlebar_->Resize(100, 15);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_LEFT);
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);

    anchor.SetGravity(Anchor::TOP_RIGHT);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_RIGHT);
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);
    // TODO: Maybe check that the active window is also getting moved.

    anchor.SetGravity(Anchor::BOTTOM_RIGHT);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::BOTTOM_RIGHT);
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);

    anchor.SetGravity(Anchor::BOTTOM_LEFT);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::BOTTOM_LEFT);
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);

    anchor.SetGravity(Anchor::TOP_LEFT);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_LEFT);
    TS_ASSERT_EQUALS(anchor.titlebar_->x(), x);
    TS_ASSERT_EQUALS(anchor.titlebar_->y(), y);
  }

  void testCycleGravity() {
    Anchor anchor("test", 10, 20);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_LEFT);

    anchor.CycleGravity(true);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_RIGHT);

    anchor.CycleGravity(true);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::BOTTOM_RIGHT);

    anchor.CycleGravity(true);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::BOTTOM_LEFT);

    anchor.CycleGravity(true);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_LEFT);

    anchor.CycleGravity(false);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::BOTTOM_LEFT);

    anchor.CycleGravity(false);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::BOTTOM_RIGHT);

    anchor.CycleGravity(false);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_RIGHT);

    anchor.CycleGravity(false);
    TS_ASSERT_EQUALS(anchor.gravity_, Anchor::TOP_LEFT);
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

    wham::Window window(XWindow::Create(50, 60, 640, 480));
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
