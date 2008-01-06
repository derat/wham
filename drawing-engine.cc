// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "drawing-engine.h"

#include <cmath>

#include "anchor.h"
#include "config.h"
#include "window.h"
#include "x.h"

namespace wham {

ref_ptr<DrawingEngine> drawing_engine(NULL);

void DrawingEngine::DrawAnchor(const Anchor& anchor,
                               XWindow* titlebar,
                               uint* width,
                               uint* height) {
  CHECK(titlebar);
  CHECK(height);
  CHECK(width);

  int ascent = 0, descent = 0;
  XWindow::GetTextSize(config->titlebar_font, "X[yj",
                       NULL, &ascent, &descent);
  *height = ascent + descent +
      2 * config->titlebar_padding + 2 * config->titlebar_border;

  const vector<Window*>& windows = anchor.windows();
  if (windows.empty()) {
    int name_width = 0;
    XWindow::GetTextSize(config->titlebar_font, anchor.name(),
                         &name_width, NULL, NULL);
    *width = name_width +
        2 * config->titlebar_padding +
        2 * config->titlebar_border;
  } else {
    int max_title_width = 0;
    for (vector<Window*>::const_iterator window = windows.begin();
         window != windows.end(); ++window) {
      int title_width = 0;
      XWindow::GetTextSize(config->titlebar_font, (*window)->title(),
                           &title_width, NULL, NULL);
      max_title_width = max(max_title_width, title_width);
    }
    *width = max_title_width * windows.size() +
        2 * config->titlebar_padding * windows.size() +
        config->titlebar_border * (windows.size() + 1);
  }
  *width = min(max(*width, config->titlebar_min_width),
               config->titlebar_max_width);

  titlebar->Resize(*width, *height);
  titlebar->Clear();

  // Draw outside border.
  titlebar->DrawLine(0, 0, *width - 1, 0, "black");
  titlebar->DrawLine(0, *height - 1,
                     *width - 1, *height - 1, "black");
  titlebar->DrawLine(0, 0, 0, *height - 1, "black");
  titlebar->DrawLine(*width - 1, 0,
                     *width - 1, *height - 1, "black");

  if (windows.empty()) {
    titlebar->DrawText(config->titlebar_border + config->titlebar_padding,
                       config->titlebar_border + config->titlebar_padding +
                         ascent, "[" + anchor.name() + "]", "black");
  } else {
    float title_width =
        static_cast<float>(*width - config->titlebar_border) /
        windows.size();
    int i = 0;
    for (vector<Window*>::const_iterator window = windows.begin();
         window != windows.end(); ++window, ++i) {
      bool active = (anchor.active_window() == *window);
      int x = static_cast<int>(roundf(i * title_width));
      int y = config->titlebar_border + config->titlebar_padding + ascent;
      int win_width = static_cast<int>(roundf((i + 1) * title_width)) - x;
      if (active) {
        titlebar->DrawBox(x, 0, win_width, *height - 1, "black");
      } else {
        titlebar->DrawLine(x, 0, x, *height - 1, "black");
      }
      titlebar->DrawText(
          x + config->titlebar_border + config->titlebar_padding, y,
          (*window)->title(), active ? "white" : "black");
    }
  }
}

}  // namespace wham
