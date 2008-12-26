// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "window.h"

#include "config.h"
#include "mock-x-window.h"
#include "window-classifier.h"
#include "window-properties.h"
#include "x-server.h"
#include "x-window.h"

using namespace wham;

class WindowTestSuite : public CxxTest::TestSuite {
 public:
  void setUp() {
    XServer::SetupTesting();
  }

  void testMove() {
    XWindow* xwin = XWindow::Create(50, 60, 640, 480);
    wham::Window win(xwin);
    int border = static_cast<int>(Config::Get()->window_border);

    // The Window object's default position should match that of its frame,
    // which should be a bit to the upper-left of the client window's
    // original position (to accomodate the border).
    TS_ASSERT_EQUALS(win.x(), 50 - border);
    TS_ASSERT_EQUALS(win.y(), 60 - border);
    TS_ASSERT_EQUALS(win.frame()->x(), win.x());
    TS_ASSERT_EQUALS(win.frame()->y(), win.y());

    // The client window should've been reparented under the frame, and it
    // should be offset the correct amount.
    TS_ASSERT_EQUALS(xwin->parent(), win.frame());
    TS_ASSERT_EQUALS(xwin->x(), border);
    TS_ASSERT_EQUALS(xwin->y(), border);

    // Now move the window and check that everything gets updated
    // correctly -- Move() takes the position of the frame, not of the
    // client window.
    win.Move(100, 110);
    TS_ASSERT_EQUALS(win.x(), 100);
    TS_ASSERT_EQUALS(win.y(), 110);
    TS_ASSERT_EQUALS(win.frame()->x(), win.x());
    TS_ASSERT_EQUALS(win.frame()->y(), win.y());

    TS_ASSERT_EQUALS(xwin->x(), border);
    TS_ASSERT_EQUALS(xwin->y(), border);
  }

  void testResize() {
    XWindow* xwin = XWindow::Create(50, 60, 640, 480);
    wham::Window win(xwin);
    uint border = Config::Get()->window_border;

    // The client window's size should be unchanged, and its frame should
    // be a bit larger to accomodate the border.
    TS_ASSERT_EQUALS(xwin->width(), 640U);
    TS_ASSERT_EQUALS(xwin->height(), 480U);
    TS_ASSERT_EQUALS(win.frame_width(), 640U + 2 * border);
    TS_ASSERT_EQUALS(win.frame_height(), 480U + 2 * border);

    // Resize() takes the size of the client window.
    win.Resize(1024, 768);
    TS_ASSERT_EQUALS(xwin->width(), 1024U);
    TS_ASSERT_EQUALS(xwin->height(), 768U);
    TS_ASSERT_EQUALS(win.frame_width(), 1024U + 2 * border);
    TS_ASSERT_EQUALS(win.frame_height(), 768U + 2 * border);
  }

  void testMap_Unmap() {
    MockXWindow* xwin =
        dynamic_cast<MockXWindow*>(XWindow::Create(50, 60, 640, 480));
    CHECK(xwin);
    wham::Window win(xwin);

    MockXWindow* frame = dynamic_cast<MockXWindow*>(win.frame());
    CHECK(frame);

    // We should initially map the client window but not the frame.
    // Further calls to Map() and Unmap() should only affect the frame.
    TS_ASSERT(xwin->mapped());
    TS_ASSERT(!frame->mapped());

    win.Map();
    TS_ASSERT(xwin->mapped());
    TS_ASSERT(frame->mapped());

    win.Unmap();
    TS_ASSERT(xwin->mapped());
    TS_ASSERT(!frame->mapped());
  }

  void testApplyConfig() {
    // At first, the window should be left at its initial size.
    uint initial_width = 200, initial_height = 100;
    XWindow* xwin = XWindow::Create(100, 200, initial_width, initial_height);
    wham::Window win(xwin);
    TS_ASSERT_EQUALS(xwin->width(), 200U);
    TS_ASSERT_EQUALS(xwin->height(), 100U);

    // Test applying a config with pixel dimensions.
    WindowConfig config("test", 400, 200);
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(), 400U);
    TS_ASSERT_EQUALS(xwin->height(), 200U);

    WindowProperties& props = win.props_;

    // Test applying a config with unit dimensions.
    config.width_type = WindowConfig::DIMENSION_UNITS;
    config.width = 80;
    config.height_type = WindowConfig::DIMENSION_UNITS;
    config.height = 25;
    props.base_width = 20;
    props.base_height = 10;
    props.width_inc = 10;
    props.height_inc = 5;
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(),
                     props.base_width + config.width * props.width_inc);
    TS_ASSERT_EQUALS(xwin->height(),
                     props.base_height + config.height * props.height_inc);

    // Test applying a config that just uses the app-supplied dimensions.
    // When the app didn't supply any dimensions in its hints, we should
    // use the initial size.
    config.width_type = WindowConfig::DIMENSION_APP;
    config.height_type = WindowConfig::DIMENSION_APP;
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(), initial_width);
    TS_ASSERT_EQUALS(xwin->height(), initial_height);

    // When it did supply dimensions via hints, we should use those.
    props.width = 640;
    props.height = 480;
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(), props.width);
    TS_ASSERT_EQUALS(xwin->height(), props.height);

    // FIXME: Add tests for the maximized settings once I've figured out
    // how those should work.

    // Switch back to pixel dimensions.
    config.width_type = WindowConfig::DIMENSION_PIXELS;
    config.width = 400;
    config.height_type = WindowConfig::DIMENSION_PIXELS;
    config.height = 300;
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(), 400U);
    TS_ASSERT_EQUALS(xwin->height(), 300U);

    // Test that min constraints limit the window's size ...
    props.min_width = 600;
    props.min_height = 400;
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(), props.min_width);
    TS_ASSERT_EQUALS(xwin->height(), props.min_height);

    // ... and that max constraints work too.
    props.min_width = 0;
    props.min_height = 0;
    props.max_width = 300;
    props.max_height = 200;
    win.ApplyConfig(config);
    TS_ASSERT_EQUALS(xwin->width(), props.max_width);
    TS_ASSERT_EQUALS(xwin->height(), props.max_height);
  }
};
