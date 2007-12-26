// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-anchor.h"

#include "window.h"
#include "x.h"

namespace wham {


const int WindowAnchor::kTitlebarHeight = 20;


WindowAnchor::WindowAnchor(const string& name, int x, int y)
    : name_(name),
      x_(x),
      y_(y),
      active_index_(0),
      active_window_(NULL),
      titlebar_(XWindow::Create(x, y, 300, kTitlebarHeight)) {
  CHECK(titlebar_);
  Move(x, y);
  titlebar_->Map();
}


void WindowAnchor::AddWindow(Window* window) {
  CHECK(find(windows_.begin(), windows_.end(), window) == windows_.end());
  windows_.push_back(window);
  if (!active_window_) SetActive(0);
}


void WindowAnchor::RemoveWindow(Window* window) {
  WindowVector::iterator it = find(windows_.begin(), windows_.end(), window);
  CHECK(it != windows_.end());
  windows_.erase(it);

  // If we removed the active window, we select a new one if possible.
  if (window == active_window_) {
    active_window_ = NULL;
    if (!windows_.empty()) SetActive(0);
  }
}


bool WindowAnchor::Move(int x, int y) {
  x_ = x;
  y_ = y;
  titlebar_->Move(x, y - kTitlebarHeight);
  if (active_window_) active_window_->Move(x, y);
  return true;
}


bool WindowAnchor::SetActive(uint index) {
  CHECK(index >= 0);
  CHECK(index <= windows_.size());

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
  active_window_->Move(x_, y_);
  active_window_->Map();
  return true;
}

}  // namespace wham
