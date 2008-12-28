// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "anchor.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "config.h"
#include "drawing-engine.h"
#include "util.h"
#include "window.h"
#include "x-server.h"
#include "x-window.h"

namespace wham {

Anchor::Anchor(const string& name, int x, int y)
    : name_(),
      x_(x),
      y_(y),
      temporary_(false),
      active_index_(0),
      active_window_(NULL),
      gravity_(TOP_LEFT),
      titlebar_(XWindow::Create(x, y, 1, 1)),
      active_(false),
      attach_(false) {
  CHECK(titlebar_);
  SetName(name);
  DrawTitlebar();
  Move(x, y);
  titlebar_->Map();
}


Anchor::~Anchor() {
  titlebar_->Destroy();
  active_window_ = NULL;
  titlebar_ = NULL;
}


void Anchor::Hide() {
  titlebar_->Unmap();
  if (active_window_) active_window_->Unmap();
}


void Anchor::Show() {
  titlebar_->Map();
  if (active_window_) active_window_->Map();
}


void Anchor::SetName(const string& name) {
  if (name_ == name) return;
  name_ = name;
}


void Anchor::AddWindow(Window* window) {
  DEBUG << "AddWindow: anchor=" << this
        << " window=0x" << hex << window->id();
  CHECK(find(windows_.begin(), windows_.end(), window) == windows_.end());
  windows_.push_back(window);
  if (!active_window_) SetActiveWindow(0);
}


void Anchor::RemoveWindow(Window* window) {
  DEBUG << "RemoveWindow: anchor=" << this
        << " window=0x" << hex << window->id();
  WindowVector::iterator it = find(windows_.begin(), windows_.end(), window);
  CHECK(it != windows_.end());
  windows_.erase(it);

  // If we removed the active window, we select a new one if possible.
  if (window == active_window_) {
    active_window_ = NULL;
    if (!windows_.empty()) {
      // FIXME: Maybe it'd make more sense to set new_index to
      // active_index_ - 1, capping it at 0 -- that's what Ion does.
      size_t new_index = active_index_;
      if (new_index >= windows_.size()) new_index = windows_.size() - 1;
      SetActiveWindow(new_index);
    } else {
      DrawTitlebar();
    }
  }
}


void Anchor::Move(int x, int y) {
  // Constrain the anchor within the root window's dimensions.
  CHECK(titlebar_->width() > 0 && titlebar_->height() > 0);
  int min_x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      0 : titlebar_->width();
  int max_x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      XServer::Get()->width() - titlebar_->width() :
      XServer::Get()->width();
  int min_y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      0 : titlebar_->height();
  int max_y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      XServer::Get()->height() - titlebar_->height() :
      XServer::Get()->height();
  x = min(max(min_x, x), max_x);
  y = min(max(min_y, y), max_y);

  x_ = x;
  y_ = y;

  UpdateTitlebarPosition();
  if (active_window_) UpdateWindowPosition(active_window_);
}


void Anchor::Raise() {
  titlebar_->Raise();
  if (active_window_) active_window_->MakeSibling(*titlebar_);
}


void Anchor::SetActive(bool active) {
  if (active == active_) return;
  active_ = active;
  DrawTitlebar();
  if (active_window_) active_window_->TakeFocus();
}


void Anchor::SetAttach(bool attach) {
  if (attach == attach_) return;
  attach_ = attach;
  DrawTitlebar();
}


bool Anchor::SetActiveWindow(uint index) {
  DEBUG << "SetActiveWindow: anchor=" << this << " index=" << index;
  if (index < 0 || index >= windows_.size()) {
    ERROR << "Ignoring request to activate window " << index
          << " in anchor 0x" << hex << this << " containing " << dec
          << windows_.size() << " window(s)";
    return false;
  }

  // If we're already displaying the correct window, we don't need to do
  // anything (except updating the index if it's changed).
  if (windows_[index] == active_window_) {
    if (index != active_index_) active_index_ = index;
    return true;
  }

  // Otherwise, we need to show a new window.  If another window is already
  // being shown, unmap it first.
  if (active_window_ != NULL) active_window_->Unmap();

  // Then, move the new window to the correct location and map it.
  active_index_ = index;
  active_window_ = windows_[active_index_];
  CHECK(active_window_);
  UpdateWindowPosition(active_window_);
  active_window_->MakeSibling(*titlebar_);
  active_window_->Map();
  active_window_->TakeFocus();

  DrawTitlebar();
  return true;
}


void Anchor::ShiftActiveWindow(bool shift_right) {
  if (windows_.size() <= 1) return;
  if ((active_index_ == 0 && !shift_right) ||
      ((active_index_ == windows_.size() - 1) && shift_right)) {
    return;
  }

  int new_index = active_index_ + (shift_right ? 1 : -1);
  CHECK_EQ(active_window_, windows_[active_index_]);

  windows_[active_index_] = windows_[new_index];
  windows_[new_index] = active_window_;
  active_index_ = new_index;
  DrawTitlebar();
}


void Anchor::DrawTitlebar() {
  DrawingEngine::Get()->DrawAnchor(*this, titlebar_);
  // Move the window to its current position to handle the case where the
  // titlebar might've been cut off.
  Move(x_, y_);
}


int Anchor::GetWindowIndexAtTitlebarPoint(int abs_x) {
  if (windows_.empty()) return -1;
  if (abs_x < titlebar_->x()) {
    ERROR << "Point falls outside of the titlebar (" << abs_x
          << " vs. " << titlebar_->x() << "); capping to "
          << titlebar_->x();
    abs_x = titlebar_->x();
  } else if (abs_x >= titlebar_->x() + static_cast<int>(titlebar_->width())) {
    ERROR << "Point falls outside of the titlebar (" << abs_x
          << " vs. " << titlebar_->x() << "+" << titlebar_->width()
          << "); capping to " << (titlebar_->x() + titlebar_->width() - 1);
    abs_x = titlebar_->x() + titlebar_->width() - 1;
  }
  return (abs_x - titlebar_->x()) * windows_.size() / titlebar_->width();
}


void Anchor::CycleActiveWindow(bool cycle_right) {
  if (windows_.size() <= 1) return;
  uint new_index =
      (active_index_ + windows_.size() + 2 * cycle_right - 1) % windows_.size();
  SetActiveWindow(new_index);
}


void Anchor::CycleActiveWindowConfig(bool forward) {
  if (!active_window_) return;
  active_window_->CycleConfig(forward);
  UpdateWindowPosition(active_window_);
}


void Anchor::SetGravity(Anchor::Gravity gravity) {
  if (gravity_ == gravity) return;

  int old_titlebar_x, old_titlebar_y;
  GetTitlebarPosition(&old_titlebar_x, &old_titlebar_y);

  gravity_ = gravity;

  // Get the offset in the titlebar's position so we can move it back to
  // the same place it was before.
  int new_titlebar_x, new_titlebar_y;
  GetTitlebarPosition(&new_titlebar_x, &new_titlebar_y);
  Move(x_ - (new_titlebar_x - old_titlebar_x),
       y_ - (new_titlebar_y - old_titlebar_y));
}


void Anchor::CycleGravity(bool forward) {
  SetGravity(static_cast<Gravity>(
      (gravity_ + NUM_GRAVITIES + (forward ? 1 : -1)) % NUM_GRAVITIES));
}


bool Anchor::TitlebarIsOverPoint(int x, int y) const {
  return titlebar_->x() <= x &&
         titlebar_->x() + static_cast<int>(titlebar_->width()) >= x &&
         titlebar_->y() <= y &&
         titlebar_->y() + static_cast<int>(titlebar_->height()) >= y;
}


void Anchor::GetGravityDirection(Gravity gravity, int* dx, int* dy) {
  CHECK(dx);
  CHECK(dy);
  if (gravity == TOP_LEFT) {
    *dx = -1; *dy = -1;
  } else if (gravity == TOP_RIGHT) {
    *dx = 1; *dy = -1;
  } else if (gravity == BOTTOM_LEFT) {
    *dx = -1; *dy = 1;
  } else if (gravity == BOTTOM_RIGHT) {
    *dx = 1; *dy = 1;
  } else {
    ERROR << "Unknown gravity " << gravity;
  }
}


void Anchor::UpdateTitlebarPosition() {
  int x, y;
  GetTitlebarPosition(&x, &y);
  titlebar_->Move(x, y);
}


void Anchor::UpdateWindowPosition(Window* window) {
  CHECK(window);
  int x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      x_ :
      x_ - window->frame_width();
  int y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      y_ + titlebar_->height() :
      y_ - titlebar_->height() - window->frame_height();
  window->Move(x, y);
}


void Anchor::GetTitlebarPosition(int* x, int* y) {
  *x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      x_ :
      x_ - titlebar_->width();
  *y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      y_ :
      y_ - titlebar_->height();
}

}  // namespace wham
