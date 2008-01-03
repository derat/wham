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

  void Clear();
  void DrawText(int x, int y, const string& text, const string& color);
  void DrawLine(int x1, int y1, int x2, int y2, const string& color);
  void DrawBox(int x, int y, uint width, uint height, const string& color);

  bool GetProperties(WindowProperties* props);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Unmap();
  void Map();

 private:
  friend class XServer;

  ::Window id_;

  static XServer* server_;
};

}  // namespace wham

#endif
