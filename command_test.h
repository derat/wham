// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "command.h"

#include "util.h"

using namespace wham;

class KeyBindingsTestSuite : public CxxTest::TestSuite {
 public:
  void testCommand_NoArg() {
    vector<string> args;

    Command cmd("create_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::CREATE_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), true);

    args.push_back("arg");
    cmd = Command("create_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::CREATE_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), false);
  }

  void testCommand_IntArg() {
    vector<string> args;

    // no args
    Command cmd("switch_nth_window", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NTH_WINDOW);
    TS_ASSERT_EQUALS(cmd.Valid(), false);

    // single integer arg
    args.push_back("10");
    cmd = Command("switch_nth_window", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NTH_WINDOW);
    TS_ASSERT_EQUALS(cmd.Valid(), true);
    TS_ASSERT_EQUALS(cmd.GetIntArg(), 10);

    // two integer args
    args.push_back("20");
    cmd = Command("switch_nth_window", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NTH_WINDOW);
    TS_ASSERT_EQUALS(cmd.Valid(), false);

    // single non-integer arg
    args.clear();
    args.push_back("10string_arg");
    cmd = Command("switch_nth_window", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NTH_WINDOW);
    TS_ASSERT_EQUALS(cmd.Valid(), false);
  }

  void testCommand_StringArg() {
    vector<string> args;

    // no args
    Command cmd("exec", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::EXEC);
    TS_ASSERT_EQUALS(cmd.Valid(), false);

    // single string arg
    args.push_back("/bin/ls");
    cmd = Command("exec", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::EXEC);
    TS_ASSERT_EQUALS(cmd.Valid(), true);
    TS_ASSERT_EQUALS(cmd.GetStringArg(), "/bin/ls");

    // two string args
    args.push_back("extra arg");
    cmd = Command("exec", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::EXEC);
    TS_ASSERT_EQUALS(cmd.Valid(), false);
  }

  void testCommand_DirectionArg() {
    vector<string> args;

    // no args
    Command cmd("switch_nearest_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NEAREST_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), false);

    // single direction arg
    args.push_back("up");
    cmd = Command("switch_nearest_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NEAREST_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), true);
    TS_ASSERT_EQUALS(cmd.GetDirectionArg(), Command::UP);

    // single direction arg with mixed case
    args.clear();
    args.push_back("Down");
    cmd = Command("switch_nearest_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NEAREST_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), true);
    TS_ASSERT_EQUALS(cmd.GetDirectionArg(), Command::DOWN);

    // two direction args
    args.push_back("left");
    cmd = Command("switch_nearest_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NEAREST_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), false);

    // single non-direction arg
    args.clear();
    args.push_back("lefty");
    cmd = Command("switch_nearest_anchor", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::SWITCH_NEAREST_ANCHOR);
    TS_ASSERT_EQUALS(cmd.Valid(), false);
  }

  void testCommand_BogusCommand() {
    Command cmd;
    TS_ASSERT_EQUALS(cmd.type(), Command::UNKNOWN);
    TS_ASSERT_EQUALS(cmd.Valid(), false);

    vector<string> args;
    cmd = Command("not_a_command", args);
    TS_ASSERT_EQUALS(cmd.type(), Command::UNKNOWN);
    TS_ASSERT_EQUALS(cmd.Valid(), false);
  }

  void testCommand_ToName() {
    TS_ASSERT_EQUALS(Command::ToName(Command::CREATE_ANCHOR), "create_anchor");
    TS_ASSERT_EQUALS(Command::ToName(Command::UNKNOWN), "unknown");
  }

  void testCommand_ToType() {
    TS_ASSERT_EQUALS(Command::ToType("create_anchor"), Command::CREATE_ANCHOR);
    TS_ASSERT_EQUALS(Command::ToType("foo"), Command::UNKNOWN);
  }

  void testCommand_NumArgs() {
    TS_ASSERT_EQUALS(Command::GetArgType(Command::CREATE_ANCHOR),
                     Command::NO_ARG);
    TS_ASSERT_EQUALS(Command::GetArgType(Command::EXEC),
                     Command::STRING_ARG);
    TS_ASSERT_EQUALS(Command::GetArgType(Command::SWITCH_NEAREST_ANCHOR),
                     Command::DIRECTION_ARG);
    TS_ASSERT_EQUALS(Command::GetArgType(Command::SWITCH_NTH_WINDOW),
                     Command::INT_ARG);
    TS_ASSERT_EQUALS(Command::GetArgType(Command::UNKNOWN),
                     Command::NO_ARG);
  }
};
