// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __KEY_BINDINGS_H__
#define __KEY_BINDINGS_H__

#include <map>
#include <string>
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
    Combo() {}
    Combo(const string& key)
        : key(key) {}
    Combo(const string& key, const vector<string>& mods)
        : key(key),
          mods(mods) {}

    bool operator==(const Combo& o) const {
      return key == o.key && mods == o.mods;
    }

    // Get a string representation of this combo.
    string ToString() const {
      string str;
      for (vector<string>::const_iterator mod = mods.begin();
           mod != mods.end(); ++mod) {
        str += *mod + "+";
      }
      str += key;
      return str;
    }

    // A string representation of the key's name
    string key;

    // String representations of modifier keys
    vector<string> mods;
  };

  enum Command {
    CMD_UNKNOWN,
    CMD_CLOSE_WINDOW,
    CMD_CREATE_ANCHOR,
    CMD_EXEC_TERM,
  };

  // Get the string representation of a command.
  static string CommandToStr(Command cmd);

  struct Binding {
    Binding()
        : combos(),
          command(CMD_UNKNOWN) {}

    string ToString() const {
      string str;
      for (vector<Combo>::const_iterator combo = combos.begin();
           combo != combos.end(); ++combo) {
        if (!str.empty()) str += ", ";
        str += combo->ToString();
      }
      return str;
    }

    vector<Combo> combos;
    Command command;
  };

  const vector<Binding>& bindings() const { return bindings_; }

 private:
  friend class ::KeyBindingsTestSuite;

  // Parse a string describing a sequence of key bindings (e.g.
  // "Ctrl+Alt+M, Shift+J").  The resulting sequence is stored in
  // 'bindings'.  False is returned on failure, in which case an error is
  // also logged to 'error' if it is non-NULL.
  static bool ParseCombos(const string& str,
                          vector<Combo>* combos,
                          string* error);

  // Look up a command from its string representation.
  // Returns CMD_UNKNOWN for invalid strings.
  static Command StrToCommand(const string& str);

  vector<Binding> bindings_;

  // An array containing string representations of commands and the
  // corresponding enum values
  struct CommandMapping {
    string str;
    Command cmd;
  };
  static CommandMapping command_mappings_[];

  // A map generated from command_mappings_ and a bool to track whether
  // it's been initialized yet or not
  static map<string, Command> commands_;
  static bool commands_initialized_;
};

}  // namespace wham

#endif
