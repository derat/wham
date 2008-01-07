// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window.h"

#include "config.h"
#include "x.h"

namespace wham {

WindowClassifier* Window::classifier_ = NULL;


Window::Window(XWindow* x_window)
    : x_window_(x_window),
      width_(0),
      height_(0),
      props_(),
      configs_() {
  UpdateProperties();
  Classify();
}


void Window::Move(int x, int y) {
  x_window_->Move(x, y);
}


void Window::Resize(uint width, uint height) {
  x_window_->Resize(width, height);
  width_ = width;
  height_ = height;
}


void Window::Unmap() {
  x_window_->Unmap();
}


void Window::Map() {
  x_window_->Map();
}


void Window::TakeFocus() {
  x_window_->TakeFocus();
}


bool Window::Classify() {
  CHECK(classifier_);
  if (!classifier_->ClassifyWindow(props_, &configs_)) {
    ERROR << "Unable to classify window";
    return false;
  }
  ApplyConfig();
  return true;
}


void Window::ApplyConfig() {
  const WindowConfig* config = configs_.GetActiveConfig();
  CHECK(config);
  Resize(config->width, config->height);
}


bool Window::UpdateProperties() {
  CHECK(x_window_);
  if (!x_window_->GetProperties(&props_)) return false;
  return true;
}

}  // namespace wham
