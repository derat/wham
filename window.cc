// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

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


bool Window::Resize(int width, int height) {
  return x_window_->Resize(width, height);
}


bool Window::ApplyConfig() {
  const WindowConfig* config = GetCurrentConfig();
  CHECK(config);
  if (!Resize(config->width, config->height)) return false;
  return true;
}


bool Window::UpdateProperties() {
  CHECK(x_window_);
  if (!x_window_->GetProperties(&props_)) return false;
  return true;
}

}  // namespace wham
