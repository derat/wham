// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>

#include "window-manager.h"

#include "config.h"
#include "x.h"

namespace wham {

WindowManager::WindowManager()
    : current_desktop_(NULL),
      mouse_down_(false),
      dragging_(false),
      drag_offset_x_(0),
      drag_offset_y_(0),
      mouse_down_x_(0),
      mouse_down_y_(0) {
  // Create a bunch of stuff just for testing.
  ref_ptr<WindowCriteria> crit(new WindowCriteria);
  crit->AddCriterion(WindowCriteria::CRITERION_TYPE_APP_NAME, "rxvt");
  ref_ptr<WindowCriteriaVector> criteria(new WindowCriteriaVector);
  criteria->push_back(crit);

  ref_ptr<WindowConfigVector> configs(new WindowConfigVector);
  configs->push_back(ref_ptr<WindowConfig>(new WindowConfig("abc", 300, 300)));

  window_classifier_.AddConfig(criteria, configs);

  criteria.reset(new WindowCriteriaVector);
  configs.reset(new WindowConfigVector);
  configs->push_back(ref_ptr<WindowConfig>(new WindowConfig("def", 400, 400)));
  window_classifier_.AddConfig(criteria, configs);

  Window::SetClassifier(&window_classifier_);

  desktops_.push_back(ref_ptr<Desktop>(new Desktop()));
  current_desktop_ = desktops_[0].get();
  current_desktop_->CreateAnchor("anchor1", 50, 50);
}


void WindowManager::HandleButtonPress(XWindow* x_window, int x, int y) {
  Anchor* anchor = current_desktop_->GetAnchorByTitlebar(x_window);
  CHECK(anchor);

  // Make this the active anchor.
  current_desktop_->SetActiveAnchor(anchor);

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
    Anchor* anchor = current_desktop_->GetAnchorByTitlebar(x_window);
    CHECK(anchor);
    anchor->ActivateWindowAtCoordinates(x, y);
  }
}


void WindowManager::HandleCreateWindow(XWindow* x_window) {
  // We don't want to manage anchor titlebars.
  if (IsAnchorWindow(x_window)) return;

  ref_ptr<Window> window(new Window(x_window));
  windows_.insert(make_pair(x_window, window));
  current_desktop_->AddWindow(window.get());
}


void WindowManager::HandleDestroyWindow(XWindow* x_window) {
  // FIXME: Do I need to handle anchor titlebars here?
  if (IsAnchorWindow(x_window)) return;

  Window* window = FindWithDefault(windows_, x_window, ref_ptr<Window>()).get();
  CHECK(window);
  for (DesktopVector::iterator desktop = desktops_.begin();
       desktop != desktops_.end(); ++desktop) {
    (*desktop)->RemoveWindow(window);
  }
  windows_.erase(x_window);
}


void WindowManager::HandleExposeWindow(XWindow* x_window) {
  Anchor* anchor = current_desktop_->GetAnchorByTitlebar(x_window);
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
  Anchor* anchor = current_desktop_->active_anchor();
  CHECK(anchor);
  anchor->Move(x - drag_offset_x_, y - drag_offset_y_);
}


void WindowManager::HandleCommand(const Command &cmd) {
  switch (cmd.type()) {
    case Command::CREATE_ANCHOR:
      current_desktop_->CreateAnchor("new", 250, 250);
      break;
    case Command::CYCLE_ANCHOR_GRAVITY:
      {
        Anchor* anchor = current_desktop_->active_anchor();
        if (anchor) anchor->CycleGravity(cmd.GetBoolArg());
      }
      break;
    case Command::EXEC:
      Exec(cmd.GetStringArg());
      break;
    case Command::SWITCH_NEAREST_ANCHOR:
      /*
      {
        Anchor* anchor = GetNearestAnchor(cmd.args[0]);
        LOG << "anchor=" << hex << anchor;
      }
      */
      break;
    case Command::SWITCH_NTH_WINDOW:
      {
        Anchor* anchor = current_desktop_->active_anchor();
        if (anchor) anchor->SetActive(cmd.GetIntArg());
      }
      break;
    default:
      ERROR << "Got unknown command " << cmd.type();
  }
}


bool WindowManager::IsAnchorWindow(XWindow* x_window) {
  for (DesktopVector::const_iterator desktop = desktops_.begin();
       desktop != desktops_.end(); ++desktop) {
    if ((*desktop)->IsTitlebarWindow(x_window)) return true;
  }
  return false;
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

}  // namespace wham
