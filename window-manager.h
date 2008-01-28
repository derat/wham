// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_MANAGER_H__
#define __WINDOW_MANAGER_H__

#include <map>
#include <set>

#include "anchor.h"
#include "command.h"
#include "desktop.h"
#include "key-bindings.h"
#include "util.h"
#include "window.h"
#include "window-classifier.h"

using namespace std;

class WindowManagerTestSuite;

namespace wham {

class XWindow;

class WindowManager {
 public:
  WindowManager();
  void SetupDefaultCrap();

  void HandleButtonPress(XWindow* x_window, int x, int y);
  void HandleButtonRelease(XWindow* x_window, int x, int y);
  void HandleCreateWindow(XWindow* x_window);
  void HandleDestroyWindow(XWindow* x_window);
  void HandleEnterWindow(XWindow* x_window);
  void HandleExposeWindow(XWindow* x_window);
  void HandleMotion(XWindow* x_window, int x, int y);

  void HandleCommand(const Command& cmd);

 private:
  friend class ::WindowManagerTestSuite;

  // Create a new desktop and switch to it.
  Desktop* CreateDesktop();

  // Attached currently-tagged windows to 'anchor' and untag them.
  void AttachTaggedWindows(Anchor* anchor);

  // Toggle the tagged state of a window.
  void ToggleWindowTag(Window* window);

  // Check if the passed-in X window is an anchor titlebar or not.
  bool IsAnchorWindow(XWindow* x_window) const;

  bool Exec(const string& command) const;

  // Get the active window from the focused anchor on the active desktop,
  // or NULL if none exists.
  Window* GetActiveWindow() const;

  // Map from client windows to Window objects.
  typedef map<XWindow*, ref_ptr<Window> > WindowMap;
  WindowMap windows_;

  typedef vector<ref_ptr<Desktop> > DesktopVector;
  DesktopVector desktops_;

  Desktop* active_desktop_;

  set<Window*> tagged_windows_;

  // Is a mouse button currently down?
  bool mouse_down_;

  // Are we dragging?
  bool dragging_;

  // Holds the offset between the anchor that's currently being dragged and
  // the pointer's position when the drag started.
  int drag_offset_x_;
  int drag_offset_y_;

  // Holds the pointer's position when a mouse button was last pressed
  int mouse_down_x_;
  int mouse_down_y_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowManager);
};

}  // namespace wham

#endif
