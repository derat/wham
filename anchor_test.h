// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "anchor.h"

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

  void testAnchor_Ctor() {
    Anchor anchor("test", 10, 20);
    TS_ASSERT_EQUALS(anchor.name(), "test");
    TS_ASSERT_EQUALS(anchor.x(), 10);
    TS_ASSERT_EQUALS(anchor.y(), 20);
    TS_ASSERT_EQUALS(anchor.gravity(), Anchor::TOP_LEFT);
  }

  ref_ptr<XServer> x_server_;
};
