// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "key-bindings.h"

#include "util.h"

using namespace wham;

namespace CxxTest {

// Make Combo structs get pretty-printed by CxxTest.
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<KeyBindings::Combo> {
 public:
  ValueTraits(const KeyBindings::Combo& value)
      : value_(value) {
  }
  const char *asString() {
    if (str_.empty()) str_ = value_.ToString();
    return str_.c_str();
  }
 private:
  KeyBindings::Combo value_;
  string str_;
};

}  // namespace CxxTest


class KeyBindingsTestSuite : public CxxTest::TestSuite {
 public:
  void testParseCombos() {
    // Test a sequence of two bindings, each of which has modifiers.
    vector<KeyBindings::Combo> expected_seq;
    expected_seq.push_back(
        KeyBindings::Combo("U", SplitString("Ctrl Shift")));
    expected_seq.push_back(
        KeyBindings::Combo("M", SplitString("Alt")));
    vector<KeyBindings::Combo> seq;
    TS_ASSERT(KeyBindings::ParseCombos("Ctrl+Shift+U, Alt+M", &seq, NULL));
    TS_ASSERT_EQUALS(seq, expected_seq);

    // Test a single binding with lots of whitespace.
    expected_seq.clear();
    expected_seq.push_back(
        KeyBindings::Combo("Foo", SplitString("Control Mod1")));
    seq.clear();
    TS_ASSERT(KeyBindings::ParseCombos(" Control + Mod1 + Foo ", &seq, NULL));
    TS_ASSERT_EQUALS(seq, expected_seq);

    // Test a sequence that doesn't have any modifiers.
    expected_seq.clear();
    expected_seq.push_back(KeyBindings::Combo("b"));
    expected_seq.push_back(KeyBindings::Combo("c"));
    seq.clear();
    TS_ASSERT(KeyBindings::ParseCombos("b, c", &seq, NULL));
    TS_ASSERT_EQUALS(seq, expected_seq);

    // Test a couple of malformed sequences.
    TS_ASSERT(!KeyBindings::ParseCombos("+R", &seq, NULL));
    TS_ASSERT(!KeyBindings::ParseCombos("Ctrl+", &seq, NULL));
  }
};
