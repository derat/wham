// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "anchor.h"

#include <cmath>
#include <limits>

#include "config.h"
#include "drawing-engine.h"
#include "util.h"
#include "window.h"
#include "x.h"

namespace wham {

Anchor::Anchor(const string& name, int x, int y)
    : name_(),
      x_(x),
      y_(y),
      active_index_(0),
      active_window_(NULL),
      gravity_(TOP_LEFT),
      titlebar_(XWindow::Create(x, y, 1, 1)) {
  CHECK(titlebar_);
  SetName(name);
  DrawTitlebar();
  Move(x, y);
  titlebar_->Map();
}


Anchor::~Anchor() {
  active_window_ = NULL;
  titlebar_ = NULL;
}


void Anchor::SetName(const string& name) {
  if (name_ == name) return;
  name_ = name;
}


void Anchor::AddWindow(Window* window) {
  CHECK(find(windows_.begin(), windows_.end(), window) == windows_.end());
  windows_.push_back(window);
  if (!active_window_) SetActive(0);
}


void Anchor::RemoveWindow(Window* window) {
  WindowVector::iterator it = find(windows_.begin(), windows_.end(), window);
  CHECK(it != windows_.end());
  windows_.erase(it);

  // If we removed the active window, we select a new one if possible.
  if (window == active_window_) {
    active_window_ = NULL;
    if (!windows_.empty()) {
      size_t new_index = active_index_;
      if (new_index >= windows_.size()) new_index = windows_.size() - 1;
      SetActive(new_index);
    } else {
      DrawTitlebar();
    }
  }
}


bool Anchor::Move(int x, int y) {
  x_ = x;
  y_ = y;

  UpdateTitlebarPosition();
  if (active_window_) UpdateWindowPosition(active_window_);
  return true;
}


bool Anchor::SetActive(uint index) {
  if (index < 0 || index >= windows_.size()) {
    ERROR << "Ignoring request to activate window " << index
          << " in anchor " << name_ << " containing " << windows_.size()
          << " window(s)";
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
  active_window_->Map();
  active_window_->TakeFocus();

  DrawTitlebar();
  return true;
}


void Anchor::DrawTitlebar() {
  DrawingEngine::Get()->DrawAnchor(*this, titlebar_);
  UpdateTitlebarPosition();
}


void Anchor::ActivateWindowAtTitlebarCoordinates(int x, int y) {
  if (windows_.empty()) return;
  int index = (x - x_) * windows_.size() / titlebar_->width();
  SetActive(index);
}


void Anchor::FocusActiveWindow() {
  if (!active_window_) return;
  active_window_->TakeFocus();
}


void Anchor::CycleActiveWindowConfig(bool forward) {
  if (!active_window_) return;
  active_window_->CycleConfig(forward);
  UpdateWindowPosition(active_window_);
}


void Anchor::SetGravity(Anchor::Gravity gravity) {
  if (gravity_ == gravity) return;
  gravity_ = gravity;
  UpdateTitlebarPosition();
  if (active_window_) UpdateWindowPosition(active_window_);
}


void Anchor::CycleGravity(bool forward) {
  SetGravity(static_cast<Gravity>(
      (gravity_ + NUM_GRAVITIES + (forward ? 1 : -1)) % NUM_GRAVITIES));
}


void Anchor::UpdateTitlebarPosition() {
  int x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      x_ :
      x_ - titlebar_->width();
  int y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      y_ :
      y_ - titlebar_->height();
  titlebar_->Move(x, y);
}


void Anchor::UpdateWindowPosition(Window* window) {
  CHECK(window);
  uint border = Config::Get()->window_border;
  int x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      x_ :
      x_ - window->width() - 2 * border;
  int y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      y_ + titlebar_->height() :
      y_ - titlebar_->height() - window->height() - 2 * border;
  window->Move(x, y);
}

}  // namespace wham
