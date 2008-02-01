// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __KEY_BINDINGS_H__
#define __KEY_BINDINGS_H__

#include <map>
#include <string>
#include <vector>

#include "command.h"
#include "config-parser.h"

using namespace std;

class KeyBindingsTestSuite;

namespace wham {

class KeyBindings {
 public:
  bool Load(const ConfigNode& conf);

  bool AddBinding(const string& combos_str,
                  const string& command_str,
                  const vector<string>& args,
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

  struct Binding {
    Binding()
        : combos(),
          command() {}

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

  vector<Binding> bindings_;
};

}  // namespace wham

#endif
