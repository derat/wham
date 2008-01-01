// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>

#include "window-manager.h"

#include "config.h"

namespace wham {

WindowManager::WindowManager()
    : active_anchor_(0),
      mouse_down_(false),
      dragging_(false),
      drag_offset_x_(0),
      drag_offset_y_(0),
      mouse_down_x_(0),
      mouse_down_y_(0) {
  // FIXME(derat): just for testing
  ref_ptr<WindowCriteria> crit(new WindowCriteria);
  crit->AddCriterion(WindowCriteria::CRITERION_TYPE_APP_NAME, "rxvt");
  ref_ptr<WindowCriteriaVector> criteria(new WindowCriteriaVector);
  criteria->push_back(crit);

  ref_ptr<WindowConfigVector> configs(new WindowConfigVector);
  configs->push_back(new WindowConfig("default", 300, 300));

  window_classifier_.AddConfig(criteria, configs);

  // Add a default config.
  criteria.reset(new WindowCriteriaVector);
  configs.reset(new WindowConfigVector);
  configs->push_back(new WindowConfig("foo", 400, 400));
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
  WindowAnchor* anchor = FindWithDefault(
      anchor_titlebars_, x_window, static_cast<WindowAnchor*>(NULL));
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
  mouse_down_ = true;
  drag_offset_x_ = x - anchor->x();
  drag_offset_y_ = y - anchor->y();
  mouse_down_x_ = x;
  mouse_down_y_ = y;
}


void WindowManager::HandleButtonRelease(XWindow* x_window, int x, int y) {
  mouse_down_ = false;
  if (dragging_) {
    dragging_ = false;
  } else {
    WindowAnchor* anchor = FindWithDefault(
        anchor_titlebars_, x_window, static_cast<WindowAnchor*>(NULL));
    if (anchor == NULL) {
      ERROR << "Ignoring button release for unknown window";
      return;
    }
    anchor->ActivateWindowAtCoordinates(x, y);
  }
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
  if (!mouse_down_) return;
  if (!dragging_) {
    if (abs(x - mouse_down_x_) <= config->dragging_threshold &&
        abs(y - mouse_down_y_) <= config->dragging_threshold) {
      return;
    }
    dragging_ = true;
  }
  WindowAnchor* anchor = anchors_[active_anchor_].get();
  CHECK(anchor);
  anchor->Move(x - drag_offset_x_, y - drag_offset_y_);
}


void WindowManager::HandleCommand(const Command &cmd) {
  switch (cmd.type) {
    case Command::CREATE_ANCHOR:
      CreateAnchor("new", 250, 250);
      break;
    case Command::EXEC:
      Exec(cmd.args[0]);
      break;
    case Command::SWITCH_ANCHOR:
      {
        WindowAnchor* anchor = GetNearestAnchor(cmd.args[0]);
        LOG << "anchor=" << hex << anchor;
      }
      break;
    case Command::SWITCH_WINDOW:
      {
        WindowAnchor* anchor = anchors_[active_anchor_].get();
        CHECK(anchor);
        anchor->SetActive(atoi(cmd.args[0].c_str()));
      }
      break;
    default:
      ERROR << "Got unknown command " << cmd.type;
  }
}


bool WindowManager::Exec(const string& command) {
  DEBUG << "Executing " << command;
  const char* shell = "/bin/sh";
  if (fork() == 0) {
    if (fork() == 0) {
      execl(shell, shell, "-c", command.c_str(), static_cast<char*>(NULL));
      ERROR << "execve() failed: " << strerror(errno);
    }
    exit(0);
  }
  wait(0);
  return true;
}


WindowAnchor* WindowManager::GetNearestAnchor(const string& direction) const {
  int dx = 0, dy = 0;
  if (direction == "left")       dx = -1;
  else if (direction == "right") dx =  1;
  else if (direction == "up")    dy = -1;
  else if (direction == "down")  dy =  1;
  else return NULL;

  ref_ptr<WindowAnchor> active = anchors_[active_anchor_];
  WindowAnchor* nearest = NULL;
  int nearest_dist = INT_MAX;
  for (WindowAnchorVector::const_iterator anchor = anchors_.begin();
       anchor != anchors_.end(); ++anchor) {
    if (*anchor == active) continue;
    int dist = 0;
    if (dy != 0) {
      dist = (*anchor)->y() - active->y();
      if (dy * dist < dy) continue;
      // FIXME: also need to check that the anchor isn't too far to the
      // side of the titlebar
    } else {
      dist = (*anchor)->x() - active->x();
      if (dx * dist < dx) continue;
    }
    if (dist < nearest_dist) {
      nearest = anchor->get();
      nearest_dist = dist;
    }
  }
  return nearest;
}

}  // namespace wham
