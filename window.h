// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"

using namespace std;

namespace wham {

class Window {
 public:
  Window(::Window x_window)
      : x_window_(x_window) {
  }

 private:
  ::Window x_window_;

 DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}  // namespace wham

#endif
