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

// How many pixels should a window be offset from its old anchor when it's
// detached?
static const int kWindowDetachOffset = 10;


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


void WindowManager::HandleButtonPress(
    XWindow* xwin, int x, int y, uint button) {
  if (button == Config::Get()->mouse_primary_button) {
    CHECK(active_desktop_);
    Anchor* anchor = active_desktop_->GetAnchorByTitlebar(xwin);
    if (anchor == NULL) return;  // FIXME: handle button presses on borders

    // Make this the active anchor.
    SetActiveAnchor(anchor);
    anchor->Raise();

    mouse_down_ = true;
    drag_offset_x_ = x - anchor->x();
    drag_offset_y_ = y - anchor->y();
    mouse_down_x_ = x;
    mouse_down_y_ = y;
  } else if (button == Config::Get()->mouse_secondary_button) {
    if (mouse_down_) {
      CHECK(active_desktop_);
      Anchor* anchor = active_desktop_->active_anchor();
      if (anchor == NULL) return;  // FIXME: handle button presses on borders
      Window* window = anchor->mutable_active_window();
      if (window == NULL) return;

      if (anchor->windows().size() > 1) {
        RemoveWindowFromDesktop(window, active_desktop_);
        int dx = 0, dy = 0;
        Anchor::GetGravityDirection(anchor->gravity(), &dx, &dy);
        Anchor* new_anchor = active_desktop_->CreateAnchor(
            "detached",
            x - drag_offset_x_ - dx * kWindowDetachOffset,
            y - drag_offset_y_ - dy * kWindowDetachOffset);
        new_anchor->set_temporary(true);
        new_anchor->SetGravity(anchor->gravity());
        AddWindowToDesktop(window, active_desktop_, new_anchor);
        SetActiveAnchor(new_anchor);
        new_anchor->Raise();

        drag_offset_x_ = x - new_anchor->x();
        drag_offset_y_ = y - new_anchor->y();
        mouse_down_x_ = x;
        mouse_down_y_ = y;
      } else {
        vector<Anchor*> anchors;
        active_desktop_->GetAnchorsAtPosition(x, y, &anchors);
        Anchor* new_anchor = NULL;
        for (vector<Anchor*>::const_iterator it = anchors.begin();
             it != anchors.end(); ++it) {
          // Skip the anchor the window's currently in.
          if (*it == anchor) continue;
          new_anchor = *it;
          break;
        }
        if (new_anchor) {
          RemoveWindowFromDesktop(window, active_desktop_);
          AddWindowToDesktop(window, active_desktop_, new_anchor);
          SetActiveAnchor(new_anchor);
          drag_offset_x_ = x - new_anchor->x();
          drag_offset_y_ = y - new_anchor->y();
          mouse_down_x_ = x;
          mouse_down_y_ = y;
        }
      }
    }
  }
}


void WindowManager::HandleButtonRelease(
    XWindow* xwin, int x, int y, uint button) {
  if (button == Config::Get()->mouse_primary_button) {
    CHECK(active_desktop_);
    mouse_down_ = false;
    if (dragging_) {
      dragging_ = false;
    } else {
      Anchor* anchor = active_desktop_->GetAnchorByTitlebar(xwin);
      // Maybe this is a window border and not a titlebar.
      if (anchor) {
        int index = anchor->GetWindowIndexAtTitlebarPoint(x);
        if (index >= 0) anchor->SetActiveWindow(index);
      }
    }
  }
}


void WindowManager::HandleEnterWindow(XWindow* xwin) {
  // We don't want to update the focus if the user is already dragging.
  if (mouse_down_) return;

  CHECK(active_desktop_);
  Anchor* anchor = NULL;
  if (IsAnchorWindow(xwin)) {
    // anchor titlebar
    anchor = active_desktop_->GetAnchorByTitlebar(xwin);
  } else {
    // client window
    Window* window =
        FindWithDefault(windows_, xwin, ref_ptr<Window>()).get();
    if (window) anchor = window->anchor();
    // FIXME: handle window borders
  }
  // In either case, we want to make this anchor active (which will also
  // focus its window).
  if (anchor) SetActiveAnchor(anchor);
}


void WindowManager::HandleExposeWindow(XWindow* xwin) {
  CHECK(active_desktop_);
  Anchor* anchor = active_desktop_->GetAnchorByTitlebar(xwin);
  if (anchor) {
    anchor->DrawTitlebar();
    return;
  }

  Window* window = FindWithDefault(frames_,
                                   static_cast<const XWindow*>(xwin),
                                   static_cast<Window*>(NULL));
  if (window) {
    window->DrawFrame();
    return;
  }
}


void WindowManager::HandleMapRequest(XWindow* xwin) {
  // We don't want to manage anchor titlebars.
  if (IsAnchorWindow(xwin)) {
    xwin->Map();
    return;
  }

  if (windows_.find(xwin) != windows_.end()) return;

  xwin->SetBorder(0);
  xwin->SelectClientEvents();
  ref_ptr<Window> window(new Window(xwin));
  windows_.insert(make_pair(xwin, window));
  frames_.insert(make_pair(window->frame(), window.get()));
  Window* transient_for = GetTransientFor(window.get());
  if (transient_for == NULL) {
    CHECK(active_desktop_);
    AddWindowToDesktop(window.get(), active_desktop_, NULL);
  } else {
    HandleTransientFor(window.get(), transient_for);
  }

  if (WindowShouldBeMapped(window.get())) window->Map();
}


