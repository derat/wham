// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "mock-x-window.h"

using namespace std;

namespace wham {

MockXWindow::MockXWindow(::Window id)
    : XWindow(id),
      mapped_(false) {
}


bool MockXWindow::UpdateProperties(WindowProperties* props,
                                   WindowProperties::ChangeType type) {
  return true;
}


void MockXWindow::Move(int x, int y) {
  x_ = x;
  y_ = y;
}


void MockXWindow::Resize(uint width, uint height) {
  width_ = width;
  height_ = height;
}


void MockXWindow::Unmap() {
  mapped_ = false;
}


void MockXWindow::Map() {
  mapped_ = true;
}


void MockXWindow::SelectEvents() {
}


void MockXWindow::TakeFocus() {
}


void MockXWindow::SetBorder(uint size) {
}


void MockXWindow::Raise() {
}


void MockXWindow::MakeSibling(XWindow* leader) {
}


void MockXWindow::Reparent(XWindow* parent, int x, int y) {
  parent_ = parent;
  Move(x, y);
}


void MockXWindow::WarpPointer(int x, int y) {
}


void MockXWindow::GetGeometry(int* x,
                              int* y,
                              uint* width,
                              uint* height,
                              uint* border_width) {
}


void MockXWindow::Destroy() {
  // TODO: Maybe call XServer::DeleteWindow() here.
}

}  // namespace wham
