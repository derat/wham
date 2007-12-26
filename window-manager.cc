// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-manager.h"

namespace wham {

WindowManager::WindowManager()
    : window_classifier_() {
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


void WindowManager::AddWindow(XWindow* x_window) {
  CHECK(windows_.count(x_window) == 0);
  ref_ptr<Window> window(new Window(x_window));
  windows_.insert(make_pair(x_window, window));
  window->Classify(window_classifier_);
}


void WindowManager::RemoveWindow(XWindow* x_window) {
  CHECK(windows_.count(x_window));
  windows_.erase(x_window);
}


}  // namespace wham
