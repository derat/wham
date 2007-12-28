// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "key-bindings.h"

using namespace wham;

class KeyBindingsTestSuite : public CxxTest::TestSuite {
 public:
  void testKeyBindings_ParseSequence() {
    KeyBindings::KeyBindingSequence seq;
    TS_ASSERT(KeyBindings::ParseSequence("Ctrl+Shift+U, Alt+M", &seq));
    TS_ASSERT(!KeyBindings::ParseSequence("Ctrl+", &seq));
  }
};
