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
    : active_desktop_(NULL),
      mouse_down_(false),
      dragging_(false),
      drag_offset_x_(0),
      drag_offset_y_(0),
      mouse_down_x_(0),
      mouse_down_y_(0) {
}


// FIXME: get rid of this
void WindowManager::SetupDefaultCrap() {
  CreateDesktop();
  active_desktop_->CreateAnchor("anchor1", 50, 50);
}


bool WindowManager::LoadConfig(const string& filename) {
  ConfigNode parsed_config;
  if (!ConfigParser::ParseFromFile(filename, &parsed_config, NULL)) {
    ERROR << "Couldn't parse file";
    return false;
  }
  ref_ptr<Config> config(new Config);
  if (!config->Load(parsed_config)) {
    ERROR << "Couldn't load config";
    return false;
  }
  XServer::Get()->RegisterKeyBindings(config->key_bindings);
  Config::Swap(config);
  return true;
}


void WindowManager::HandleButtonPress(XWindow* x_window, int x, int y) {
  Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
  CHECK(anchor);

  // Make this the active anchor.
  active_desktop_->SetActiveAnchor(anchor);

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
    Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
    CHECK(anchor);
    anchor->ActivateWindowAtCoordinates(x, y);
  }
}


void WindowManager::HandleCreateWindow(XWindow* x_window) {
  // We don't want to manage anchor titlebars.
  if (IsAnchorWindow(x_window)) return;

  x_window->SetBorder(Config::Get()->window_border);
  x_window->SelectEvents();
  ref_ptr<Window> window(new Window(x_window));
  windows_.insert(make_pair(x_window, window));
  active_desktop_->AddWindow(window.get());
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


void WindowManager::HandleEnterWindow(XWindow* x_window) {
  if (IsAnchorWindow(x_window)) {
    // Anchor titlebar; give its active window the focus.
    Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
    CHECK(anchor);
    anchor->FocusActiveWindow();
  } else {
    // Client window; give it the focus.
    x_window->TakeFocus();
  }
}


void WindowManager::HandleExposeWindow(XWindow* x_window) {
  Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
  CHECK(anchor);
  anchor->DrawTitlebar();
}


void WindowManager::HandleMotion(XWindow* x_window, int x, int y) {
  if (!mouse_down_) return;
  if (!dragging_) {
    if (abs(x - mouse_down_x_) <= Config::Get()->dragging_threshold &&
        abs(y - mouse_down_y_) <= Config::Get()->dragging_threshold) {
      return;
    }
    dragging_ = true;
  }
  Anchor* anchor = active_desktop_->active_anchor();
  CHECK(anchor);
  anchor->Move(x - drag_offset_x_, y - drag_offset_y_);
}


void WindowManager::HandleCommand(const Command &cmd) {
  if (cmd.type() == Command::ATTACH_TAGGED_WINDOWS) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) AttachTaggedWindows(anchor);
  } else if (cmd.type() == Command::CREATE_ANCHOR) {
    active_desktop_->CreateAnchor("new", 250, 250);
  } else if (cmd.type() == Command::CYCLE_ANCHOR_GRAVITY) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->CycleGravity(cmd.GetBoolArg());
  } else if (cmd.type() == Command::CYCLE_WINDOW_CONFIG) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->CycleActiveWindowConfig(cmd.GetBoolArg());
  } else if (cmd.type() == Command::EXEC) {
    Exec(cmd.GetStringArg());
  } else if (cmd.type() == Command::SWITCH_NEAREST_ANCHOR) {
    /*
    Anchor* anchor = GetNearestAnchor(cmd.args[0]);
    LOG << "anchor=" << hex << anchor;
    */
  } else if (cmd.type() == Command::SWITCH_NTH_WINDOW) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->SetActive(cmd.GetIntArg());
  } else if (cmd.type() == Command::TOGGLE_TAG) {
    Window* window = GetActiveWindow();
    if (window) {
      ToggleWindowTag(window);
      Anchor* anchor = active_desktop_->active_anchor();
      CHECK(anchor);
      anchor->DrawTitlebar();
    }
  } else {
    ERROR << "Got unknown command " << cmd.type();
  }
}


Desktop* WindowManager::CreateDesktop() {
  // FIXME: Append this after the active desktop, if one exists.
  desktops_.push_back(ref_ptr<Desktop>(new Desktop()));
  active_desktop_ = desktops_.back().get();
  return desktops_.back().get();
}


void WindowManager::AttachTaggedWindows(Anchor* anchor) {
  CHECK(anchor);
  for (set<Window*>::iterator it = tagged_windows_.begin();
       it != tagged_windows_.end(); ++it) {
    Window* window = *it;
    for (DesktopVector::iterator desktop = desktops_.begin();
         desktop != desktops_.end(); ++desktop) {
      (*desktop)->RemoveWindow(window);
    }
    anchor->AddWindow(window);
  }
  while (!tagged_windows_.empty()) {
    ToggleWindowTag(*(tagged_windows_.begin()));
  }
}


void WindowManager::ToggleWindowTag(Window* window) {
  CHECK(window);
  set<Window*>::iterator it = tagged_windows_.find(window);
  if (it != tagged_windows_.end()) {
    tagged_windows_.erase(it);
    window->set_tagged(false);
  } else {
    tagged_windows_.insert(window);
    window->set_tagged(true);
  }
}


bool WindowManager::IsAnchorWindow(XWindow* x_window) const {
  for (DesktopVector::const_iterator desktop = desktops_.begin();
       desktop != desktops_.end(); ++desktop) {
    if ((*desktop)->IsTitlebarWindow(x_window)) return true;
  }
  return false;
}


bool WindowManager::Exec(const string& command) const {
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


Window* WindowManager::GetActiveWindow() const {
  Anchor* anchor = active_desktop_->active_anchor();
  if (anchor == NULL) return NULL;
  return anchor->mutable_active_window();
}

}  // namespace wham
