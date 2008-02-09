// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "window.h"

#include "mock-x.h"
#include "x.h"

using namespace wham;

class WindowTestSuite : public CxxTest::TestSuite {
 public:
  void setUp() {
    XServer::SetupTesting();
  }

  void testMove() {
    XWindow* xwin = XWindow::Create(50, 60, 640, 480);
    wham::Window win(xwin);
    TS_ASSERT_EQUALS(xwin->x(), 50);
    TS_ASSERT_EQUALS(xwin->y(), 60);

    win.Move(100, 110);
    TS_ASSERT_EQUALS(xwin->x(), 100);
    TS_ASSERT_EQUALS(xwin->y(), 110);
  }

  void testResize() {
    XWindow* xwin = XWindow::Create(50, 60, 640, 480);
    wham::Window win(xwin);
    TS_ASSERT_EQUALS(xwin->width(), 640U);
    TS_ASSERT_EQUALS(xwin->height(), 480U);

    win.Resize(1024, 768);
    TS_ASSERT_EQUALS(xwin->width(), 1024U);
    TS_ASSERT_EQUALS(xwin->height(), 768U);
  }

  void testMap_Unmap() {
    MockXWindow* xwin =
        dynamic_cast<MockXWindow*>(XWindow::Create(50, 60, 640, 480));
    CHECK(xwin);
    wham::Window win(xwin);
    TS_ASSERT_EQUALS(xwin->mapped(), false);

    win.Map();
    TS_ASSERT_EQUALS(xwin->mapped(), true);

    win.Unmap();
    TS_ASSERT_EQUALS(xwin->mapped(), false);
  }
};
