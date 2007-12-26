// Copyright 2007, Daniel Erat <dan@erat.org>
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

  bool Classify(const WindowClassifier& classifier);

  const WindowConfig* GetActiveConfig() const {
    return configs_.GetActiveConfig();
  }

  bool Move(int x, int y);
  bool Resize(uint width, uint height);
  bool Unmap();
  bool Map();

  string title() const { return props_.window_name; }
  int title_width() const { return title_width_; }
  int title_height() const { return title_ascent_ + title_descent_; }
  int title_ascent() const { return title_ascent_; }
  int title_descent() const { return title_descent_; }

 private:
  bool ApplyConfig();

  // Update 'props_' with this window's properties.
  bool UpdateProperties();

  // A pointer to information about the X window; used for interacting with
  // the server
  XWindow* x_window_;  // not owned

  WindowProperties props_;

  // Information about the size of the window's title
  int title_width_;
  int title_ascent_;
  int title_descent_;

  WindowConfigSet configs_;

  DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}  // namespace wham

#endif
