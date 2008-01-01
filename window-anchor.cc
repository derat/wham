// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-anchor.h"

#include <cmath>
#include <limits>

#include "config.h"
#include "util.h"
#include "window.h"
#include "x.h"

namespace wham {

DEFINE_string(titlebar_font, "fixed");
DEFINE_int(titlebar_padding, 2);
DEFINE_int(titlebar_border, 1);


int WindowAnchor::font_ascent_ = 0;
int WindowAnchor::font_descent_ = 0;


WindowAnchor::WindowAnchor(const string& name, int x, int y)
    : name_(),
      x_(x),
      y_(y),
      active_index_(0),
      active_window_(NULL),
      gravity_(TOP_LEFT),
      titlebar_(XWindow::Create(x, y, 1, 1)),
      titlebar_width_(0),
      titlebar_height_(0),
      name_width_(0),
      name_ascent_(0),
      name_descent_(0) {
  CHECK(titlebar_);
  // FIXME: move this somewhere else; it needs to be done whenever the font
  // changes
  XWindow::GetTextSize(config->titlebar_font, "X[yj",
                       NULL, &font_ascent_, &font_descent_);
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
                         &name_width_, NULL, NULL);
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
      size_t new_index = active_index_;
      if (new_index >= windows_.size()) new_index = windows_.size() - 1;
      SetActive(new_index);
    } else {
      DrawTitlebar();
    }
  }
}


bool WindowAnchor::Move(int x, int y) {
  x_ = x;
  y_ = y;

  titlebar_->Move(x, y);
  if (active_window_) UpdateWindowPosition(active_window_);
  return true;
}


bool WindowAnchor::SetActive(uint index) {
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

  DrawTitlebar();
  return true;
}


void WindowAnchor::DrawTitlebar() {
  titlebar_height_ = font_ascent_ + font_descent_ +
      2 * config->titlebar_padding + 2 * config->titlebar_border;
  if (windows_.empty()) {
    titlebar_width_ = name_width_ + 2 * config->titlebar_padding +
                      2 * config->titlebar_border;
  } else {
    int max_title_width = 0;
    for (WindowVector::const_iterator window = windows_.begin();
         window != windows_.end(); ++window) {
      max_title_width = max(max_title_width, (*window)->title_width());
    }
    titlebar_width_ = max_title_width * windows_.size() +
        2 * config->titlebar_padding * windows_.size() +
        config->titlebar_border * (windows_.size() + 1);
  }
  titlebar_width_ = min(max(titlebar_width_, config->titlebar_min_width),
                        config->titlebar_max_width);

  //LOG << "before=" << titlebar_height_;
  titlebar_->Resize(titlebar_width_, titlebar_height_);
  //LOG << "after=" << titlebar_height_;
  titlebar_->Clear();
  titlebar_->DrawLine(0, 0, titlebar_width_ - 1, 0, "black");
  titlebar_->DrawLine(0, titlebar_height_ - 1,
                      titlebar_width_ - 1, titlebar_height_ - 1, "black");
  titlebar_->DrawLine(0, 0, 0, titlebar_height_ - 1, "black");
  titlebar_->DrawLine(titlebar_width_ - 1, 0,
                      titlebar_width_ - 1, titlebar_height_ - 1, "black");

  if (windows_.empty()) {
    titlebar_->DrawText(config->titlebar_border + config->titlebar_padding,
                        config->titlebar_border + config->titlebar_padding +
                          font_ascent_, "[" + name_ + "]", "black");
  } else {
    float title_width =
        static_cast<float>(titlebar_width_ - config->titlebar_border) /
        windows_.size();
    int rounded_width = static_cast<int>(roundf(title_width));
    int i = 0;
    for (WindowVector::const_iterator window = windows_.begin();
         window != windows_.end(); ++window, ++i) {
      bool active = (active_window_ == *window);
      int x = static_cast<int>(roundf(i * title_width));
      int y = config->titlebar_border + config->titlebar_padding +
              font_ascent_;
      if (active) {
        titlebar_->DrawBox(x, 0, rounded_width, titlebar_height_ - 1, "black");
      } else {
        titlebar_->DrawLine(x, 0, x, titlebar_height_ - 1, "black");
      }
      titlebar_->DrawText(
          x + config->titlebar_border + config->titlebar_padding, y,
          (*window)->title(), active ? "white" : "black");
    }
  }
}


void WindowAnchor::ActivateWindowAtCoordinates(int x, int y) {
  if (windows_.empty()) return;
  int index = (x - x_) * windows_.size() / titlebar_width_;
  SetActive(index);
}


void WindowAnchor::SetGravity(WindowAnchor::Gravity gravity) {
  if (gravity_ == gravity) return;
  gravity_ = gravity;
  Move(x_, y_);
}


void WindowAnchor::UpdateWindowPosition(Window* window) {
  CHECK(window);
  int x = (gravity_ == TOP_LEFT || gravity_ == BOTTOM_LEFT) ?
      x_ : x_ + titlebar_width_ - window->width();
  int y = (gravity_ == TOP_LEFT || gravity_ == TOP_RIGHT) ?
      y_ + titlebar_height_ : y_ - window->height();
  LOG << "y_=" << y_ << " titlebar_height=" << titlebar_height_
      << " y=" << y;
  window->Move(x, y);
}

}  // namespace wham
