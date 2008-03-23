// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>

#include "window-manager.h"

#include "config.h"
#include "config-parser.h"
#include "x-server.h"
#include "x-window.h"

namespace wham {

WindowManager::WindowManager()
    : active_desktop_(NULL),
      attach_follows_active_(true),
      mouse_down_(false),
      dragging_(false),
      drag_offset_x_(0),
      drag_offset_y_(0),
      mouse_down_x_(0),
      mouse_down_y_(0) {
}


// FIXME: get rid of this
void WindowManager::SetupDefaultCrap() {
  Desktop* desktop = CreateDesktop();
  SetActiveDesktop(desktop);
  active_desktop_->CreateAnchor("anchor1", 50, 50);
}


bool WindowManager::LoadConfig(const string& filename) {
  vector<ConfigError> errors;
  ref_ptr<Config> config(new Config);
  bool status = config->Load(filename, &errors);
  for (vector<ConfigError>::const_iterator error = errors.begin();
       error != errors.end(); ++error) {
    ERROR << error->ToString();
  }
  if (!status) {
    ERROR << "Couldn't load config";
    return false;
  }
  XServer::Get()->RegisterKeyBindings(config->key_bindings);
  WindowClassifier::Swap(config->window_classifier);
  Config::Swap(config);
  return true;
}


void WindowManager::HandleButtonPress(XWindow* x_window, int x, int y) {
  CHECK(active_desktop_);
  Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
  CHECK(anchor);

  // Make this the active anchor.
  SetActiveAnchor(anchor);
  anchor->Raise();

  mouse_down_ = true;
  drag_offset_x_ = x - anchor->x();
  drag_offset_y_ = y - anchor->y();
  mouse_down_x_ = x;
  mouse_down_y_ = y;
}


void WindowManager::HandleButtonRelease(XWindow* x_window, int x, int y) {
  CHECK(active_desktop_);
  mouse_down_ = false;
  if (dragging_) {
    dragging_ = false;
  } else {
    Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
    CHECK(anchor);
    int index = anchor->GetWindowIndexAtTitlebarPoint(x);
    if (index >= 0) anchor->SetActiveWindow(index);
  }
}


void WindowManager::HandleCreateWindow(XWindow* x_window) {
  // FIXME: Move the window to the correct location here, and maybe even
  // classify it so we can resize it.  We want to do this before it's
  // mapped to avoid flicker.
}


void WindowManager::HandleDestroyWindow(XWindow* x_window) {
  if (IsAnchorWindow(x_window)) return;

  Window* window = FindWithDefault(windows_, x_window, ref_ptr<Window>()).get();
  if (window == NULL) return;  // Maybe it never got mapped.
  for (set<Desktop*>::iterator desktop = window_desktops_[window].begin();
       desktop != window_desktops_[window].end(); ++desktop) {
    RemoveWindowFromDesktop(window, *desktop);
  }
  window_desktops_.erase(window);
  windows_.erase(x_window);
}


void WindowManager::HandleEnterWindow(XWindow* x_window) {
  CHECK(active_desktop_);
  Anchor* anchor = NULL;
  if (IsAnchorWindow(x_window)) {
    // anchor titlebar
    anchor = active_desktop_->GetAnchorByTitlebar(x_window);
  } else {
    // client window
    Window* window =
        FindWithDefault(windows_, x_window, ref_ptr<Window>()).get();
    anchor = active_desktop_->GetAnchorContainingWindow(window);
  }
  // In either case, we want to make this anchor active (which will also
  // focus its window).
  CHECK(anchor);
  SetActiveAnchor(anchor);
}


void WindowManager::HandleExposeWindow(XWindow* x_window) {
  CHECK(active_desktop_);
  Anchor* anchor = active_desktop_->GetAnchorByTitlebar(x_window);
  CHECK(anchor);
  anchor->DrawTitlebar();
}


void WindowManager::HandleMapWindow(XWindow* x_window) {
  // We don't want to manage anchor titlebars.
  if (IsAnchorWindow(x_window)) return;

  if (windows_.find(x_window) == windows_.end()) {
    x_window->SetBorder(Config::Get()->window_border);
    x_window->SelectEvents();
    ref_ptr<Window> window(new Window(x_window));
    windows_.insert(make_pair(x_window, window));
    Window* transient_for = GetTransientFor(window.get());
    if (transient_for == NULL) {
      CHECK(active_desktop_);
      AddWindowToDesktop(window.get(), active_desktop_, NULL);
    } else {
      HandleTransientFor(window.get(), transient_for);
    }
  }
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
  CHECK(active_desktop_);
  Anchor* anchor = active_desktop_->active_anchor();
  CHECK(anchor);
  anchor->Move(x - drag_offset_x_, y - drag_offset_y_);
}


void WindowManager::HandlePropertyChange(
    XWindow* x_window, WindowProperties::ChangeType type) {
  if (IsAnchorWindow(x_window)) return;

  Window* window = FindWithDefault(windows_, x_window, ref_ptr<Window>()).get();
  CHECK(window);

  if (type == WindowProperties::TRANSIENT_CHANGE) {
    // FIXME: handle this?
  } else {
    bool need_to_redraw = window->HandlePropertyChange(type);
    if (need_to_redraw) {
      CHECK(active_desktop_);
      Anchor* anchor = active_desktop_->GetAnchorContainingWindow(window);
      if (anchor) anchor->DrawTitlebar();
    }
  }
}


void WindowManager::HandleCommand(const Command &cmd) {
  CHECK(active_desktop_);

  if (cmd.type() == Command::ATTACH_TAGGED_WINDOWS) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) AttachTaggedWindows(anchor);
  } else if (cmd.type() == Command::CREATE_ANCHOR) {
    active_desktop_->CreateAnchor("new", 250, 250);
  } else if (cmd.type() == Command::CREATE_DESKTOP) {
    SetActiveDesktop(CreateDesktop());
  } else if (cmd.type() == Command::CYCLE_ANCHOR_GRAVITY) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->CycleGravity(cmd.GetBoolArg());
  } else if (cmd.type() == Command::CYCLE_DESKTOP) {
    int num = (desktops_.size() + GetDesktopIndex(active_desktop_) +
               (cmd.GetBoolArg() ? 1 : -1)) % desktops_.size();
    SetActiveDesktop((desktops_.begin() + num)->get());
  } else if (cmd.type() == Command::CYCLE_WINDOW_CONFIG) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->CycleActiveWindowConfig(cmd.GetBoolArg());
  } else if (cmd.type() == Command::DISPLAY_WINDOW_PROPS) {
    Window* window = GetActiveWindow();
    if (window) LOG << window->props().DebugString();
  } else if (cmd.type() == Command::EXEC) {
    Exec(cmd.GetStringArg());
  } else if (cmd.type() == Command::SET_ATTACH_ANCHOR) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor == active_desktop_->attach_anchor()) {
      DEBUG << "Setting anchor_follows_active_ from "
            << attach_follows_active_ << " to " << !attach_follows_active_;
      attach_follows_active_ = !attach_follows_active_;
    } else if (anchor) {
      DEBUG << "Setting anchor_follows_active_ to 1";
      attach_follows_active_ = true;
      active_desktop_->SetAttachAnchor(anchor);
    }
  } else if (cmd.type() == Command::SWITCH_NEAREST_ANCHOR) {
    Anchor* anchor = active_desktop_->GetNearestAnchor(cmd.GetDirectionArg());
    if (anchor) SetActiveAnchor(anchor);
  } else if (cmd.type() == Command::SWITCH_NTH_DESKTOP) {
    int num = cmd.GetIntArg();
    if (num >= 0 && num < static_cast<int>(desktops_.size())) {
      SetActiveDesktop((desktops_.begin() + num)->get());
    }
  } else if (cmd.type() == Command::SWITCH_NTH_WINDOW) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->SetActiveWindow(cmd.GetIntArg());
  } else if (cmd.type() == Command::TOGGLE_TAG) {
    Window* window = GetActiveWindow();
    if (window) ToggleWindowTag(window);
  } else {
    ERROR << "Got unknown command " << cmd.type();
  }
}


