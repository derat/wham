// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_MANAGER_H__
#define __WINDOW_MANAGER_H__

#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "key-bindings.h"
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
  void HandleButtonRelease(XWindow* x_window, int x, int y);
  void HandleCreateWindow(XWindow* x_window);
  void HandleDestroyWindow(XWindow* x_window);
  void HandleExposeWindow(XWindow* x_window);
  void HandleMotion(XWindow* x_window, int x, int y);

  bool Exec(const string& command);

  // TODO: maybe move Command to this file
  void HandleCommand(KeyBindings::Command cmd);

 private:
  WindowClassifier window_classifier_;

  typedef map<XWindow*, ref_ptr<Window> > WindowMap;
  WindowMap windows_;

  typedef map<XWindow*, WindowAnchor*> WindowAnchorTitlebarMap;
  WindowAnchorTitlebarMap anchor_titlebars_;

  typedef vector<ref_ptr<WindowAnchor> > WindowAnchorVector;
  WindowAnchorVector anchors_;

  typedef vector<WindowAnchor*> WindowAnchorPtrVector;
  map<Window*, WindowAnchorPtrVector> windows_to_anchors_;

  size_t active_anchor_;

  // Is a mouse button currently down?
  bool mouse_down_;

  // Are we dragging?
  bool dragging_;

  // Holds the offset between the anchor that's currently being dragged and
  // the pointer's position when the drag started.
  int drag_offset_x_;
  int drag_offset_y_;

  // Holds the pointer's position when a mouse button was last pressed
  int mouse_down_x_;
  int mouse_down_y_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowManager);
};

}  // namespace wham

#endif
