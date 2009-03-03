// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __MOCK_X_WINDOW_H__
#define __MOCK_X_WINDOW_H__

#include "x-window.h"

using namespace std;

namespace wham {

class MockXWindow : public XWindow {
 public:
  MockXWindow(::Window id);
  ~MockXWindow() {}

  bool UpdateProperties(WindowProperties* props,
                        WindowProperties::ChangeType type);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();
  void SelectEvents();
  void TakeFocus();
  void SetBorder(uint size);
  void Raise();
  void MakeSibling(const XWindow& leader);
  void Reparent(XWindow* parent, int x, int y);
  void WarpPointer(int x, int y);
  void GetGeometry(int* x,
                   int* y,
                   uint* width,
                   uint* height,
                   uint* border_width);
  void Destroy();

  bool mapped() { return mapped_; }

 private:
  ::Window id_;

  bool mapped_;
};

}  // namespace wham

#endif
