// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-anchor.h"

#include <limits>

#include "config.h"
#include "util.h"
#include "window.h"
#include "x.h"

namespace wham {

DEFINE_string(titlebar_font, "fixed");
DEFINE_int(titlebar_padding, 2);
DEFINE_int(titlebar_border, 1);


WindowAnchor::WindowAnchor(const string& name, int x, int y)
    : name_(),
      x_(x),
      y_(y),
      active_index_(0),
      active_window_(NULL),
      titlebar_(XWindow::Create(x, y, 1, 1)),
      titlebar_width_(0),
      titlebar_height_(0),
      name_width_(0),
      name_ascent_(0),
      name_descent_(0) {
  CHECK(titlebar_);
  SetName(name);
  DrawTitlebar();
  Move(x, y);
  titlebar_->Map();
}


WindowAnchor::~WindowAnchor() {
  active_window_ = NULL;
  titlebar_ = NULL;
}


void WindowAnchor::SetName(const string& name) {
  if (name_ == name) return;
  name_ = name;
  titlebar_->GetTextSize(config->titlebar_font, "[" + name_ + "]",
                         &name_width_, &name_ascent_, &name_descent_);
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
    if (!windows_.empty()) {
      SetActive(0);
    } else {
      DrawTitlebar();
    }
  }
}


bool WindowAnchor::Move(int x, int y) {
  x_ = x;
  y_ = y;
  titlebar_->Move(x, y - titlebar_height_);
  if (active_window_) active_window_->Move(x, y);
  return true;
}


bool WindowAnchor::SetActive(uint index) {
  CHECK(index >= 0);
  CHECK(index < windows_.size());

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

  DrawTitlebar();
  return true;
}


void WindowAnchor::DrawTitlebar() {
  int ascent = 0;

  if (windows_.empty()) {
    titlebar_width_ = name_width_ + 2 * config->titlebar_padding +
                      2 * config->titlebar_border;
    titlebar_width_ = min(max(titlebar_width_, config->titlebar_min_width),
                          config->titlebar_max_width);
    titlebar_height_ = name_ascent_ + name_descent_ +
                       2 * config->titlebar_padding +
                       2 * config->titlebar_border;
    LOG << "name_ascent_=" << name_ascent_
        << " name_descent_=" << name_descent_;
    ascent = name_ascent_;
  } else {
    int max_title_width = 0;
    int max_title_ascent = 0;
    int max_title_descent = 0;
    for (WindowVector::const_iterator window = windows_.begin();
         window != windows_.end(); ++window) {
      max_title_width = max(max_title_width, (*window)->title_width());
      max_title_ascent = max(max_title_ascent, (*window)->title_ascent());
      max_title_descent = max(max_title_descent, (*window)->title_descent());
    }
    titlebar_width_ = max_title_width * windows_.size() +
                      2 * config->titlebar_padding * windows_.size() +
                      config->titlebar_border * (windows_.size() + 1);
    titlebar_width_ = min(max(titlebar_width_, config->titlebar_min_width),
                          config->titlebar_max_width);
    titlebar_height_ = max_title_ascent + max_title_descent +
                       2 * config->titlebar_padding +
                       2 * config->titlebar_border;
    LOG << "max_title_ascent=" << max_title_ascent
        << " max_title_descent=" << max_title_descent;
    ascent = max_title_ascent;
  }

  //LOG << "before=" << titlebar_height_;
  titlebar_->Resize(titlebar_width_, titlebar_height_);
  //LOG << "after=" << titlebar_height_;
  titlebar_->Clear();
  titlebar_->DrawLine(0, 0, titlebar_width_ - 1, 0);
  titlebar_->DrawLine(0, titlebar_height_ - 1,
                      titlebar_width_ - 1, titlebar_height_ - 1);
  titlebar_->DrawLine(0, 0, 0, titlebar_height_ - 1);
  titlebar_->DrawLine(titlebar_width_ - 1, 0,
                      titlebar_width_ - 1, titlebar_height_ - 1);

  if (windows_.empty()) {
    titlebar_->DrawText(config->titlebar_border + config->titlebar_padding,
                        config->titlebar_border + config->titlebar_padding +
                          ascent, "[" + name_ + "]");
  } else {
    int title_width = (titlebar_width_ - config->titlebar_border) /
                      windows_.size();
    int i = 0;
    for (WindowVector::const_iterator window = windows_.begin();
         window != windows_.end(); ++window, ++i) {
      int x = i * title_width;
      int y = config->titlebar_border + config->titlebar_padding +
              ascent;
      titlebar_->DrawLine(x, 0, x, titlebar_height_ - 1);
      titlebar_->DrawText(
          x + config->titlebar_border + config->titlebar_padding, y,
          (*window)->title());
    }
  }

  titlebar_->Move(x_, y_ - titlebar_height_);
}

}  // namespace wham
