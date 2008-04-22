// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __ANCHOR_H__
#define __ANCHOR_H__

#include <vector>

#include "util.h"

using namespace std;

class AnchorTestSuite;

namespace wham {

class Window;   // from window.h
class XWindow;  // from x-window.h

// A collection of windows, exactly one of which is visible at any given
// time.
class Anchor {
 public:
  const static int kTitlebarHeight;

  // Where an anchor should appear in relation to its windows.
  enum Gravity {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_RIGHT,
    BOTTOM_LEFT,
    NUM_GRAVITIES,
  };

  Anchor(const string& name, int x, int y, Gravity gravity=TOP_LEFT);
  ~Anchor();

  string name() const { return name_; }
  int x() const { return x_; }
  int y() const { return y_; }
  bool temporary() const { return temporary_; }
  void set_temporary(bool temporary) { temporary_ = temporary; }
  XWindow* titlebar() { return titlebar_; }
  Gravity gravity() const { return gravity_; }
  const vector<Window*>& windows() const { return windows_; }
  const Window* active_window() const { return active_window_; }
  Window* mutable_active_window() { return active_window_; }
  bool active() const { return active_; }
  bool attach() const { return attach_; }

  // Hide or show this anchor.
  void Hide();
  void Show();

  // Set the anchor's name.
  void SetName(const string& name);

  // Add a window to the anchor.
  // If no other windows are present, the new window will be made active.
  void AddWindow(Window* window);

  // Remove a window from the anchor.
  void RemoveWindow(Window* window);

  // Move the anchor to a new position.
  // The anchor will be constrained within the root window's dimensions.
  void Move(int x, int y);

  // Raise this anchor to the top of the stacking order.
  void Raise();

  // Set the anchor to be active or non-active, redrawing the titlebar if
  // necessary.
  void SetActive(bool active);

  // Set the anchor as the one to which new windows will be attached,
  // redrawing the titlebar if necessary.
  void SetAttach(bool attach);

  // Set which (zero-indexed) window should be currently displayed.
  bool SetActiveWindow(uint index);

  // Instruct the drawing engine to draw this anchor's titlebar.
  void DrawTitlebar();

  // Get the index number of the window represented in the titlebar at the
  // given absolute (that is, not relative to the titlebar's position) X
  // value.  Constrains too-small or -large values, and returns -1 if no
  // windows are present in the anchor.
  int GetWindowIndexAtTitlebarPoint(int abs_x);

  // If there is an active window, give the focus to it.
  void FocusActiveWindow();

  // Cycle the config of the active window, updating the window's position
  // onscreen if necessary.
  void CycleActiveWindowConfig(bool forward);

  // Change the anchor's gravity.
  // The anchor's position is shifted such that the position of the
  // titlebar remains the same.
  void SetGravity(Gravity gravity);

  // Move the anchor's gravity forward or backward in the progression (top
  // left, top right, bottom right, bottom left).
  void CycleGravity(bool forward);

  // Does the titlebar covers the passed-in point?
  bool TitlebarIsOverPoint(int x, int y) const;

  static void GetGravityDirection(Gravity gravity, int* dx, int* dy);

 private:
  friend class ::AnchorTestSuite;

  // Move the titlebar window to the appropriate position, given the
  // position of the anchor and its gravity.
  void UpdateTitlebarPosition();

  // Move 'window' to the appropriate position.
  void UpdateWindowPosition(Window* window);

  // Get the position of the titlebar window, given the anchor's current
  // position and its gravity.
  void GetTitlebarPosition(int* x, int* y);

  // The anchor's name.
  string name_;

  // The anchor's position on the screen.  This is the top-left point of
  // the titlebar if gravity_ is TOP_LEFT, the bottom-right point of the
  // titlebar for BOTTOM_RIGHt, and so on.
  int x_;
  int y_;

  // Should this anchor be destroyed when it doesn't contain any windows?
  bool temporary_;

  // Pointers to windows stored within this anchor
  typedef vector<Window*> WindowVector;
  WindowVector windows_;  // not owned

  // Index into 'windows_' of the currently-active window, and a pointer to
  // the window itself
  uint active_index_;
  Window* active_window_;

  // Where an anchor should appear in relation to its windows.
  Gravity gravity_;

  // Titlebar window; not owned.
  XWindow* titlebar_;

  // Is this anchor active?
  bool active_;

  // Do new windows get attached to this anchor?
  bool attach_;

  DISALLOW_EVIL_CONSTRUCTORS(Anchor);
};

}  // namespace wham

#endif
