// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "mock-x.h"

using namespace std;

namespace wham {

MockXWindow::MockXWindow(::Window id)
    : XWindow(id) {
}


void MockXWindow::Clear() {
}


void MockXWindow::DrawText(
    int x, int y, const string& text, const string& color) {
}


void MockXWindow::DrawLine(
    int x1, int y1, int x2, int y2, const string& color) {
}


void MockXWindow::DrawBox(
    int x, int y, uint width, uint height, const string& color) {
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

}  // namespace wham
