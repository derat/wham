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

  void CreateAnchor(const string& name, int x, int y);

  void HandleButtonPress(XWindow* x_window, int x, int y);
  void HandleButtonRelease(XWindow* x_window);
  void HandleCreateWindow(XWindow* x_window);
  void HandleDestroyWindow(XWindow* x_window);
  void HandleMotion(XWindow* x_window, int x, int y);

 private:
  WindowClassifier window_classifier_;

  typedef map<XWindow*, ref_ptr<Window> > WindowMap;
  WindowMap windows_;

  typedef map<XWindow*, WindowAnchor*> WindowAnchorTitlebarMap;
  WindowAnchorTitlebarMap anchor_titlebars_;

  typedef vector<ref_ptr<WindowAnchor> > WindowAnchorVector;
  WindowAnchorVector anchors_;

  size_t active_anchor_;
  bool in_drag_;
  int drag_offset_x_;
  int drag_offset_y_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowManager);
};

}  // namespace wham

#endif
