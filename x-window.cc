// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "x-window.h"

#include <iostream>
#include <X11/Xatom.h>

#include "mock-x-window.h"
#include "util.h"
#include "x-server.h"

using namespace std;

namespace wham {

// X input mask for client windows that we're managing.
static const uint kClientInputMask =
    EnterWindowMask | PropertyChangeMask | StructureNotifyMask;

// X input mask for windows that are created via the Create() method.
static const uint kCreateInputMask =
    ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
    ExposureMask | PointerMotionMask;


XWindow::XWindow(::Window id)
    : parent_(NULL),
      id_(id),
      input_mask_(0) {
  if (!XServer::Testing()) {
    GetGeometry(&x_, &y_, &width_, &height_, NULL);
    initial_x_ = x_;
    initial_y_ = y_;
    initial_width_ = width_;
    initial_height_ = height_;
  }
}


XWindow* XWindow::Create(int x, int y, uint width, uint height) {
  xcb_window_t id;
  if (XServer::Testing()) {
    static int win_id = 1;
    id = win_id++;
  } else {
    id = xcb_generate_id(xcb_conn());
    xcb_create_window(xcb_conn(),
                      XCB_COPY_FROM_PARENT,  // depth
                      id,
                      xcb_screen()->root,
                      x, y,
                      width, height,
                      0,  // border_width
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      xcb_screen()->root_visual,
                      0, NULL);
  }
  DEBUG << "Created window 0x" << hex << id;
  XWindow* win = XServer::Get()->GetWindow(id, true);
  if (XServer::Testing()) {
    // FIXME: This is super-ugly; change it.
    win->x_ = win->initial_x_ = x;
    win->y_ = win->initial_y_ = y;
    win->width_ = win->initial_width_ = width;
    win->height_ = win->initial_height_ = height;
  } else {
    win->SelectInput(kCreateInputMask);
  }
  return win;
}


bool XWindow::UpdateProperties(WindowProperties* props,
                               WindowProperties::ChangeType type) {
  CHECK(props);

  if (type == WindowProperties::WINDOW_NAME_CHANGE) {
    char* window_name = NULL;
    if (!XFetchName(dpy(), id_, &window_name)) {
      ERROR << "XFetchName() failed for 0x" << hex << id_;
      return false;
    }
    props->window_name = window_name ? window_name : "";
    if (window_name) XFree(window_name);
  } else if (type == WindowProperties::ICON_NAME_CHANGE) {
    char* icon_name = NULL;
    if (!XGetIconName(dpy(), id_, &icon_name)) {
      ERROR << "XGetIconName() failed for 0x" << hex << id_;
      return false;
    }
    props->icon_name = icon_name ? icon_name : "";
    if (icon_name) XFree(icon_name);
  } else if (type == WindowProperties::COMMAND_CHANGE) {
    char **argv = NULL;
    int argc = 0;
    if (!XGetCommand(dpy(), id_, &argv, &argc)) {
      ERROR << "XGetCommand() failed for 0x" << hex << id_;
      return false;
    }
    // FIXME: update command
    XFreeStringList(argv);
  } else if (type == WindowProperties::CLASS_CHANGE) {
    XClassHint class_hint;
    if (!XGetClassHint(dpy(), id_, &class_hint)) {
      ERROR << "XGetClassHint() failed for 0x" << hex << id_;
      return false;
    }
    props->app_name = class_hint.res_name ? class_hint.res_name : "";
    props->app_class = class_hint.res_class ? class_hint.res_class : "";
    if (class_hint.res_name) XFree(class_hint.res_name);
    if (class_hint.res_class) XFree(class_hint.res_class);
  } else if (type == WindowProperties::WM_HINTS_CHANGE) {
    XSizeHints* size_hints = XAllocSizeHints();
    CHECK(size_hints);
    long supplied_hints = 0;
    if (!XGetWMNormalHints(dpy(), id_, size_hints, &supplied_hints)) {
      ERROR << "XGetWMNormalHints() failed for 0x" << hex << id_;
      XFree(size_hints);
      return false;
    }
    if (size_hints->flags & USPosition || size_hints->flags & PPosition) {
      props->x = size_hints->x;
      props->y = size_hints->y;
    }
    if (size_hints->flags & USSize || size_hints->flags & PSize) {
      props->width = static_cast<uint>(size_hints->width);
      props->height = static_cast<uint>(size_hints->height);
    }
    if (size_hints->flags & PMinSize) {
      props->min_width = static_cast<uint>(size_hints->min_width);
      props->min_height = static_cast<uint>(size_hints->min_height);
    }
    if (size_hints->flags & PMaxSize) {
      props->max_width = static_cast<uint>(size_hints->max_width);
      props->max_height = static_cast<uint>(size_hints->max_height);
    }
    if (size_hints->flags & PResizeInc) {
      props->width_inc = static_cast<uint>(size_hints->width_inc);
      props->height_inc = static_cast<uint>(size_hints->height_inc);
    }
    if (size_hints->flags & PAspect) {
      props->min_aspect = static_cast<float>(size_hints->min_aspect.x) /
                          size_hints->min_aspect.y;
      props->max_aspect = static_cast<float>(size_hints->max_aspect.x) /
                          size_hints->max_aspect.y;
    }
    if (size_hints->flags & PBaseSize) {
      props->base_width = static_cast<uint>(size_hints->base_width);
      props->base_height = static_cast<uint>(size_hints->base_height);
    }
    XFree(size_hints);
  } else if (type == WindowProperties::TRANSIENT_CHANGE) {
    props->transient_for = GetTransientFor();
  } else {
    ERROR << "Unable to handle property change of type "
          << WindowProperties::ChangeTypeToStr(type);
    CHECK(false);
  }

  return true;
}


void XWindow::Move(int x, int y) {
  if (x == x_ && y == y_) return;
  x_ = x;
  y_ = y;
  XMoveWindow(dpy(), id_, x, y);
}


void XWindow::Resize(uint width, uint height) {
  if (width == width_ && height == height_) return;
  width_ = width;
  height_ = height;
  XResizeWindow(dpy(), id_, width, height);
}


void XWindow::Unmap() {
  // Cribbed from blackbox.  When we unmap a window ourselves, we don't
  // want to get notification about it, so grab the server and unselect
  // structure events.
  XGrabServer(dpy());
  XSelectInput(dpy(), id_, input_mask_ & ~StructureNotifyMask);
  XUnmapWindow(dpy(), id_);
  XSelectInput(dpy(), id_, input_mask_);
  XUngrabServer(dpy());
}


void XWindow::Map() {
  XMapWindow(dpy(), id_);
}


void XWindow::SelectClientEvents() {
  SelectInput(kClientInputMask);
}


void XWindow::TakeFocus() {
  XSetInputFocus(dpy(), id_, RevertToPointerRoot, CurrentTime);
  // FIXME: debugging
  XSync(dpy(), False);
}


void XWindow::SetBorder(uint size) {
  XSetWindowBorderWidth(dpy(), id_, size);
}


void XWindow::Raise() {
  XRaiseWindow(dpy(), id_);
}


void XWindow::MakeSibling(const XWindow& leader) {
  DEBUG << "MakeSibling: id=0x" << hex << id_ << " leader=0x" << leader.id();
  XWindowChanges changes;
  changes.sibling = leader.id();
  changes.stack_mode = Below;
  XConfigureWindow(dpy(), id_, CWSibling | CWStackMode, &changes);
}


void XWindow::Reparent(XWindow* parent, int x, int y) {
  DEBUG << "Reparent: id=0x" << hex << id_ << " parent=0x" << parent->id();
  xcb_reparent_window(xcb_conn(), id_,
                      parent ? parent->id() : xcb_screen()->root,
                      x, y);
  parent_ = parent;
}


void XWindow::WarpPointer(int x, int y) {
  // FIXME: Figure out why this doesn't seem to be working.  Maybe one
  // can't warp the cursor under Xnest?
  DEBUG << "WarpPointer: id=0x" << hex << id_ << " x=" << x << " y=" << y;
  XWarpPointer(dpy(), None, id_, 0, 0, 0, 0, x, y);
}


void XWindow::GetGeometry(int* x,
                          int* y,
                          uint* width,
                          uint* height,
                          uint* border_width) {
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(xcb_conn(), id_);
  ref_ptr<xcb_get_geometry_reply_t> geometry(
      xcb_get_geometry_reply(xcb_conn(), cookie, NULL));
  if (x) *x = geometry->x;
  if (y) *y = geometry->y;
  if (width) *width = geometry->width;
  if (height) *height = geometry->height;
  if (border_width) *border_width = geometry->border_width;
}


void XWindow::Destroy() {
  DEBUG << "Destroy: id=0x" << hex << id_;
  XServer::Get()->DeleteWindow(id_);
  xcb_destroy_window(xcb_conn(), id_);
}


xcb_connection_t* XWindow::xcb_conn() { return XServer::Get()->xcb_conn(); }


const xcb_screen_t* XWindow::xcb_screen() {
  return XServer::Get()->xcb_screen();
}


::Display* XWindow::dpy() { return XServer::Get()->display(); }


int XWindow::scr() { return XServer::Get()->screen_num(); }


::Window XWindow::root() { return XServer::Get()->root(); }


XWindow* XWindow::GetTransientFor() {
  ::Window win_id;
  if (!XGetTransientForHint(dpy(), id_, &win_id)) {
    ERROR << "XGetTransientForHint() failed for 0x" << hex << id_;
    return NULL;
  }
  XWindow* win = XServer::Get()->GetWindow(win_id, false);
  if (win == NULL) {
    ERROR << hex << "0x" << id_ << " claims to be a transient for 0x"
          << win_id << ", which isn't registered";
  }
  return win;
}


void XWindow::SelectInput(uint mask) {
  XSelectInput(dpy(), id_, mask);
  input_mask_ = mask;
}

}  // namespace wham
