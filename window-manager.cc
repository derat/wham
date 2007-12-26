// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-manager.h"

namespace wham {

WindowManager::WindowManager(::Display* display)
    : display_(display),
      window_classifier_() {
  // FIXME(derat): just for testing
  ref_ptr<WindowCriteria> crit(new WindowCriteria);
  crit->AddCriterion(WindowCriteria::CRITERION_TYPE_APP_NAME, "rxvt");
  ref_ptr<WindowCriteriaVector> criteria(new WindowCriteriaVector);
  criteria->push_back(crit);

  ref_ptr<WindowConfigVector> configs(new WindowConfigVector);
  configs->push_back(new WindowConfig("default", 400, 800));

  window_classifier_.AddConfig(criteria, configs);

  // Add a default config.
  criteria.reset(new WindowCriteriaVector);
  configs.reset(new WindowConfigVector);
  configs->push_back(new WindowConfig("foo", 300, 300));
  window_classifier_.AddConfig(criteria, configs);
}


void WindowManager::AddWindow(::Window x_window) {
  CHECK(windows_.count(x_window) == 0);
  ref_ptr<Window> window(new Window(x_window));
  windows_.insert(make_pair(x_window, window));

  WindowProperties props;
  if (!GetWindowProperties(x_window, &props)) {
    ERR << "Unable to get window properties for " << static_cast<int>(x_window);
    return;
  }
  //LOG << " props: window_name=" << props.window_name
  //    << " icon_name=" << props.icon_name
  //    << " command=" << props.command
  //    << " app_name=" << props.app_name
  //    << " app_class=" << props.app_class;

  WindowConfigSet configs;
  if (!window_classifier_.ClassifyWindow(props, &configs)) {
    ERR << "Unable to classify window " << static_cast<int>(x_window);
    return;
  }
  if (configs.NumConfigs() == 0) {
    ERR << "Didn't get any configs for " << static_cast<int>(x_window);
    return;
  }

  const WindowConfig* config = configs.GetDefaultConfig();;
  CHECK(config);
  ::XMoveResizeWindow(
      display_, x_window, 40, 40, config->width, config->height);
}


void WindowManager::RemoveWindow(::Window x_window) {
  CHECK(windows_.count(x_window));
  windows_.erase(x_window);
}


bool WindowManager::GetWindowProperties(
    ::Window x_window, WindowProperties* props) {
  CHECK(props);

  char* window_name = NULL;
  if (!XFetchName(display_, x_window, &window_name)) return false;
  props->window_name = window_name ? window_name : "";
  if (window_name) XFree(window_name);

  char* icon_name = NULL;
  if (!XGetIconName(display_, x_window, &icon_name)) return false;
  props->icon_name = icon_name ? icon_name : "";
  if (icon_name) XFree(icon_name);

  // FIXME: get command with XGetCommand()

  XClassHint class_hint;
  if (!XGetClassHint(display_, x_window, &class_hint)) return false;
  props->app_name = class_hint.res_name ? class_hint.res_name : "";
  props->app_class = class_hint.res_class ? class_hint.res_class : "";
  if (class_hint.res_name) XFree(class_hint.res_name);
  if (class_hint.res_class) XFree(class_hint.res_class);

  return true;
}

}  // namespace wham
