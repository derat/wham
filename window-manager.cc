// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-manager.h"

namespace wham {

WindowManager::WindowManager()
    : active_anchor_(0),
      in_drag_(false),
      drag_offset_x_(0),
      drag_offset_y_(0) {
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

  CreateAnchor("anchor1", 50, 50);
  CreateAnchor("anchor2", 300, 300);
  active_anchor_ = 0;
}


void WindowManager::CreateAnchor(const string& name, int x, int y) {
  ref_ptr<WindowAnchor> anchor(new WindowAnchor(name, x, y));
  anchors_.push_back(anchor);
  anchor_titlebars_.insert(make_pair(anchor->titlebar(), anchor.get()));
}


void WindowManager::HandleButtonPress(XWindow* x_window, int x, int y) {
  WindowAnchor* anchor = FindWithDefault(anchor_titlebars_, x_window,
                                         static_cast<WindowAnchor*>(NULL));
  if (anchor == NULL) {
    ERROR << "Ignoring button press for unknown window";
    return;
  }
  LOG << "Got button press for anchor " << anchor->name();

  // Make this the active anchor.
  for (uint i = 0; i < anchors_.size(); ++i) {
    if (anchor == anchors_[i].get()) {
      active_anchor_ = i;
      break;
    }
  }
  in_drag_ = true;
  drag_offset_x_ = x - anchor->x();
  drag_offset_y_ = y - anchor->y();
}


void WindowManager::HandleButtonRelease(XWindow* x_window) {
  in_drag_ = false;
}


void WindowManager::HandleCreateWindow(XWindow* x_window) {
  // We don't want to manage anchor titlebars.
  if (anchor_titlebars_.count(x_window)) return;

  CHECK(windows_.count(x_window) == 0);
  ref_ptr<Window> window(new Window(x_window));
  windows_.insert(make_pair(x_window, window));
  window->Classify(window_classifier_);

  WindowAnchor* anchor = anchors_[active_anchor_].get();
  anchor->AddWindow(window.get());
  windows_to_anchors_[window.get()].push_back(anchor);
  anchor->SetActive(anchor->NumWindows()-1);
}


void WindowManager::HandleDestroyWindow(XWindow* x_window) {
  CHECK(windows_.count(x_window) == 1);
  Window* window = windows_[x_window].get();
  CHECK(window);
  WindowAnchorPtrVector& anchors = windows_to_anchors_[window];
  for (WindowAnchorPtrVector::iterator anchor = anchors.begin();
       anchor != anchors.end(); ++anchor) {
    (*anchor)->RemoveWindow(window);
  }
  windows_to_anchors_.erase(window);
  windows_.erase(x_window);
}


void WindowManager::HandleExposeWindow(XWindow* x_window) {
  CHECK(anchor_titlebars_.count(x_window));
  WindowAnchor* anchor = anchor_titlebars_[x_window];
  CHECK(anchor);
  anchor->DrawTitlebar();
}


void WindowManager::HandleMotion(XWindow* x_window, int x, int y) {
  if (!in_drag_) return;
  WindowAnchor* anchor = anchors_[active_anchor_].get();
  CHECK(anchor);
  anchor->Move(x - drag_offset_x_, y - drag_offset_y_);
}


}  // namespace wham
