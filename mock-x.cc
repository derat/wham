// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "mock-x.h"

using namespace std;

namespace wham {

MockXWindow::MockXWindow(::Window id)
    : XWindow(id) {
}


bool MockXWindow::GetProperties(WindowProperties* props) {
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
}


void MockXWindow::Map() {
}


void MockXWindow::SelectEvents() {
}


void MockXWindow::TakeFocus() {
}


void MockXWindow::SetBorder(uint size) {
}


void MockXWindow::Raise() {
}


void MockXWindow::MakeSibling(const XWindow& leader) {
}


void MockXWindow::GetGeometry(int* x,
                              int* y,
                              uint* width,
                              uint* height,
                              uint* border_width) {
}

}  // namespace wham
