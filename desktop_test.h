// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "desktop.h"

#include "util.h"
#include "x.h"

using namespace wham;

class DesktopTestSuite : public CxxTest::TestSuite {
 public:
  void testDesktop_CreateAnchor() {
    XServer x_server;
    x_server.Init();
    XWindow::SetTesting(true);
    Desktop desktop;
    desktop.CreateAnchor("test", 10, 10);
  }
};
