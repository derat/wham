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
  ApplyConfig();
  return true;
}


void Window::Move(int x, int y) {
  x_window_->Move(x, y);
}


void Window::Resize(uint width, uint height) {
  x_window_->Resize(width, height);
}


void Window::Unmap() {
  x_window_->Unmap();
}


void Window::Map() {
  x_window_->Map();
}


void Window::ApplyConfig() {
  const WindowConfig* config = GetActiveConfig();
  CHECK(config);
  Resize(config->width, config->height);
}


bool Window::UpdateProperties() {
  CHECK(x_window_);
  if (!x_window_->GetProperties(&props_)) return false;
  x_window_->GetTextSize(config->titlebar_font, props_.window_name,
                         &title_width_, NULL, NULL);
  return true;
}

}  // namespace wham
