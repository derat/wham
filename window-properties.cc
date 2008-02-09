// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-properties.h"

#include <sstream>

#include "util.h"
#include "x-window.h"

namespace wham {

string WindowProperties::ChangeTypeToStr(WindowProperties::ChangeType type) {
  if (type == WINDOW_NAME_CHANGE) return "window name";
  if (type == ICON_NAME_CHANGE) return "icon name";
  if (type == COMMAND_CHANGE) return "command";
  if (type == CLASS_CHANGE) return "class";
  if (type == WM_HINTS_CHANGE) return "wm hints";
  if (type == TRANSIENT_CHANGE) return "transient";
  else return StringPrintf("other (%d)", type);
}


string WindowProperties::DebugString() const {
  ostringstream out;
  out << "window_name=\"" << window_name << "\"\n"
      << "icon_name=\"" << icon_name << "\"\n"
      << "command=\"" << command << "\"\n"
      << "app_name=\"" << app_name << "\"\n"
      << "app_class=\"" << app_class << "\"\n"
      << "x=" << x << "\n"
      << "y=" << y << "\n"
      << "width=" << width << "\n"
      << "height=" << height << "\n"
      << "min_width=" << min_width << "\n"
      << "min_height=" << min_height << "\n"
      << "max_width=" << max_width << "\n"
      << "max_height=" << max_height << "\n"
      << "width_inc=" << width_inc << "\n"
      << "height_inc=" << height_inc << "\n"
      << "min_aspect=" << min_aspect << "\n"
      << "max_aspect=" << max_aspect << "\n"
      << "base_width=" << base_width << "\n"
      << "base_height=" << base_height << "\n"
      << "transient_for=0x" << hex
      << (transient_for ? transient_for->id() : 0) << "\n";
  return out.str();
}


bool WindowProperties::UpdateAll(XWindow* win) {
  CHECK(win);
  win->UpdateProperties(this, WINDOW_NAME_CHANGE);
  win->UpdateProperties(this, ICON_NAME_CHANGE);
  win->UpdateProperties(this, COMMAND_CHANGE);
  win->UpdateProperties(this, CLASS_CHANGE);
  win->UpdateProperties(this, WM_HINTS_CHANGE);
  win->UpdateProperties(this, TRANSIENT_CHANGE);
  // FIXME: do something with errors?
  return true;
}

}  // namespace wham
