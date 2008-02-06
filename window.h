// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "util.h"
#include "window-classifier.h"
#include "window-properties.h"

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
  void Raise();
  void MakeSibling(const XWindow& leader);
  void HandlePropertyChange(WindowProperties::ChangeType type);

  string title() const { return props_.window_name; }

  int x() const;
  int y() const;
  uint width() const;
  uint height() const;

  bool tagged() const { return tagged_; }
  void set_tagged(bool tagged) { tagged_ = tagged; }

  XWindow* x_window() const { return x_window_; }

 private:
  bool Classify();

  void ApplyConfig();

  // Update 'props_' with this window's properties.
  bool UpdateProperties(WindowProperties::ChangeType type, bool* changed);

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
