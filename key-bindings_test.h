// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "key-bindings.h"

#include "util.h"

using namespace wham;

#define MOD_CONTROL KeyBindings::KeyBinding::MOD_CONTROL
#define MOD_MOD1 KeyBindings::KeyBinding::MOD_MOD1
#define MOD_SHIFT KeyBindings::KeyBinding::MOD_SHIFT

namespace CxxTest {

// Make KeyBinding classes get pretty-printed by CxxTest.
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<KeyBindings::KeyBinding> {
 public:
  ValueTraits(const KeyBindings::KeyBinding& value)
      : value_(value) {
  }
  const char *asString() {
    if (str_.empty()) {
      if (value_.mods & MOD_CONTROL) str_ += "Control+";
      if (value_.mods & MOD_MOD1) str_ += "Mod1+";
      if (value_.mods & MOD_SHIFT) str_ += "Shift+";
      str_ += value_.key;
    }
    return str_.c_str();
  }
 private:
  KeyBindings::KeyBinding value_;
  string str_;
};

}  // namespace CxxTest


class KeyBindingsTestSuite : public CxxTest::TestSuite {
 public:
  void testKeyBindings_ParseSequence() {
    // Test a sequence of two bindings, each of which has modifiers.
    KeyBindings::KeyBindingSequence expected_seq;
    expected_seq.push_back(
        KeyBindings::KeyBinding(MOD_CONTROL | MOD_SHIFT, "U"));
    expected_seq.push_back(
        KeyBindings::KeyBinding(MOD_MOD1, "M"));
    KeyBindings::KeyBindingSequence seq;
    TS_ASSERT(KeyBindings::ParseSequence("Ctrl+Shift+U, Alt+M", &seq, NULL));
    TS_ASSERT_EQUALS(seq, expected_seq);

    // Test a single binding with lots of whitespace.
    expected_seq.clear();
    expected_seq.push_back(
        KeyBindings::KeyBinding(MOD_CONTROL | MOD_MOD1, "Foo"));
    seq.clear();
    TS_ASSERT(KeyBindings::ParseSequence(" Control + Mod1 + Foo ", &seq, NULL));
    TS_ASSERT_EQUALS(seq, expected_seq);

    // Test a sequence that doesn't have any modifiers.
    expected_seq.clear();
    expected_seq.push_back(KeyBindings::KeyBinding("b"));
    expected_seq.push_back(KeyBindings::KeyBinding("c"));
    seq.clear();
    TS_ASSERT(KeyBindings::ParseSequence("b, c", &seq, NULL));
    TS_ASSERT_EQUALS(seq, expected_seq);

    // Test a couple of malformed sequences.
    TS_ASSERT(!KeyBindings::ParseSequence("+R", &seq, NULL));
    TS_ASSERT(!KeyBindings::ParseSequence("Foo+J", &seq, NULL));
    TS_ASSERT(!KeyBindings::ParseSequence("Ctrl+", &seq, NULL));
  }

 private:
};