Desktop* WindowManager::CreateDesktop() {
  vector<ref_ptr<Desktop> >::iterator it = desktops_.end();
  // Append the new desktop after the active desktop.
  if (active_desktop_) {
    int i = GetDesktopIndex(active_desktop_);
    CHECK(i >= 0 && i < static_cast<int>(desktops_.size()));
    it = desktops_.begin() + i + 1;
    DEBUG << "Inserting desktop after position " << i;
  }
  ref_ptr<Desktop> desktop(new Desktop());
  desktops_.insert(it, desktop);
  DEBUG << "Created desktop " << desktop->name();
  return desktop.get();
}


int WindowManager::GetDesktopIndex(Desktop* desktop) {
  for (uint i = 0; i < desktops_.size(); ++i) {
    if (desktops_[i].get() == desktop) {
      return static_cast<int>(i);
    }
  }
  return -1;
}


void WindowManager::SetActiveDesktop(Desktop* desktop) {
  CHECK(desktop);
  DEBUG << "Switching to desktop " << desktop->name();
  if (desktop == active_desktop_) return;
  if (active_desktop_) active_desktop_->Hide();
  active_desktop_ = desktop;
  desktop->Show();
}


void WindowManager::AttachTaggedWindows(Anchor* anchor) {
  CHECK(anchor);
  CHECK(active_desktop_->HasAnchor(anchor));

  while (!tagged_windows_.empty()) {
    Window* window = *(tagged_windows_.begin());
    ToggleWindowTag(window);

    // FIXME: Should we really remove the window from all desktops here?
    // It'd probably make more sense for "attach" to move the window to a
    // new anchor if it's already present in a different anchor on this
    // desktop, and just add it to this desktop otherwise.
    for (set<Desktop*>::iterator desktop = window_desktops_[window].begin();
         desktop != window_desktops_[window].end(); ++desktop) {
      RemoveWindowFromDesktop(window, *desktop);
    }
    AddWindowToDesktop(window, active_desktop_, anchor);
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
  Anchor* anchor = active_desktop_->GetAnchorContainingWindow(window);
  if (anchor != NULL) anchor->DrawTitlebar();
}


void WindowManager::SetActiveAnchor(Anchor* anchor) {
  CHECK(anchor);
  CHECK(active_desktop_);
  active_desktop_->SetActiveAnchor(anchor);
  if (attach_follows_active_) active_desktop_->SetAttachAnchor(anchor);
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


Window* WindowManager::GetTransientFor(Window* transient) const {
  CHECK(transient);
  XWindow* xwin = transient->transient_for();
  return FindWithDefault(windows_, xwin, ref_ptr<Window>()).get();
}


void WindowManager::HandleTransientFor(Window* transient, Window* win) {
  CHECK(transient);
  CHECK(win);

  // FIXME: find the right desktop(s)
  CHECK(active_desktop_);
  Anchor* win_anchor = active_desktop_->GetAnchorContainingWindow(win);
  CHECK(win_anchor);

  int x = win->x() + win->width() / 2 - transient->width() / 2;
  int y = win->y() + win->height() / 2 - transient->height() / 2;
  Anchor* anchor = active_desktop_->CreateAnchor("transient", x, y);
  anchor->set_transient(true);
  AddWindowToDesktop(transient, active_desktop_, anchor);
}


Window* WindowManager::GetActiveWindow() const {
  CHECK(active_desktop_);
  Anchor* anchor = active_desktop_->active_anchor();
  if (anchor == NULL) return NULL;
  return anchor->mutable_active_window();
}


void WindowManager::AddWindowToDesktop(
    Window* window, Desktop* desktop, Anchor* anchor) {
  CHECK(window);
  CHECK(desktop);

  if (anchor == NULL) {
    desktop->AddWindow(window);
  } else {
    desktop->AddWindowToAnchor(window, anchor);
  }
  window_desktops_[window].insert(desktop);
}


void WindowManager::RemoveWindowFromDesktop(Window* window, Desktop* desktop) {
  CHECK(window);
  CHECK(desktop);

  desktop->RemoveWindow(window);
  window_desktops_[window].erase(desktop);
}

}  // namespace wham
