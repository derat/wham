// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __KEY_BINDINGS_H__
#define __KEY_BINDINGS_H__

#include <map>
#include <vector>

using namespace std;

class KeyBindingsTestSuite;

namespace wham {

class KeyBindings {
 public:
  bool AddBinding(const string& combos_str,
                  const string& command_str,
                  string* error);

  struct Combo {
    Combo()
        : mods(0),
          key("") {}
    Combo(const string& key)
        : mods(0),
          key(key) {}
    Combo(uint mods, const string& key)
        : mods(mods),
          key(key) {}

    bool operator==(const Combo& o) const {
      return mods == o.mods && key == o.key;
    }

    enum Modifier {
      MOD_INVALID = 0,
      MOD_SHIFT   = 1 << 0,
      MOD_CONTROL = 1 << 1,
      MOD_MOD1    = 1 << 2,
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

  enum Command {
    CMD_UNKNOWN,
    CMD_CREATE_ANCHOR,
  };

  struct KeyBinding {
    KeyBinding()
        : combos(),
          command(CMD_UNKNOWN) {}

    vector<Combo> combos;
    Command command;
  };

 private:
  friend class ::KeyBindingsTestSuite;

  // Parse a string describing a sequence of key bindings (e.g.
  // "Ctrl+Alt+M, Shift+J").  The resulting sequence is stored in
  // 'bindings'.  False is returned on failure, in which case an error is
  // also logged to 'error' if it is non-NULL.
  static bool ParseCombos(const string& str,
                          vector<Combo>* combos,
                          string* error);

  static Command LookupCommand(const string& str);

  struct CommandMapping {
    string str;
    Command cmd;
  };

  static CommandMapping command_mappings_[];
  static map<string, Command> commands_;
  static bool commands_initialized_;
};

}  // namespace wham

#endif
