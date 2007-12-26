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

  static XWindow* Create(int x, int y, uint width, uint height);

  static void GetTextSize(const string& font, const string& text,
                          int* width, int* ascent, int* descent);
  void Clear();
  void DrawText(int x, int y, const string& text);
  void DrawLine(int x1, int y1, int x2, int y2);

  bool GetProperties(WindowProperties* props);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();

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
  // FIXME: free fonts_ and gc_ in d'tor

  bool Init();
  void RunEventLoop(WindowManager* window_manager);

  Display* display() { return display_; }
  int screen_num() { return screen_num_; }
  GC gc() { return gc_; }

  XWindow* GetWindow(::Window id, bool create);

  XFontStruct* GetFontInfo(const string& font);

 private:
  Display* display_;
  int screen_num_;

  bool initialized_;

  typedef map< ::Window, ref_ptr<XWindow> > XWindowMap;
  XWindowMap windows_;

  map<string, XFontStruct* > fonts_;

  GC gc_;

  DISALLOW_EVIL_CONSTRUCTORS(XServer);
};

}  // namespace wham

#endif
