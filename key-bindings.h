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

  struct KeyBinding {
    KeyBinding()
        : mods(0),
          key("") {}
    KeyBinding(const string& key)
        : mods(0),
          key(key) {}
    KeyBinding(uint mods, const string& key)
        : mods(mods),
          key(key) {}

    bool operator==(const KeyBinding& o) const {
      return mods == o.mods && key == o.key;
    }

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

    uint mods;
    string key;
  };

 private:
  friend class ::KeyBindingsTestSuite;

  typedef vector<KeyBinding> KeyBindingSequence;

  // Parse a string describing a sequence of key bindings (e.g.
  // "Ctrl+Alt+M, Shift+J").  The resulting sequence is stored in
  // 'bindings'.  False is returned on failure, in which case an error is
  // also logged to 'error' if it is non-NULL.
  static bool ParseSequence(const string& str,
                            KeyBindingSequence* bindings,
                            string* error);
};

}  // namespace wham

#endif
