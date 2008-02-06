// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window.h"

#include <sstream>
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
  props_.UpdateAll(x_window_);
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


void Window::Raise() {
  x_window_->Raise();
}


void Window::MakeSibling(const XWindow& leader) {
  x_window_->MakeSibling(leader);
}


void Window::HandlePropertyChange(WindowProperties::ChangeType type) {
  bool changed = false;
  UpdateProperties(type, &changed);
  if (changed) {
    DEBUG << "Properties changed; reclassifying";
    Classify();
  }
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

  uint width = 0;
  if (config->width_type == WindowConfig::DIMENSION_PIXELS) {
    width = config->width;
  } else if (config->width_type == WindowConfig::DIMENSION_UNITS) {
    width = props_.base_width + config->width * props_.width_inc;
  } else if (config->width_type == WindowConfig::DIMENSION_APP) {
    width = props_.width;
  } else if (config->width_type == WindowConfig::DIMENSION_MAX) {
    width = XServer::Get()->width(); // FIXME: ugly
  } else {
    ERROR << "Unknown width type " << config->width_type;
  }

  uint height = 0;
  if (config->height_type == WindowConfig::DIMENSION_PIXELS) {
    height = config->height;
  } else if (config->height_type == WindowConfig::DIMENSION_UNITS) {
    height = props_.base_height + config->height * props_.height_inc;
  } else if (config->height_type == WindowConfig::DIMENSION_APP) {
    height = props_.height;
  } else if (config->height_type == WindowConfig::DIMENSION_MAX) {
    height = XServer::Get()->height(); // FIXME: ugly
  } else {
    ERROR << "Unknown height type " << config->height_type;
  }

  if (width <= 0 || height <= 0) {
    ERROR << "Not resizing to (" << width << ", " << height << ")";
    return;
  }
  Resize(width, height);
}


bool Window::UpdateProperties(WindowProperties::ChangeType type,
                              bool* changed) {
  CHECK(changed);
  WindowProperties new_props;
  if (!x_window_->UpdateProperties(&new_props, type)) return false;
  *changed = (new_props != props_);
  props_ = new_props;
  return true;
}

}  // namespace wham
