// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_PROPERTIES_H__
#define __WINDOW_PROPERTIES_H__

#include <string>

using namespace std;

namespace wham {

class XWindow;

// X-supplied information about a window.  This consists largely of window
// manager hints (window and icon name, etc.) and is used to classify the
// window, find transients, etc.
struct WindowProperties {
  // Different types of properties that can change in a client window.
  enum ChangeType {
    WINDOW_NAME_CHANGE,
    ICON_NAME_CHANGE,
    COMMAND_CHANGE,
    CLASS_CHANGE,
    WM_HINTS_CHANGE,
    TRANSIENT_CHANGE,
    OTHER_CHANGE,
  };

  static string ChangeTypeToStr(ChangeType type);

  WindowProperties()
      : window_name(""),
        icon_name(""),
        command(""),
        app_name(""),
        app_class(""),
        x(0),
        y(0),
        width(0),
        height(0),
        min_width(0),
        min_height(0),
        max_width(0),
        max_height(0),
        width_inc(0),
        height_inc(0),
        min_aspect(0),
        max_aspect(0),
        base_width(0),
        base_height(0),
        transient_for(NULL) {}

  string DebugString() const;

  bool operator==(const WindowProperties& o) {
    // Yuck.
    return window_name == o.window_name &&
           icon_name == o.icon_name &&
           command == o.command &&
           app_name == o.app_name &&
           app_class == o.app_class &&
           x == o.x &&
           y == o.y &&
           width == o.width &&
           height == o.height &&
           min_width == o.min_width &&
           min_height == o.min_height &&
           max_width == o.max_width &&
           max_height == o.max_height &&
           width_inc == o.width_inc &&
           height_inc == o.height_inc &&
           min_aspect == o.min_aspect &&
           max_aspect == o.max_aspect &&
           base_width == o.base_width &&
           base_height == o.base_height &&
           transient_for == o.transient_for;
  }

  bool operator!=(const WindowProperties& o) {
    return (!operator==(o));
  }

  // Update all properties for the passed-in window.
  bool UpdateAll(XWindow* win);

  string window_name;  // from XFetchName()
  string icon_name;    // from XGetIconName()
  string command;      // from XGetCommand()
  string app_name;     // from XGetClassHint()
  string app_class;    // from XGetClassHint()

  // from XGetWMSizeHints()
  int x;
  int y;
  int width;
  int height;
  int min_width;
  int min_height;
  int max_width;
  int max_height;
  int width_inc;
  int height_inc;
  float min_aspect;
  float max_aspect;
  int base_width;
  int base_height;
  // TODO: add win_gravity?

  XWindow* transient_for;
};

}  // namespace wham

#endif
