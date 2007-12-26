// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config.h"
#include "window.h"
#include "x.h"

namespace wham {

Window::Window(XWindow* x_window)
    : x_window_(x_window),
      props_(),
      configs_() {
  UpdateProperties();
}


bool Window::Classify(const WindowClassifier& classifier) {
  if (!classifier.ClassifyWindow(props_, &configs_)) {
    ERROR << "Unable to classify window";
    return false;
  }
  return ApplyConfig();
}


bool Window::Move(int x, int y) {
  return x_window_->Move(x, y);
}


bool Window::Resize(uint width, uint height) {
  return x_window_->Resize(width, height);
}


bool Window::Unmap() {
  return x_window_->Unmap();
}


bool Window::Map() {
  return x_window_->Map();
}


bool Window::ApplyConfig() {
  const WindowConfig* config = GetActiveConfig();
  CHECK(config);
  if (!Resize(config->width, config->height)) return false;
  return true;
}


bool Window::UpdateProperties() {
  CHECK(x_window_);
  if (!x_window_->GetProperties(&props_)) return false;
  x_window_->GetTextSize(config->titlebar_font, props_.window_name,
                         &title_width_, &title_ascent_, &title_descent_);
  return true;
}

}  // namespace wham
