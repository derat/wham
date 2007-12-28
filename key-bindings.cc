// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <pcrecpp.h>

#include "key-bindings.h"
#include "util.h"

namespace wham {

bool KeyBindings::ParseSequence(const string& str,
                                KeyBindingSequence* bindings) {
  CHECK(bindings);

  static pcrecpp::RE mod_re("\\s*((\\w+)\\s*\\+)");  // modifier and plus
  static pcrecpp::RE key_re("\\s*(\\w+)");           // main key
  static pcrecpp::RE comma_re("\\s*,");              // comma

  pcrecpp::StringPiece input(str);

  do {
    KeyBinding binding;

    string modifier;
    while (mod_re.Consume(&input, static_cast<void*>(NULL), &modifier)) {
      LOG << "got modifier " << modifier;
      KeyBinding::Modifier mod_bit = KeyBinding::StrToModifier(modifier);
      if (mod_bit == KeyBinding::MOD_INVALID) {
        ERROR << "Got invalid modifier \"" << modifier << "\"";
        return false;
      }
      binding.mod_mask |= mod_bit;
    }
    string key;
    if (!key_re.Consume(&input, &key)) {
      ERROR << "Expected key at \"" << input.as_string()
            << "\" but didn't find one";
      return false;
    }
    LOG << "got key " << key;
    binding.key = key;
    bindings->push_back(binding);
  } while (comma_re.Consume(&input));

  return true;
}

}  // namespace wham