void WindowManager::HandleMotion(XWindow* xwin, int x, int y) {
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
    XWindow* xwin, WindowProperties::ChangeType type) {
  if (IsAnchorWindow(xwin)) return;

  Window* window = FindWithDefault(windows_, xwin, ref_ptr<Window>()).get();
  CHECK(window);

  if (type == WindowProperties::TRANSIENT_CHANGE) {
    // FIXME: handle this?
  } else {
    bool changed = false;
    window->HandlePropertyChange(type, &changed);
    if (changed) {
      window->anchor()->DrawTitlebar();
    }
  }
}


void WindowManager::HandleUnmapWindow(XWindow* xwin) {
  // FIXME: create a little method that does this
  Window* window = FindWithDefault(windows_, xwin, ref_ptr<Window>()).get();
  if (!window) return;

  DEBUG << "Stopping management of 0x" << hex << xwin->id();
  for (set<Desktop*>::iterator desktop = window_desktops_[window].begin();
       desktop != window_desktops_[window].end(); ++desktop) {
    RemoveWindowFromDesktop(window, *desktop);
  }
  window_desktops_.erase(window);
  frames_.erase(window->frame());
  windows_.erase(xwin);
}


void WindowManager::HandleWindowDamage(XWindow* xwin) {
  //xwin->CopyToOverlay();
  XServer::Get()->RepaintOverlay();  // FIXME
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
  } else if (cmd.type() == Command::CYCLE_WINDOW) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->CycleActiveWindow(cmd.GetBoolArg());
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
  } else if (cmd.type() == Command::SHIFT_WINDOW_IN_ANCHOR) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->ShiftActiveWindow(cmd.GetBoolArg());
  } else if (cmd.type() == Command::SLIDE_ANCHOR) {
    Anchor* anchor = active_desktop_->active_anchor();
    if (anchor) anchor->Slide(cmd.GetDirectionArg());
  } else if (cmd.type() == Command::SWITCH_NEAREST_ANCHOR) {
    Anchor* anchor = active_desktop_->GetNearestAnchor(cmd.GetDirectionArg());
    if (anchor) {
      SetActiveAnchor(anchor);
      // FIXME: Figure out how this should fit together with animations.
      // FIXME: This is still busted -- as soon as I move the pointer after
      // warping it, it jumps back to its previous position.  Is this
      // because I'm doing something wrong, or just something that happens
      // when running under Xephyr?
      //anchor->titlebar()->WarpPointer(0, 0);
    }
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
    DEBUG << "Inserting new desktop after position " << i;
  }
  ref_ptr<Desktop> desktop(new Desktop());
  desktops_.insert(it, desktop);
  DEBUG << "Created desktop " << desktop->DebugString();
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
  DEBUG << "Switching to desktop " << desktop->DebugString();
  if (desktop == active_desktop_) return;
  if (active_desktop_) active_desktop_->Hide();
  active_desktop_ = desktop;
  desktop->Show();
}


void WindowManager::AttachTaggedWindows(Anchor* anchor) {
  CHECK(anchor);
  Desktop* desktop = anchor->desktop();
  CHECK(desktop);

  while (!tagged_windows_.empty()) {
    Window* window = *(tagged_windows_.begin());
    ToggleWindowTag(window);

    Anchor* old_anchor = window->anchor();
    CHECK(old_anchor);
    Desktop* old_desktop = old_anchor->desktop();
    CHECK(old_desktop);

    if (anchor == old_anchor) continue;

    RemoveWindowFromDesktop(window, old_desktop);
    AddWindowToDesktop(window, desktop, anchor);
    if (!WindowShouldBeMapped(window)) window->Unmap();
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
  window->anchor()->DrawTitlebar();
}


void WindowManager::SetActiveAnchor(Anchor* anchor) {
  CHECK(anchor);
  CHECK(active_desktop_);
  active_desktop_->SetActiveAnchor(anchor);
  if (attach_follows_active_) active_desktop_->SetAttachAnchor(anchor);
}


bool WindowManager::IsAnchorWindow(XWindow* xwin) const {
  for (DesktopVector::const_iterator desktop = desktops_.begin();
       desktop != desktops_.end(); ++desktop) {
    if ((*desktop)->IsTitlebarWindow(xwin)) return true;
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

  Anchor* win_anchor = win->anchor();
  CHECK(win_anchor);
  Desktop* desktop = win_anchor->desktop();
  CHECK(desktop);

  int x = win->x() + win->width() / 2 - transient->width() / 2;
  int y = win->y() + win->height() / 2 - transient->height() / 2;
  Anchor* anchor = desktop->CreateAnchor("transient", x, y);
  anchor->set_temporary(true);
  AddWindowToDesktop(transient, desktop, anchor);
}


Window* WindowManager::GetActiveWindow() const {
  CHECK(active_desktop_);
  Anchor* anchor = active_desktop_->active_anchor();
  if (anchor == NULL) return NULL;
  return anchor->mutable_active_window();
}


Window* WindowManager::GetWindowByFrame(XWindow* xwin) const {
  for (WindowMap::const_iterator win = windows_.begin();
       win != windows_.end(); ++win) {
    if (win->second->frame() == xwin) return win->second.get();
  }
  return NULL;
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


bool WindowManager::WindowShouldBeMapped(Window* window) const {
  CHECK(window);
  return (window->anchor()->desktop() == active_desktop_ &&
          window->anchor()->active_window() == window);
}


}  // namespace wham
