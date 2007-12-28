// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __KEY_BINDINGS_H__
#define __KEY_BINDINGS_H__

#include <vector>

using namespace std;

class KeyBindingsTestSuite;

namespace wham {

class KeyBindings {
 public:
  // e.g. "Alt+Shift+U, Ctrl+C"
  void AddBinding(const string& binding);

 private:
  friend class ::KeyBindingsTestSuite;

  struct KeyBinding {
    enum Modifier {
      MOD_INVALID = 0,
      MOD_SHIFT   = 1 << 0,
      MOD_CONTROL = 1 << 1,
      MOD_MOD1    = 1 << 2
    };

    static Modifier StrToModifier(const string& str) {
      if (str == "Shift") return MOD_SHIFT;
      if (str == "Control" || str == "Ctrl") return MOD_CONTROL;
      if (str == "Mod1" || str == "Alt") return MOD_MOD1;
      return MOD_INVALID;
    }

    uint mod_mask;
    string key;
  };

  typedef vector<KeyBinding> KeyBindingSequence;

  static bool ParseSequence(const string& str,
                            KeyBindingSequence* bindings);
};

}  // namespace wham

#endif
