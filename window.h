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

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();

  string title() const { return props_.window_name; }
  int title_width() const { return title_width_; }

  uint width() const { return width_; }
  uint height() const { return height_; }

  XWindow* x_window() const { return x_window_; }

  static void SetClassifier(WindowClassifier* classifier) {
    classifier_ = classifier;
  }

 private:
  bool Classify();

  void ApplyConfig();

  // Update 'props_' with this window's properties.
  bool UpdateProperties();

  static WindowClassifier* classifier_;

  // A pointer to information about the X window; used for interacting with
  // the server
  XWindow* x_window_;  // not owned

  uint width_;
  uint height_;

  WindowProperties props_;

  // Information about the size of the window's title
  int title_width_;

  WindowConfigSet configs_;

  DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}  // namespace wham

#endif
