// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "util.h"
#include "x.h"

using namespace wham;

class XTestSuite : public CxxTest::TestSuite {
 public:
  void testXServer_UpdateKeyBindingMap() {
    KeyBindings bindings;
    XServer::XKeyBindingMap binding_map;

    TS_ASSERT(bindings.AddBinding("Mod1+N", "create_anchor", NULL));
    TS_ASSERT(bindings.AddBinding("Ctrl+U,C", "close_window", NULL));
    TS_ASSERT(bindings.AddBinding("Ctrl+U,Shift+N", "create_anchor", NULL));

    XServer::UpdateKeyBindingMap(bindings, &binding_map);

    TS_ASSERT_EQUALS(binding_map.size(), 2U);

    const XKeyBinding* binding = GetBinding(binding_map, XK_N, Mod1Mask);
    TS_ASSERT(binding != NULL);
    if (binding) {
      TS_ASSERT_EQUALS(binding->keysym, static_cast<uint>(XK_N));
      TS_ASSERT_EQUALS(binding->required_mods, static_cast<uint>(Mod1Mask));
      TS_ASSERT_EQUALS(binding->inherited_mods, 0U);
      TS_ASSERT_EQUALS(binding->command, KeyBindings::CMD_CREATE_ANCHOR);
      TS_ASSERT(binding->children.empty());
    }

    binding = GetBinding(binding_map, XK_U, ControlMask);
    TS_ASSERT(binding != NULL);
    if (binding) {
      TS_ASSERT_EQUALS(binding->keysym, static_cast<uint>(XK_U));
      TS_ASSERT_EQUALS(binding->required_mods, static_cast<uint>(ControlMask));
      TS_ASSERT_EQUALS(binding->inherited_mods, 0U);
      TS_ASSERT_EQUALS(binding->command, KeyBindings::CMD_UNKNOWN);
      TS_ASSERT_EQUALS(binding->children.size(), 2U);
      if (binding->children.size() == 2) {
        XKeyBinding* child = binding->children[0].get();
        TS_ASSERT_EQUALS(child->keysym, static_cast<uint>(XK_C));
        TS_ASSERT_EQUALS(child->required_mods, 0U);
        TS_ASSERT_EQUALS(child->inherited_mods, static_cast<uint>(ControlMask));
        TS_ASSERT_EQUALS(child->command, KeyBindings::CMD_CLOSE_WINDOW);
        TS_ASSERT(child->children.empty());

        child = binding->children[1].get();
        TS_ASSERT_EQUALS(child->keysym, static_cast<uint>(XK_N));
        TS_ASSERT_EQUALS(child->required_mods, static_cast<uint>(ShiftMask));
        TS_ASSERT_EQUALS(child->inherited_mods, static_cast<uint>(ControlMask));
        TS_ASSERT_EQUALS(child->command, KeyBindings::CMD_CREATE_ANCHOR);
        TS_ASSERT(child->children.empty());
      }
    }
  }

  static const XKeyBinding* GetBinding(
      const XServer::XKeyBindingMap& binding_map, KeySym keysym, uint mods) {
    XServer::XKeyCombo combo = make_pair(keysym, mods);
    XServer::XKeyBindingMap::const_iterator it = binding_map.find(combo);
    if (it == binding_map.end()) return NULL;
    return it->second.get();
  }
};
