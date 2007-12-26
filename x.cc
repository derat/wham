// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "x.h"

#include <iostream>

#include "util.h"
#include "window-classifier.h"
#include "window-manager.h"

using namespace std;

namespace wham {

XServer* XWindow::server_ = NULL;


static const char* XEventTypeToName(int type) {
  switch (type) {
    case KeyPress: return "KeyPress";
    case KeyRelease: return "KeyRelease";
    case ButtonPress: return "ButtonPress";
    case ButtonRelease: return "ButtonRelease";
    case MotionNotify: return "MotionNotify";
    case EnterNotify: return "EnterNotify";
    case LeaveNotify: return "LeaveNotify";
    case FocusIn: return "FocusIn";
    case FocusOut: return "FocusOut";
    case KeymapNotify: return "KeymapNotify";
    case Expose: return "Expose";
    case GraphicsExpose: return "GraphicsExpose";
    case NoExpose: return "NoExpose";
    case VisibilityNotify: return "VisibilityNotify";
    case CreateNotify: return "CreateNotify";
    case DestroyNotify: return "DestroyNotify";
    case UnmapNotify: return "UnmapNotify";
    case MapNotify: return "MapNotify";
    case MapRequest: return "MapRequest";
    case ReparentNotify: return "ReparentNotify";
    case ConfigureNotify: return "ConfigureNotify";
    case ConfigureRequest: return "ConfigureRequest";
    case GravityNotify: return "GravityNotify";
    case ResizeRequest: return "ResizeRequest";
    case CirculateNotify: return "CirculateNotify";
    case CirculateRequest: return "CirculateRequest";
    case PropertyNotify: return "PropertyNotify";
    case SelectionClear: return "SelectionClear";
    case SelectionRequest: return "SelectionRequest";
    case SelectionNotify: return "SelectionNotify";
    case ColormapNotify: return "ColormapNotify";
    case ClientMessage: return "ClientMessage";
    case MappingNotify: return "MappingNotify";
    default: return "Unknown event";
  }
}


bool XWindow::GetProperties(WindowProperties* props) {
  char* window_name = NULL;
  if (!XFetchName(server_->display(), id_, &window_name)) {
    ERROR << "XFetchName() failed for 0x" << hex << id_;
    return false;
  }
  props->window_name = window_name ? window_name : "";
  if (window_name) XFree(window_name);

  char* icon_name = NULL;
  if (!XGetIconName(server_->display(), id_, &icon_name)) {
    ERROR << "XGetIconName() failed for 0x" << hex << id_;
    return false;
  }
  props->icon_name = icon_name ? icon_name : "";
  if (icon_name) XFree(icon_name);

  // FIXME: get command with XGetCommand()

  XClassHint class_hint;
  if (!XGetClassHint(server_->display(), id_, &class_hint)) {
    ERROR << "XGetClassHint() failed for 0x" << hex << id_;
    return false;
  }
  props->app_name = class_hint.res_name ? class_hint.res_name : "";
  props->app_class = class_hint.res_class ? class_hint.res_class : "";
  if (class_hint.res_name) XFree(class_hint.res_name);
  if (class_hint.res_class) XFree(class_hint.res_class);

  //LOG << " props: window_name=" << props->window_name
  //    << " icon_name=" << props->icon_name
  //    << " command=" << props->command
  //    << " app_name=" << props->app_name
  //    << " app_class=" << props->app_class;

  return true;
}


bool XWindow::Resize(int width, int height) {
  return XResizeWindow(server_->display(), id_, width, height) == Success;
}


XServer::XServer()
    : display_(NULL),
      screen_num_(-1),
      initialized_(false) {
  XWindow::server_ = this;
}


bool XServer::Init() {
  CHECK(!initialized_);

  display_ = XOpenDisplay(NULL);
  if (display_ == NULL) {
    LOG << "Can't open display " << XDisplayName(NULL);
    return 1;
  }
  screen_num_ = DefaultScreen(display_);

  XSelectInput(display_,
               RootWindow(display_, screen_num_),
               SubstructureNotifyMask);

  initialized_ = true;
  XWindow::server_ = this;
  return true;
}


void XServer::RunEventLoop(WindowManager* window_manager) {
  CHECK(window_manager);

  XEvent event;
  while (true) {
    XNextEvent(display_, &event);
    LOG << XEventTypeToName(event.type);
    switch (event.type) {
      case CreateNotify:
        {
          XCreateWindowEvent& e = event.xcreatewindow;
          LOG << "  window=0x" << hex << e.window
              << " parent=0x" << e.parent << dec
              << " x=" << e.x << " y=" << e.y
              << " width=" << e.width << " height=" << e.height
              << " border=" << e.border_width
              << " override=" << e.override_redirect;
          XWindow* x_window = GetWindow(e.window, true);
          window_manager->AddWindow(x_window);
        }
        break;
      case DestroyNotify:
        {
          XDestroyWindowEvent& e = event.xdestroywindow;
          LOG << "  window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          CHECK(x_window);
          window_manager->RemoveWindow(x_window);
        }
        break;
      case ConfigureNotify:
        {
          XConfigureEvent& e = event.xconfigure;
          LOG << "  window=0x" << hex << e.window << dec
              << " x=" << e.x << " y=" << e.y
              << " width=" << e.width << " height=" << e.height
              << " border=" << e.border_width
              << " above=" << static_cast<int>(e.above)
              << " override=" << e.override_redirect;
        }
        break;
    }
  }
}


XWindow* XServer::GetWindow(::Window id, bool create) {
  XWindowMap::iterator it = windows_.find(id);
  if (it != windows_.end()) return it->second.get();
  if (!create) return NULL;
  ref_ptr<XWindow> window(new XWindow(id));
  windows_.insert(make_pair(id, window));
  return window.get();
}

}  // namespace wham
