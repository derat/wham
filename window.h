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

  bool Classify(const WindowClassifier& classifier);

  const WindowConfig* GetCurrentConfig() const {
    return configs_.GetCurrentConfig();
  }

 private:
  bool ApplyConfig();

  bool Resize(int width, int height);

  bool UpdateProperties();

  XWindow* x_window_;  // not owned

  WindowProperties props_;

  WindowConfigSet configs_;

  DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}  // namespace wham

#endif
