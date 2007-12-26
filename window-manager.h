// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_MANAGER_H__
#define __WINDOW_MANAGER_H__

#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"
#include "window.h"
#include "window-anchor.h"
#include "window-classifier.h"

using namespace std;

namespace wham {

class WindowManager {
 public:
  WindowManager();

  void AddWindow(XWindow* x_window);
  void RemoveWindow(XWindow* x_window);

 private:
  WindowClassifier window_classifier_;

  typedef map<XWindow*, ref_ptr<Window> > WindowMap;
  WindowMap windows_;

  typedef vector<ref_ptr<WindowAnchor> > WindowAnchorVector;
  WindowAnchorVector anchors_;

  size_t active_anchor_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowManager);
};

}  // namespace wham

#endif
