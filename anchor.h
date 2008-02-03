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
class XWindow;  // from x.h

// A collection of windows, exactly one of which is visible at any given
// time.
class Anchor {
 public:
  const static int kTitlebarHeight;

  Anchor(const string& name, int x, int y);
  ~Anchor();

  // Where an anchor should appear in relation to its windows.
  enum Gravity {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_RIGHT,
    BOTTOM_LEFT,
    NUM_GRAVITIES,
  };

  string name() const { return name_; }
  int x() const { return x_; }
  int y() const { return y_; }
  XWindow* titlebar() { return titlebar_; }
  Gravity gravity() const { return gravity_; }
  const vector<Window*>& windows() const { return windows_; }
  const Window* active_window() const { return active_window_; }
  Window* mutable_active_window() { return active_window_; }

  // Set the anchor's name.
  void SetName(const string& name);

  // Add a window to the anchor.
  // If no other windows are present, the new window will be made active.
  void AddWindow(Window* window);

  // Remove a window from the anchor.
  void RemoveWindow(Window* window);

  // Move the anchor to a new position.
  void Move(int x, int y);

  // Raise this anchor to the top of the stacking order.
  void Raise();

  // Set which (zero-indexed) window should be currently displayed.
  bool SetActive(uint index);

  void DrawTitlebar();

  void ActivateWindowAtTitlebarCoordinates(int x, int y);

  void FocusActiveWindow();

  void CycleActiveWindowConfig(bool forward);

  // Change the anchor's gravity.
  void SetGravity(Gravity gravity);
  void CycleGravity(bool forward);

 private:
  friend class ::AnchorTestSuite;

  void UpdateTitlebarPosition();
  void UpdateWindowPosition(Window* window);

  string name_;

  int x_;
  int y_;

  typedef vector<Window*> WindowVector;
  WindowVector windows_;  // not owned

  // Index into 'windows_' of the currently-active window, and a pointer to
  // the window itself
  uint active_index_;
  Window* active_window_;

  // Where an anchor should appear in relation to its windows.
  Gravity gravity_;

  XWindow* titlebar_;

  DISALLOW_EVIL_CONSTRUCTORS(Anchor);
};

}  // namespace wham

#endif
