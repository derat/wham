// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __X_SERVER_H__
#define __X_SERVER_H__

#include <map>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "command.h"
#include "util.h"
#include "x-window.h"

using namespace std;

class XServerTestSuite;  // from x-server_test.h

namespace wham {

class KeyBindings;
class WindowManager;


struct XKeyBinding {
  XKeyBinding(KeySym keysym,
              uint required_mods,
              uint inherited_mods,
              Command command)
      : keysym(keysym),
        required_mods(required_mods),
        inherited_mods(inherited_mods),
        command(command),
        children() {}

  KeySym keysym;
  uint required_mods;
  uint inherited_mods;
  Command command;
  vector<ref_ptr<XKeyBinding> > children;
};


// Represents a connection to an X server.
class XServer {
 public:
  XServer();

  // Get the current X connection.
  static XServer* Get() {
    CHECK(singleton_.get());
    return singleton_.get();
  }

  // Swap in a new connection to an X server.
  static void Swap(ref_ptr<XServer> new_x_server) {
    singleton_.swap(new_x_server);
  }

  // Helper method that test suites can call in their setUp() methods.
  static void SetupTesting();

  // Connect to the real X server and initialize internal data.
  // Returns true on success.
  bool Init();

  // Has Init() been called successfully?
  bool Initialized() const { return initialized_; }

  // Start reading events from the X server and handling them.
  void RunEventLoop(WindowManager* window_manager);

  Display* display() { return display_; }
  int screen_num() { return screen_num_; }
  ::Window root() { return root_; }
  uint width() const { return width_; }
  uint height() const { return height_; }

  void RegisterKeyBindings(const KeyBindings& bindings);

  // FIXME: clean this up
  static void SetTesting(bool testing) { testing_ = testing; }
  static bool Testing() { return testing_; }

 private:
  friend class ::XServerTestSuite;
  friend class XWindow;

  XWindow* GetWindow(::Window id, bool create);
  void DeleteWindow(::Window id);

  // Read the next event (or possibly more than one, in the case of expose
  // events) and handle it.
  void ProcessEvent(WindowManager *window_manager);

  // Convert a vector containing string representations of modifiers keys
  // into a bitmap consisting of the corresponding X modifier masks.
  // Returns false if any unknown modifiers were seen.
  static bool GetModifiers(const vector<string>& mods, uint* mod_bits);

  static KeySym LowercaseKeysym(KeySym keysym);

  typedef pair<KeySym, uint> XKeyCombo;
  typedef map<XKeyCombo, ref_ptr<XKeyBinding> > XKeyBindingMap;

  static void UpdateKeyBindingMap(const KeyBindings& bindings,
                                  XKeyBindingMap* binding_map);

  void HandleKeyPress(KeySym keysym, uint mods, WindowManager* window_manager);

  Display* display_;
  int screen_num_;
  ::Window root_;

  // Dimensions of the root window.
  uint width_;
  uint height_;

  bool initialized_;

  typedef map< ::Window, ref_ptr<XWindow> > XWindowMap;
  XWindowMap windows_;

  XKeyBindingMap bindings_;
  XKeyBinding* in_progress_binding_;

  // Singleton object.
  static ref_ptr<XServer> singleton_;

  static bool testing_;

  DISALLOW_EVIL_CONSTRUCTORS(XServer);
};

}  // namespace wham

#endif
