// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "util.h"
#include "window-classifier.h"

using namespace std;

namespace wham {

class XWindow;

class Window {
 public:
  Window(XWindow* x_window);
  ~Window() {
    x_window_ = NULL;
  }

  void CycleConfig(bool forward);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();
  void TakeFocus();

  string title() const { return props_.window_name; }

  uint width() const { return width_; }
  uint height() const { return height_; }

  bool tagged() const { return tagged_; }
  void set_tagged(bool tagged) { tagged_ = tagged; }

  XWindow* x_window() const { return x_window_; }

 private:
  bool Classify();

  void ApplyConfig();

  // Update 'props_' with this window's properties.
  bool UpdateProperties();

  // A pointer to information about the X window; used for interacting with
  // the server
  XWindow* x_window_;  // not owned

  uint width_;
  uint height_;

  WindowProperties props_;

  WindowConfigSet configs_;

  bool tagged_;

  DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}  // namespace wham

#endif
