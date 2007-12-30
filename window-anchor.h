// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_ANCHOR_H__
#define __WINDOW_ANCHOR_H__

#include <vector>

#include "util.h"

using namespace std;

namespace wham {

class Window;   // from window.h
class XWindow;  // from x.h

// A collection of windows, exactly one of which is visible at any given
// time.
class WindowAnchor {
 public:
  const static int kTitlebarHeight;

  WindowAnchor(const string& name, int x, int y);
  ~WindowAnchor();

  string name() const { return name_; }
  int x() const { return x_; }
  int y() const { return y_; }
  XWindow* titlebar() { return titlebar_; }

  void SetName(const string& name);

  // Add a window to the anchor.
  void AddWindow(Window* window);

  // Remove a window from the anchor.
  void RemoveWindow(Window* window);

  // Move the anchor to a new position.
  bool Move(int x, int y);

  // Set which (zero-indexed) window should be currently displayed.
  // 'index' must be >= 0 and less than the number of windows in the
  // anchor.
  bool SetActive(uint index);

  uint NumWindows() const { return windows_.size(); }

  void DrawTitlebar();

 private:
  string name_;

  int x_;
  int y_;

  typedef vector<Window*> WindowVector;
  WindowVector windows_;  // not owned

  // Index into 'windows_' of the currently-active window, and a pointer to
  // the window itself
  uint active_index_;
  Window* active_window_;

  XWindow* titlebar_;
  int titlebar_width_;
  int titlebar_height_;

  // FIXME: update these when the font changes
  static int font_ascent_;
  static int font_descent_;

  // Information about the size of the anchor's name
  int name_width_;
  int name_ascent_;
  int name_descent_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowAnchor);
};

}  // namespace wham

#endif
