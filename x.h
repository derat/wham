// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __X_H__
#define __X_H__

#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"

using namespace std;

namespace wham {

class WindowManager;
class WindowProperties;
class XServer;

class XWindow {
 public:
  XWindow(::Window id)
      : id_(id) {
  }

  bool GetProperties(WindowProperties* props);

  bool Move(int x, int y);
  bool Resize(unsigned int width, unsigned int height);
  bool Unmap();
  bool Map();

  bool operator<(const XWindow& o) const {
    return id_ < o.id_;
  }

 private:
  friend class XServer;

  ::Window id_;

  static XServer* server_;
};


class XServer {
 public:
  XServer();

  bool Init();
  void RunEventLoop(WindowManager* window_manager);

  Display* display() { return display_; }
  int screen_num() { return screen_num_; }

 private:
  XWindow* GetWindow(::Window id, bool create);

  Display* display_;
  int screen_num_;

  bool initialized_;

  typedef map< ::Window, ref_ptr<XWindow> > XWindowMap;
  XWindowMap windows_;

  DISALLOW_EVIL_CONSTRUCTORS(XServer);
};

}  // namespace wham

#endif
