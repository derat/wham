// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __X_WINDOW_H__
#define __X_WINDOW_H__

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>
}

#include "util.h"
#include "window-properties.h"

using namespace std;

namespace wham {

class WindowProperties;
class XServer;

class XWindow {
 public:
  XWindow(::Window id);
  virtual ~XWindow() {}

  static XWindow* Create(int x, int y, uint width, uint height);

  ::Window id() const { return id_; }

  // Update 'props' with this window's current properties of type 'type'.
  virtual bool UpdateProperties(WindowProperties* props,
                                WindowProperties::ChangeType type);

  virtual void Move(int x, int y);
  virtual void Resize(uint width, uint height);
  virtual void Unmap();
  virtual void Map();
  virtual void SelectClientEvents();
  virtual void TakeFocus();
  virtual void SetBorder(uint size);
  virtual void Raise();
  virtual void MakeSibling(const XWindow& leader);
  virtual void Reparent(XWindow* parent, int x, int y);
  virtual void WarpPointer(int x, int y);
  // FIXME: change this to UpdateGeometry() and just update in-object vals
  virtual void GetGeometry(int* x,
                           int* y,
                           uint* width,
                           uint* height,
                           uint* border_width);
  virtual void Destroy();

  virtual bool operator<(const XWindow& o) const {
    return id_ < o.id_;
  }

  XWindow* parent() const { return parent_; }

  int x() const { return x_; }
  int y() const { return y_; }
  uint width() const { return width_; }
  uint height() const { return height_; }

  int left() const { return x_; }
  int right() const { return x_ + width_; }
  int top() const { return y_; }
  int bottom() const { return y_ + height_; }

  int initial_x() const { return initial_x_; }
  int initial_y() const { return initial_y_; }
  uint initial_width() const { return initial_width_; }
  uint initial_height() const { return initial_height_; }

 protected:
  int x_;
  int y_;
  uint width_;
  uint height_;

  int initial_x_;
  int initial_y_;
  uint initial_width_;
  uint initial_height_;

  XWindow* parent_;

 private:
  // Convenience methods.
  static xcb_connection_t* xcb_conn();
  static const xcb_screen_t* xcb_screen();
  static ::Display* dpy();
  static int scr();
  static ::Window root();

  // Get the window for which this window is a transient.
  // Returns NULL if no such window exists.
  virtual XWindow* GetTransientFor();

  void SelectInput(uint mask);

  ::Window id_;

  uint input_mask_;

  DISALLOW_EVIL_CONSTRUCTORS(XWindow);
};

}  // namespace wham

#endif
