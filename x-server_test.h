// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "command.h"
#include "key-bindings.h"
#include "util.h"
#include "x-server.h"

using namespace wham;

class XServerTestSuite : public CxxTest::TestSuite {
 public:
  void testGetModifiers() {
    vector<string> mods;
    uint mod_bits = 0;

    TS_ASSERT(XServer::GetModifiers(mods, &mod_bits));

    mods.push_back("Control");
    mods.push_back("Shift");
    TS_ASSERT(XServer::GetModifiers(mods, &mod_bits));
    TS_ASSERT_EQUALS(mod_bits, static_cast<uint>(ControlMask|ShiftMask));

    mods.clear();
    mods.push_back("Ctrl");
    mods.push_back("Mod1");
    TS_ASSERT(XServer::GetModifiers(mods, &mod_bits));
    TS_ASSERT_EQUALS(mod_bits, static_cast<uint>(ControlMask|Mod1Mask));

    mods.clear();
    mods.push_back("Foo");
    mods.push_back("mod1");
    TS_ASSERT(!XServer::GetModifiers(mods, &mod_bits));
    TS_ASSERT_EQUALS(mod_bits, static_cast<uint>(Mod1Mask));
  }

  void testUpdateKeyBindingMap() {
    KeyBindings bindings;
    XServer::XKeyBindingMap binding_map;

    vector<string> args;
    TS_ASSERT(bindings.AddBinding("Mod1+N", "create_anchor", args, NULL));
    TS_ASSERT(bindings.AddBinding("Ctrl+U,c", "close_window", args, NULL));
    TS_ASSERT(
        bindings.AddBinding("Ctrl+u,Shift+N", "create_anchor", args, NULL));

    XServer::UpdateKeyBindingMap(bindings, &binding_map);

    TS_ASSERT_EQUALS(binding_map.size(), 2U);

    const XKeyBinding* binding = GetBinding(binding_map, XK_n, Mod1Mask);
    TS_ASSERT(binding != NULL);
    if (binding) {
      TS_ASSERT_EQUALS(binding->keysym, static_cast<uint>(XK_n));
      TS_ASSERT_EQUALS(binding->required_mods, static_cast<uint>(Mod1Mask));
      TS_ASSERT_EQUALS(binding->inherited_mods, 0U);
      TS_ASSERT_EQUALS(binding->command.type(), Command::CREATE_ANCHOR);
      TS_ASSERT(binding->children.empty());
    }

    binding = GetBinding(binding_map, XK_u, ControlMask);
    TS_ASSERT(binding != NULL);
    if (binding) {
      TS_ASSERT_EQUALS(binding->keysym, static_cast<uint>(XK_u));
      TS_ASSERT_EQUALS(binding->required_mods, static_cast<uint>(ControlMask));
      TS_ASSERT_EQUALS(binding->inherited_mods, 0U);
      TS_ASSERT_EQUALS(binding->command.type(), Command::UNKNOWN);
      TS_ASSERT_EQUALS(binding->children.size(), 2U);
      if (binding->children.size() == 2) {
        XKeyBinding* child = binding->children[0].get();
        TS_ASSERT_EQUALS(child->keysym, static_cast<uint>(XK_c));
        TS_ASSERT_EQUALS(child->required_mods, 0U);
        TS_ASSERT_EQUALS(child->inherited_mods, static_cast<uint>(ControlMask));
        TS_ASSERT_EQUALS(child->command.type(), Command::CLOSE_WINDOW);
        TS_ASSERT(child->children.empty());

        child = binding->children[1].get();
        TS_ASSERT_EQUALS(child->keysym, static_cast<uint>(XK_n));
        TS_ASSERT_EQUALS(child->required_mods, static_cast<uint>(ShiftMask));
        TS_ASSERT_EQUALS(child->inherited_mods, static_cast<uint>(ControlMask));
        TS_ASSERT_EQUALS(child->command.type(), Command::CREATE_ANCHOR);
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
