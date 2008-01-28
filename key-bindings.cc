// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "key-bindings.h"

#include <pcrecpp.h>

#include "util.h"

namespace wham {

bool KeyBindings::Load(const ParsedConfig::Node& conf) {
  for (vector<ref_ptr<ParsedConfig::Node> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ParsedConfig::Node& node = *(it->get());
    if (node.tokens.empty()) {
      ERROR << "Got empty top-level node from config";
      return false;
    }
    if (node.tokens[0] == "bind") {
      if (node.tokens.size() < 3) {
        ERROR << "\"bind\" requires at least 2 arguments; got "
              << (node.tokens.size() - 1) << " instead";
        return false;
      }
      vector<string> args;
      for (size_t i = 3; i < node.tokens.size(); ++i) {
        args.push_back(node.tokens[i]);
      }
      if (!AddBinding(node.tokens[1], node.tokens[2], args, NULL)) {
        return false;
      }
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
      combo.mods.push_back(mod);
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
