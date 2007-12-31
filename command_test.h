// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "command.h"

#include "util.h"

using namespace wham;

class KeyBindingsTestSuite : public CxxTest::TestSuite {
 public:
  void testCommand_ToName() {
    TS_ASSERT_EQUALS(Command::ToName(Command::CREATE_ANCHOR), "create_anchor");
    TS_ASSERT_EQUALS(Command::ToName(Command::UNKNOWN), "unknown");
  }

  void testCommand_ToType() {
    TS_ASSERT_EQUALS(Command::ToType("create_anchor"), Command::CREATE_ANCHOR);
    TS_ASSERT_EQUALS(Command::ToType("foo"), Command::UNKNOWN);
  }

  void testCommand_NumArgs() {
    TS_ASSERT_EQUALS(Command::NumArgs(Command::CREATE_ANCHOR), 0U);
    TS_ASSERT_EQUALS(Command::NumArgs(Command::SWITCH_WINDOW), 1U);
    TS_ASSERT_EQUALS(Command::NumArgs(Command::UNKNOWN), 0U);
  }
};
