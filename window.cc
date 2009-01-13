// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window.h"

#include <sstream>

#include "config.h"
#include "drawing-engine.h"
#include "x-server.h"
#include "x-window.h"

namespace wham {

Window::Window(XWindow* xwin)
    : xwin_(xwin),
      frame_(NULL),
      anchor_(NULL),
      props_(),
      configs_(),
      tagged_(false) {
  CHECK(xwin_);
  props_.UpdateAll(xwin_);

  uint border = Config::Get()->window_border;
  frame_ = XWindow::Create(
      xwin->x() - border, xwin->y() - border,
      xwin->width() + 2 * border, xwin->height() + 2 * border);

  // Map the client window but not the frame.
  xwin->Reparent(frame_, border, border);
  xwin->Map();

  Classify();
}


Window::~Window() {
  frame_->Destroy();
  xwin_ = NULL;
  frame_ = NULL;
}


void Window::CycleConfig(bool forward) {
  ApplyActiveConfig();
}


void Window::Move(int x, int y) {
  frame_->Move(x, y);
}


void Window::Resize(uint width, uint height) {
  DEBUG << "Resizing 0x" << hex << xwin_->id() << dec
        << " to (" << width << ", " << height << ")";
  uint border = Config::Get()->window_border;
  frame_->Resize(width + 2 * border, height + 2 * border);
  xwin_->Resize(width, height);
}


void Window::Map() {
  DrawFrame();
  frame_->Map();
}


void Window::Unmap() {
  frame_->Unmap();
}


void Window::TakeFocus() {
  DEBUG << "TakeFocus: 0x" << hex << xwin_->id();
  xwin_->TakeFocus();
}


void Window::Raise() {
  frame_->Raise();
}


void Window::MakeSibling(const XWindow& leader) {
  frame_->MakeSibling(leader);
}


void Window::HandlePropertyChange(
    WindowProperties::ChangeType type, bool* changed) {
  CHECK(changed);
  UpdateProperties(type, changed);
  if (*changed) {
    DEBUG << "Properties changed for 0x" << hex << xwin_->id()
          << "; reclassifying";
    Classify();
  }
}


void Window::DrawFrame() {
  DrawingEngine::Get()->DrawWindowFrame(frame_);
}


int Window::x() const { return frame_->x(); }


int Window::y() const { return frame_->y(); }


uint Window::width() const { return xwin_->width(); }


uint Window::height() const { return xwin_->height(); }


uint Window::frame_width() const { return frame_->width(); }


uint Window::frame_height() const { return frame_->height(); }


uint Window::id() const { return xwin_->id(); }


bool Window::Classify() {
  DEBUG << "Classifying window 0x" << hex << xwin_->id();
  if (!WindowClassifier::Get()->ClassifyWindow(props_, &configs_)) {
    ERROR << "Unable to classify window 0x" << hex << xwin_->id();
    return false;
  }
  ApplyActiveConfig();
  return true;
}


void Window::ApplyConfig(const WindowConfig& config) {
  DEBUG << "Applying config " << config.DebugString()
        << " to 0x" << hex << xwin_->id();

  uint width = 0;
  if (config.width_type == WindowConfig::DIMENSION_PIXELS) {
    width = config.width;
  } else if (config.width_type == WindowConfig::DIMENSION_UNITS) {
    width = props_.base_width + config.width * props_.width_inc;
  } else if (config.width_type == WindowConfig::DIMENSION_APP) {
    width = props_.width > 0 ? props_.width : xwin_->initial_width();
  } else if (config.width_type == WindowConfig::DIMENSION_MAX) {
    width = XServer::Get()->width(); // FIXME: ugly
  } else {
    ERROR << "Unknown width type " << config.width_type;
  }

  uint height = 0;
  if (config.height_type == WindowConfig::DIMENSION_PIXELS) {
    height = config.height;
  } else if (config.height_type == WindowConfig::DIMENSION_UNITS) {
    height = props_.base_height + config.height * props_.height_inc;
  } else if (config.height_type == WindowConfig::DIMENSION_APP) {
    height = props_.height > 0 ? props_.height : xwin_->initial_height();
  } else if (config.height_type == WindowConfig::DIMENSION_MAX) {
    height = XServer::Get()->height(); // FIXME: ugly
  } else {
    ERROR << "Unknown height type " << config.height_type;
  }

  if (props_.min_width > 0) {
    width = max(width, props_.min_width);
  }
  if (props_.max_width > 0) {
    width = min(width, props_.max_width);
  }

  if (props_.min_height > 0) {
    height = max(height, props_.min_height);
  }
  if (props_.max_height > 0) {
    height = min(height, props_.max_height);
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
  WindowProperties new_props(props_);
  if (!xwin_->UpdateProperties(&new_props, type)) return false;
  *changed = (new_props != props_);
  props_ = new_props;
  return true;
}

}  // namespace wham
