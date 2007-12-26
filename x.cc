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


XWindow* XWindow::Create(int x, int y, uint width, uint height) {
  ::Window win =
      XCreateSimpleWindow(server_->display(),
          RootWindow(server_->display(), server_->screen_num()),
          x, y, width, height, 0 /* border */,
          BlackPixel(server_->display(), server_->screen_num()),
          WhitePixel(server_->display(), server_->screen_num()));
  XSelectInput(server_->display(), win,
               ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
               ExposureMask);
  return server_->GetWindow(win, true);
}


void XWindow::GetTextSize(const string& font, const string& text,
                          int* width, int* ascent, int* descent) {
  XFontStruct* font_info = server_->GetFontInfo(font);
  int tmp_dir, tmp_ascent, tmp_descent;
  XCharStruct overall;
  XTextExtents(font_info, text.c_str(), text.size(),
               &tmp_dir, &tmp_ascent, &tmp_descent, &overall);
  if (width) *width = overall.width;
  if (ascent) *ascent = overall.ascent;
  if (descent) *descent = overall.descent;
}


void XWindow::Clear() {
  XClearWindow(server_->display(), id_);
}


void XWindow::DrawText(int x, int y, const string& text) {
  XDrawString(server_->display(), id_, server_->gc(), x, y,
              text.c_str(), text.size());
}


void XWindow::DrawLine(int x1, int y1, int x2, int y2) {
  XDrawLine(server_->display(), id_, server_->gc(), x1, y1, x2, y2);
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

  return true;
}


void XWindow::Move(int x, int y) {
  XMoveWindow(server_->display(), id_, x, y);
}


void XWindow::Resize(uint width, uint height) {
  XResizeWindow(server_->display(), id_, width, height);
}


void XWindow::Unmap() {
  XUnmapWindow(server_->display(), id_);
}


void XWindow::Map() {
  XMapWindow(server_->display(), id_);
}


XServer::XServer()
    : display_(NULL),
      screen_num_(-1),
      initialized_(false) {
}


bool XServer::Init() {
  CHECK(!initialized_);

  display_ = XOpenDisplay(NULL);
  if (display_ == NULL) {
    LOG << "Can't open display " << XDisplayName(NULL);
    return false;
  }
  screen_num_ = DefaultScreen(display_);

  gc_ = XCreateGC(display_, RootWindow(display_, screen_num_), 0, NULL);
  XSetForeground(display_, gc_, BlackPixel(display_, screen_num_));

  // debugging
  //XSynchronize(display_, True);

  XSelectInput(display_,
               RootWindow(display_, screen_num_),
               SubstructureNotifyMask);

  XWindow::server_ = this;
  initialized_ = true;
  return true;
}


void XServer::RunEventLoop(WindowManager* window_manager) {
  CHECK(window_manager);

  XEvent event;
  while (true) {
    XNextEvent(display_, &event);
    switch (event.type) {
      case ButtonPress:
        {
          XButtonEvent& e = event.xbutton;
          LOG << "ButtonPress: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          window_manager->HandleButtonPress(x_window, e.x_root, e.y_root);
        }
        break;
      case ButtonRelease:
        {
          XButtonEvent& e = event.xbutton;
          LOG << "ButtonRelease: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          window_manager->HandleButtonRelease(x_window);
        }
        break;
      case ConfigureNotify:
        {
          /*
          XConfigureEvent& e = event.xconfigure;
          LOG << "ConfigureNotify: window=0x" << hex << e.window << dec
              << " x=" << e.x << " y=" << e.y
              << " width=" << e.width << " height=" << e.height
              << " border=" << e.border_width
              << " above=" << static_cast<int>(e.above)
              << " override=" << e.override_redirect;
              */
        }
        break;
      case CreateNotify:
        {
          XCreateWindowEvent& e = event.xcreatewindow;
          LOG << "CreateNotify: window=0x" << hex << e.window
              << " parent=0x" << e.parent << dec
              << " x=" << e.x << " y=" << e.y
              << " width=" << e.width << " height=" << e.height
              << " border=" << e.border_width
              << " override=" << e.override_redirect;
          XWindow* x_window = GetWindow(e.window, true);
          window_manager->HandleCreateWindow(x_window);
        }
        break;
      case DestroyNotify:
        {
          XDestroyWindowEvent& e = event.xdestroywindow;
          LOG << "DestroyNotify: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          CHECK(x_window);
          window_manager->HandleDestroyWindow(x_window);
        }
        break;
      case Expose:
        {
          XExposeEvent& e = event.xexpose;
          //LOG << "Expose: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          CHECK(x_window);
          window_manager->HandleExposeWindow(x_window);
        }
        break;
      case MotionNotify:
        {
          XMotionEvent& e = event.xmotion;
          //LOG << "MotionNotify: window=0x" << hex << e.window << dec
          //    << " x=" << e.x_root << " y=" << e.y_root;
          XWindow* x_window = GetWindow(e.window, false);
          window_manager->HandleMotion(x_window, e.x_root, e.y_root);
        }
        break;
      default:
        LOG << XEventTypeToName(event.type);
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


XFontStruct* XServer::GetFontInfo(const string& font) {
  map<string, XFontStruct*>::const_iterator it = fonts_.find(font);
  if (it != fonts_.end()) return it->second;
  XFontStruct* font_info = XLoadQueryFont(display_, font.c_str());
  CHECK(font_info);
  fonts_.insert(make_pair(font, font_info));
  return font_info;
}

}  // namespace wham
