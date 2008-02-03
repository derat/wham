// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window.h"

#include "x.h"

namespace wham {

Window::Window(XWindow* x_window)
    : x_window_(x_window),
      width_(0),
      height_(0),
      props_(),
      configs_(),
      tagged_(false) {
  CHECK(x_window_);
  x_window_->GetGeometry(NULL, NULL, &width_, &height_, NULL);
  HandlePropertyChange();
}


void Window::CycleConfig(bool forward) {
  configs_.CycleActiveConfig(forward);
  ApplyConfig();
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


void Window::HandlePropertyChange() {
  UpdateProperties();
  Classify();
}


int Window::x() const { return x_window_->x(); }


int Window::y() const { return x_window_->y(); }


uint Window::width() const { return x_window_->width(); }


uint Window::height() const { return x_window_->height(); }


bool Window::Classify() {
  if (!WindowClassifier::Get()->ClassifyWindow(props_, &configs_)) {
    ERROR << "Unable to classify window";
    return false;
  }
  ApplyConfig();
  return true;
}


void Window::ApplyConfig() {
  const WindowConfig* config = configs_.GetActiveConfig();
  CHECK(config);
  // FIXME: interpret non-pixel dimensions correctly
  Resize(config->width, config->height);
}


bool Window::UpdateProperties() {
  if (!x_window_->GetProperties(&props_)) return false;
  return true;
}

}  // namespace wham
