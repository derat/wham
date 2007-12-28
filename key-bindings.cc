// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <pcrecpp.h>

#include "key-bindings.h"
#include "util.h"

namespace wham {

bool KeyBindings::ParseSequence(const string& str,
                                KeyBindingSequence* bindings,
                                string* error) {
  CHECK(bindings);
  if (error) *error = "";

  static pcrecpp::RE mod_re("\\s*((\\w+)\\s*\\+)");  // modifier and plus
  static pcrecpp::RE key_re("\\s*(\\w+)");           // main key
  static pcrecpp::RE comma_re("\\s*,");              // comma

  pcrecpp::StringPiece input(str);

  do {
    KeyBinding binding;

    string modifier;
    while (mod_re.Consume(&input, static_cast<void*>(NULL), &modifier)) {
      KeyBinding::Modifier mod_bit = KeyBinding::StrToModifier(modifier);
      if (mod_bit == KeyBinding::MOD_INVALID) {
        if (error) *error = "Got invalid modifier \"" + modifier + "\"";
        return false;
      }
      binding.mods |= mod_bit;
    }
    string key;
    if (!key_re.Consume(&input, &key)) {
      if (error) {
        *error = "Expected key at \"" + input.as_string() +
                 "\" but didn't find one";
      }
      return false;
    }
    binding.key = key;
    bindings->push_back(binding);
  } while (comma_re.Consume(&input));

  return true;
}

}  // namespace wham
