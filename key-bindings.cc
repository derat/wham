// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "key-bindings.h"

#include <pcrecpp.h>

#include "config-parser.h"
#include "util.h"

namespace wham {

bool KeyBindings::Load(const ConfigNode& conf,
                       vector<ConfigError>* errors) {
  CHECK(errors);
  CHECK(!conf.tokens.empty());
  CHECK(conf.tokens[0] == "key_bindings");

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());
    if (node.tokens.empty()) {
      errors->push_back(
          ConfigError("Got empty block within key bindings", node.line_num));
    } else if (node.tokens[0] == "mod_alias") {
      if (node.tokens.size() != 3) {
        string msg = StringPrintf(
            "\"mod_alias\" requires 2 arguments (alias and replacement); "
            "got %d instead", node.tokens.size() - 1);
        errors->push_back(ConfigError(msg, node.line_num));
        continue;
      }
      vector<string> parts;
      SplitStringUsing(node.tokens[2], "+", &parts);
      mod_aliases_[node.tokens[1]] = parts;
    } else if (node.tokens[0] == "bind") {
      if (node.tokens.size() < 3) {
        string msg = StringPrintf(
            "\"bind\" requires at least 2 arguments (key combination "
            "and command); got %d instead", node.tokens.size() - 1);
        errors->push_back(ConfigError(msg, node.line_num));
        continue;
      }
      vector<string> args;
      for (size_t i = 3; i < node.tokens.size(); ++i) {
        args.push_back(node.tokens[i]);
      }
      string error_msg;
      if (!AddBinding(node.tokens[1], node.tokens[2], args, &error_msg)) {
        errors->push_back(ConfigError(error_msg, node.line_num));
      }
    } else {
      string msg = StringPrintf("Got unknown token \"%s\" with %d parameter(s)",
                                node.tokens[0].c_str(), node.tokens.size() - 1);
      errors->push_back(ConfigError(msg, node.line_num));
    }
  }
  return true;
}


bool KeyBindings::AddBinding(const string& combos_str,
                             const string& command_str,
                             const vector<string>& args,
                             string* error) {
  if (error) *error = "";

  Binding binding;
  if (!ParseCombos(combos_str, &(binding.combos), error)) return false;

  binding.command = Command(command_str, args);
  if (binding.command.type() == Command::UNKNOWN) {
    if (error) *error = "Unknown command \"" + command_str + "\"";
    return false;
  }
  if (!binding.command.Valid()) {
    if (error) {
      *error = "Invalid arguments supplied for command \"" + command_str + "\"";
    }
    return false;
  }

  bindings_.push_back(binding);
  return true;
}


bool KeyBindings::ParseCombos(const string& str,
                              vector<Combo>* combos,
                              string* error) {
  CHECK(combos);
  if (error) *error = "";

  static pcrecpp::RE mod_re("\\s*((\\w+)\\s*\\+)");  // modifier and plus
  static pcrecpp::RE key_re("\\s*(\\w+)");           // main key
  static pcrecpp::RE comma_re("\\s*,");              // comma

  pcrecpp::StringPiece input(str);

  do {
    Combo combo;

    string mod;
    while (mod_re.Consume(&input, static_cast<void*>(NULL), &mod)) {
      map<string, vector<string> >::const_iterator alias =
          mod_aliases_.find(mod);
      if (alias != mod_aliases_.end()) {
        for (vector<string>::const_iterator alias_mod = alias->second.begin();
             alias_mod != alias->second.end(); ++alias_mod) {
          combo.mods.push_back(*alias_mod);
        }
      } else {
        combo.mods.push_back(mod);
      }
    }
    string key;
    if (!key_re.Consume(&input, &key)) {
      if (error) {
        *error = "Expected key at \"" + input.as_string() +
                 "\" but didn't find one";
      }
      return false;
    }
    combo.key = key;
    combos->push_back(combo);
  } while (comma_re.Consume(&input));

  return true;
}

}  // namespace wham
