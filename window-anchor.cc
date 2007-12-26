// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-anchor.h"

#include "util.h"
#include "window.h"
#include "x.h"

namespace wham {

DEFINE_string(titlebar_font, "fixed");
DEFINE_int(titlebar_padding, 2);
DEFINE_int(titlebar_border, 1);


WindowAnchor::WindowAnchor(const string& name, int x, int y)
    : name_(name),
      x_(x),
      y_(y),
      active_index_(0),
      active_window_(NULL),
      titlebar_(XWindow::Create(x, y, 1, 1)),
      titlebar_width_(0),
      titlebar_height_(0) {
  CHECK(titlebar_);
  DrawTitlebar();
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
  string title = active_window_ ? active_window_->title() : name_;
  int text_width = 0;
  int text_ascent = 0;
  int text_descent = 0;
  titlebar_->GetTextSize(SETTING_titlebar_font, title,
                         &text_width, &text_ascent, &text_descent);
  LOG << "title=" << title << " width=" << text_width
      << " ascent=" << text_ascent << " descent=" << text_descent;

  titlebar_width_ = text_width + 2 * SETTING_titlebar_padding +
                    2 * SETTING_titlebar_border;
  titlebar_height_ = text_ascent + text_descent +
                     2 * SETTING_titlebar_padding +
                     2 * SETTING_titlebar_border;
  //LOG << "before=" << titlebar_height_;
  titlebar_->Resize(titlebar_width_, titlebar_height_);
  //LOG << "after=" << titlebar_height_;
  titlebar_->Clear();
  titlebar_->DrawText(SETTING_titlebar_border + SETTING_titlebar_padding,
                      SETTING_titlebar_border + SETTING_titlebar_padding +
                        text_ascent,
                      title);
  titlebar_->Move(x_, y_ - titlebar_height_);
}

}  // namespace wham
