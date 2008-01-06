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
}


void MockXWindow::Resize(uint width, uint height) {
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

}  // namespace wham
