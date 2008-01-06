// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __DRAWING_ENGINE_H__
#define __DRAWING_ENGINE_H__

#include <vector>

#include "util.h"

using namespace std;

namespace wham {

class Anchor;   // from anchor.h
class Window;   // from window.h
class XWindow;  // from x.h
class XServer;  // from x.h

class DrawingEngine;
extern ref_ptr<DrawingEngine> drawing_engine;

class DrawingEngine {
 public:
  DrawingEngine(XServer* x_server)
      : x_server_(x_server) {
  }

  static void Swap(ref_ptr<DrawingEngine> new_drawing_engine) {
    drawing_engine.swap(new_drawing_engine);
  }

  // Draw an anchor's titlebar.
  // The titlebar's dimensions are stored in 'titlebar_width' and
  // 'titlebar_height'.
  void DrawAnchor(const Anchor& anchor,
                  XWindow* titlebar,
                  uint* titlebar_width,
                  uint* titlebar_height);

 private:
  XServer* x_server_;

  DISALLOW_EVIL_CONSTRUCTORS(DrawingEngine);
};

}  // namespace wham

#endif
