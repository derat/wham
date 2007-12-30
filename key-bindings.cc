// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <pcrecpp.h>

#include "key-bindings.h"
#include "util.h"

namespace wham {

KeyBindings::CommandMapping KeyBindings::command_mappings_[] = {
  { "close_window",  CMD_CLOSE_WINDOW },
  { "create_anchor", CMD_CREATE_ANCHOR },
  { "TERMINATOR", CMD_UNKNOWN },
};

map<string, KeyBindings::Command> KeyBindings::commands_;

bool KeyBindings::commands_initialized_ = false;


bool KeyBindings::AddBinding(const string& combos_str,
                             const string& command_str,
                             string* error) {
  if (error) *error = "";

  Binding binding;
  if (!ParseCombos(combos_str, &(binding.combos), error)) return false;

  binding.command = StrToCommand(command_str);
  if (binding.command == CMD_UNKNOWN) {
    if (error) *error = "Unknown command \"" + command_str + "\"";
    return false;
  }

  bindings_.push_back(binding);
  return true;
}


string KeyBindings::CommandToStr(Command cmd) {
  for (int i = 0; command_mappings_[i].cmd != CMD_UNKNOWN; ++i) {
    if (command_mappings_[i].cmd == cmd) {
      return command_mappings_[i].str;
    }
  }
  return "unknown";
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

    string modifier;
    while (mod_re.Consume(&input, static_cast<void*>(NULL), &modifier)) {
      Combo::Modifier mod_bit = Combo::StrToModifier(modifier);
      if (mod_bit == Combo::MOD_UNKNOWN) {
        if (error) *error = "Got unknown modifier \"" + modifier + "\"";
        return false;
      }
      combo.mods |= mod_bit;
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


KeyBindings::Command KeyBindings::StrToCommand(const string& str) {
  if (!commands_initialized_) {
    for (int i = 0; command_mappings_[i].cmd != CMD_UNKNOWN; ++i) {
      commands_.insert(
          make_pair(command_mappings_[i].str, command_mappings_[i].cmd));
    }
    commands_initialized_ = true;
  }
  return FindWithDefault(commands_, str, CMD_UNKNOWN);
}

}  // namespace wham
