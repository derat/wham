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

  bool LoadConfig(const string& filename);

  void HandleButtonPress(XWindow* xwin, int x, int y, uint button);
  void HandleButtonRelease(XWindow* xwin, int x, int y, uint button);
  void HandleEnterWindow(XWindow* xwin);
  void HandleExposeWindow(XWindow* xwin);
  void HandleMapRequest(XWindow* xwin);
  void HandleMotion(XWindow* xwin, int x, int y);
  void HandlePropertyChange(XWindow* xwin,
                            WindowProperties::ChangeType type);
  void HandleUnmapWindow(XWindow* xwin);
  void HandleCommand(const Command& cmd);

 private:
  friend class ::WindowManagerTestSuite;

  // Create a new desktop after the active one in 'desktops_'.
  // Don't switch to it automatically.
  Desktop* CreateDesktop();

  // Get the index of 'desktop' within 'desktops_'.
  // Returns -1 if it's not present.
  int GetDesktopIndex(Desktop* desktop);

  // Set the passed-in desktop to be active.
  void SetActiveDesktop(Desktop* desktop);

  // Attached currently-tagged windows to 'anchor' and untag them.
  void AttachTaggedWindows(Anchor* anchor);

  // Toggle the tagged state of a window.
  void ToggleWindowTag(Window* window);

  // Set the passed-in anchor as active.
  void SetActiveAnchor(Anchor* anchor);

  // Check if the passed-in X window is an anchor titlebar or not.
  bool IsAnchorWindow(XWindow* xwin) const;

  // Execute the passed-in command.
  bool Exec(const string& command) const;

  // Get the window for which 'transient' is a transient.
  // Returns NULL if no transient-for window is set, or if the
  // transient-for window doesn't exist.
  Window* GetTransientFor(Window* transient) const;

  // Handle 'transient' declaring itself as a transient window for 'win'.
  // This involves finding the desktop (all desktops?) where 'win' exists
  // and adding it there.
  void HandleTransientFor(Window* transient, Window* win);

  // Get the active window from the focused anchor on the active desktop,
  // or NULL if none exists.
  Window* GetActiveWindow() const;

  // Get the window to which a frame belongs, or NULL if it's not a
  // frame.
  Window* GetWindowByFrame(XWindow* xwin) const;

  // Add 'window' to 'desktop'.  If 'anchor' is non-NULL, we will use that
  // anchor; otherwise, we'll use the regular logic for deciding which
  // anchor should be used.
  void AddWindowToDesktop(Window* window, Desktop* desktop, Anchor* anchor);

  // Remove 'window' from 'desktop'.
  void RemoveWindowFromDesktop(Window* window, Desktop* desktop);

  // Should a window currently be mapped onscreen?
  bool WindowShouldBeMapped(Window* window) const;

  // Map from client windows to Window objects.
  typedef map<XWindow*, ref_ptr<Window> > WindowMap;
  WindowMap windows_;

  // Map from each window frame to the window owning it.
  typedef map<const XWindow*, Window*> FrameMap;
  FrameMap frames_;

  // Map from a window to all desktops where it's present.
  map<Window*, set<Desktop*> > window_desktops_;

  // All desktops.
  typedef vector<ref_ptr<Desktop> > DesktopVector;
  DesktopVector desktops_;

  // The desktop that's currently being viewed.
  Desktop* active_desktop_;

  set<Window*> tagged_windows_;

  // Do we attach new windows to the currently-focused anchor?
  bool attach_follows_active_;

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
