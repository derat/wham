// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __MOCK_X_H__
#define __MOCK_X_H__

#include "x.h"

using namespace std;

namespace wham {

class MockXWindow : public XWindow {
 public:
  MockXWindow(::Window id);
  ~MockXWindow() {}

  bool GetProperties(WindowProperties* props);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();
  void SelectEvents();
  void TakeFocus();
  void SetBorder(uint size);

 private:
  ::Window id_;
};

}  // namespace wham

#endif
