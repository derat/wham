// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __X_H__
#define __X_H__

#include <map>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "command.h"
#include "util.h"

using namespace std;

class XTestSuite;  // from x_test.h

namespace wham {

class KeyBindings;
class WindowManager;
class WindowProperties;
class XServer;

class XWindow {
 public:
  XWindow(::Window id);

  static XWindow* Create(int x, int y, uint width, uint height);

  static void GetTextSize(const string& font, const string& text,
                          int* width, int* ascent, int* descent);
  void Clear();
  void DrawText(int x, int y, const string& text, const string& color);
  void DrawLine(int x1, int y1, int x2, int y2, const string& color);
  void DrawBox(int x, int y, uint width, uint height, const string& color);

  bool GetProperties(WindowProperties* props);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();

  static XServer* GetServer() { return server_; }

  bool operator<(const XWindow& o) const {
    return id_ < o.id_;
  }

 private:
  friend class XServer;

  ::Window id_;

  static XServer* server_;
};


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


class XServer {
 public:
  XServer();
  // FIXME: free fonts_ and gcs_ in d'tor

  // Connect to the real X server and initialize internal objects.
  // Returns true on success.
  bool Init();

  // Start reading events from the X server and handling them.
  void RunEventLoop(WindowManager* window_manager);

  Display* display() { return display_; }
  int screen_num() { return screen_num_; }
  ::Window root() { return root_; }

  XWindow* GetWindow(::Window id, bool create);

  GC GetGC(const string& name);

  XFontStruct* GetFontInfo(const string& font);

  void RegisterKeyBindings(const KeyBindings& bindings);

 private:
  friend class ::XTestSuite;

  // Convert a vector containing string representations of modifiers keys
  // into a bitmap consisting of the corresponding X modifier masks.
  // Returns false if any unknown modifiers were seen.
  static bool GetModifiers(const vector<string>& mods, uint* mod_bits);

  typedef pair<KeySym, uint> XKeyCombo;
  typedef map<XKeyCombo, ref_ptr<XKeyBinding> > XKeyBindingMap;

  static void UpdateKeyBindingMap(const KeyBindings& bindings,
                                  XKeyBindingMap* binding_map);

  void HandleKeyPress(KeySym keysym, uint mods, WindowManager* window_manager);

  Display* display_;
  int screen_num_;

  ::Window root_;

  bool initialized_;

  typedef map< ::Window, ref_ptr<XWindow> > XWindowMap;
  XWindowMap windows_;

  map<string, XFontStruct* > fonts_;

  GC default_gc_;
  map<string, GC> gcs_;

  XKeyBindingMap bindings_;
  XKeyBinding* in_progress_binding_;

  DISALLOW_EVIL_CONSTRUCTORS(XServer);
};

}  // namespace wham

#endif
