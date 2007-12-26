// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_MANAGER_H__
#define __WINDOW_MANAGER_H__

#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"
#include "window.h"
#include "window-classifier.h"

using namespace std;

namespace wham {

class WindowManager {
 public:
  WindowManager(::Display* display);

  void AddWindow(::Window x_window);

  void RemoveWindow(::Window x_window);

 private:
  bool GetWindowProperties(::Window x_window, WindowProperties* props);

  ::Display* display_;

  WindowClassifier window_classifier_;

  map< ::Window, ref_ptr<Window> > windows_;

 DISALLOW_EVIL_CONSTRUCTORS(WindowManager);
};

}  // namespace wham

#endif
