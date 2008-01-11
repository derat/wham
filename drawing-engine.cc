// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "drawing-engine.h"

#include <cmath>

#include "anchor.h"
#include "config.h"
#include "window.h"
#include "x.h"

namespace wham {

ref_ptr<DrawingEngine> DrawingEngine::singleton_(new DrawingEngine);



const DrawingEngine::Style::StringDef DrawingEngine::Style::string_defs_[] = {
  { FOCUSED_ANCHOR__BACKGROUND,
    "focused_anchor.background", "#bbbbbb" },
  { FOCUSED_ANCHOR__ACTIVE_WINDOW__BACKGROUND,
    "focused_anchor.active_window.background", "#ff0000" },
  { FOCUSED_ANCHOR__INACTIVE_WINDOW__BACKGROUND,
    "focused_anchor.inactive_window.background", "#bbbbbb" },
  { INVALID_TYPE, NULL, NULL },
};

const DrawingEngine::Style::UintDef DrawingEngine::Style::uint_defs_[] = {
  { FOCUSED_ANCHOR__BORDER_WIDTH,
    "focused_anchor.border_width", 1 },
  { FOCUSED_ANCHOR__PADDING,
    "focused_anchor.padding", 2 },
  { FOCUSED_ANCHOR__WINDOW_SPACING,
    "focused_anchor.window_spacing", 1 },
  { FOCUSED_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH,
    "focused_anchor.active_window.border_width", 1 },
  { FOCUSED_ANCHOR__ACTIVE_WINDOW__PADDING,
    "focused_anchor.active_window.padding", 3 },
  { FOCUSED_ANCHOR__INACTIVE_WINDOW__BACKGROUND,
    "focused_anchor.inactive_window.border_width", 1 },
  { FOCUSED_ANCHOR__INACTIVE_WINDOW__PADDING,
    "focused_anchor.inactive_window.padding", 3 },
  { INVALID_TYPE, NULL, 0 },
};

const DrawingEngine::Style::ColorsDef
    DrawingEngine::Style::colors_defs_[] = {
  { FOCUSED_ANCHOR__BORDER_COLOR,
    "focused_anchor.border_color",
    "#dddddd", "#dddddd", "#999999", "#999999" },
  { FOCUSED_ANCHOR__ACTIVE_WINDOW__BORDER_COLOR,
    "focused_anchor.active_window.border_color",
    "#dddddd", "#dddddd", "#999999", "#999999" },
  { FOCUSED_ANCHOR__INACTIVE_WINDOW__BORDER_COLOR,
    "focused_anchor.inactive_window.border_color",
    "#dddddd", "#dddddd", "#999999", "#999999" },
  { INVALID_TYPE, NULL, NULL, NULL, NULL, NULL },
};

bool DrawingEngine::Style::initialized_ = false;

// A string that hopefully measures the full ascent and descent for a given
// font.
static const string kFullHeightString = "X[yj|";


DrawingEngine::DrawingEngine()
    : initialized_(false),
      gc_(0),
      gc_font_(NULL),
      style_(new Style) {
}


void DrawingEngine::DrawAnchor(const Anchor& anchor,
                               XWindow* titlebar,
                               uint* width,
                               uint* height) {
  CHECK(titlebar);
  CHECK(height);
  CHECK(width);

  InitIfNeeded();
  ::Window win = titlebar->id();

  // Don't do anything if there's no real X connection.
  if (XServer::Testing()) {
    return;
  }

  Config* c = Config::Get();
  const uint border = c->anchor_border_width;
  const uint padding = c->anchor_padding;
  const string& border_color = c->anchor_border_color;
  const string& font = c->anchor_font;

  int ascent = 0, descent = 0;
  GetTextSize(font, kFullHeightString, NULL, &ascent, &descent);
  *height = ascent + descent + 2 * padding + 2 * border;

  const vector<Window*>& windows = anchor.windows();
  if (windows.empty()) {
    int name_width = 0;
    GetTextSize(font, anchor.name(), &name_width, NULL, NULL);
    *width = name_width + 2 * padding + 2 * border;
  } else {
    int max_title_width = 0;
    for (vector<Window*>::const_iterator window = windows.begin();
         window != windows.end(); ++window) {
      int title_width = 0;
      GetTextSize(font, (*window)->title(), &title_width, NULL, NULL);
      max_title_width = max(max_title_width, title_width);
    }
    *width = max_title_width * windows.size() +
        2 * padding * windows.size() +
        border * (windows.size() + 1);
  }
  *width = min(max(*width, c->anchor_min_width), c->anchor_max_width);

  titlebar->SetBorder(0);
  titlebar->Resize(*width, *height);
  Clear(win);
  // FIXME: set the background on init and just clear the window instead
  DrawBox(win, 0, 0, *width - 1, *height - 1, c->anchor_inactive_bg_color);

  // Draw outside border.
  DrawLine(win, 0, 0, *width - 1, 0, border_color);
  DrawLine(win, 0, *height - 1, *width - 1, *height - 1, border_color);
  DrawLine(win, 0, 0, 0, *height - 1, border_color);
  DrawLine(win, *width - 1, 0, *width - 1, *height - 1, border_color);

  if (windows.empty()) {
    DrawText(win, border + padding, border + padding + ascent,
             "[" + anchor.name() + "]", c->anchor_inactive_text_color, font);
  } else {
    float title_width = static_cast<float>(*width - border) / windows.size();
    int i = 0;
    for (vector<Window*>::const_iterator window = windows.begin();
         window != windows.end(); ++window, ++i) {
      bool active = (anchor.active_window() == *window);
      int x = static_cast<int>(roundf(i * title_width));
      int y = border + padding + ascent;
      int win_width = static_cast<int>(roundf((i + 1) * title_width)) - x;
      if (active) {
        DrawBox(win, x, 1, win_width, *height - 2 * border,
                c->anchor_active_focused_bg_color);
      }
      DrawLine(win, x, 0, x, *height - 1, border_color);
      DrawText(
          win, x + border + padding, y, (*window)->title(),
          active ? c->anchor_active_focused_text_color
                 : c->anchor_inactive_text_color, font);
    }
  }
}


DrawingEngine::Style::Style() {
  if (!initialized_) Init();

  for (int i = 0; string_defs_[i].name != NULL; ++i) {
    const StringDef& def = string_defs_[i];
    strings_.insert(make_pair(def.type, def.default_value));
  }
  for (int i = 0; uint_defs_[i].name != NULL; ++i) {
    const UintDef& def = uint_defs_[i];
    uints_.insert(make_pair(def.type, def.default_value));
  }
  for (int i = 0; colors_defs_[i].name != NULL; ++i) {
    const ColorsDef& def = colors_defs_[i];
    colors_.insert(
        make_pair(def.type, Colors(def.top, def.left, def.bottom, def.right)));
  }
}


const string& DrawingEngine::Style::GetString(Type type) const {
  map<Type, string>::const_iterator it = strings_.find(type);
  CHECK(it != strings_.end());
  return it->second;
}


uint DrawingEngine::Style::GetUint(Type type) const {
  map<Type, uint>::const_iterator it = uints_.find(type);
  CHECK(it != uints_.end());
  return it->second;
}


void DrawingEngine::Style::GetColors(
    Type type, DrawingEngine::Style::Colors* colors) const {
  CHECK(colors);
  map<Type, Colors>::const_iterator it = colors_.find(type);
  CHECK(it != colors_.end());
  *colors = it->second;
}


void DrawingEngine::Style::Init() {
  CHECK(!initialized_);
  for (int i = 0; string_defs_[i].name != NULL; ++i) {
  }
  initialized_ = true;
}


void DrawingEngine::InitIfNeeded() {
  if (initialized_) return;

  if (!XServer::Testing()) {
    CHECK(XServer::Get()->Initialized());
    gc_ = XCreateGC(dpy(), root(), 0, NULL);
  }
  initialized_ = true;
}


void DrawingEngine::Clear(::Window win) {
  XClearWindow(dpy(), win);
}


void DrawingEngine::DrawText(::Window win,
                             int x,
                             int y,
                             const string& text,
                             const string& color,
                             const string& font) {
  ChangeColor(color);
  ChangeFont(font);
  XDrawString(dpy(), win, gc_, x, y, text.c_str(), text.size());
}


void DrawingEngine::DrawLine(::Window win,
                             int x1,
                             int y1,
                             int x2,
                             int y2,
                             const string& color) {
  ChangeColor(color);
  XDrawLine(dpy(), win, gc_, x1, y1, x2, y2);
}


void DrawingEngine::DrawBox(::Window win,
                            int x,
                            int y,
                            uint width,
                            uint height,
                            const string& color) {
  ChangeColor(color);
  XFillRectangle(dpy(), win, gc_, x, y, width, height);
}


void DrawingEngine::ChangeColor(const string& name) {
  // FIXME: Is it safe to leave this at 0 if we fail to load the color?
  uint pixel = 0;
  map<string, uint>::const_iterator it = colors_.find(name);
  if (it != colors_.end()) {
    pixel = it->second;
  } else {
    Colormap colormap = DefaultColormap(XServer::Get()->display(),
                                        XServer::Get()->screen_num());
    XColor color;
    // FIXME: Is it okay to pass the same struct in for both the hardware and
    // exact color?
    if (!XAllocNamedColor(XServer::Get()->display(), colormap, name.c_str(),
                          &color, &color)) {

      ERROR << "Unable to allocate color " << name;
    } else {
      colors_.insert(make_pair(name, color.pixel));
      pixel = color.pixel;
    }
  }

  XGCValues gc_val;
  gc_val.foreground = pixel;
  XChangeGC(XServer::Get()->display(), gc_, GCForeground, &gc_val);
}


void DrawingEngine::ChangeFont(const string& name) {
  XFontStruct* font = GetFontInfo(name);
  CHECK(font);
  if (gc_font_ == font) return;
  XSetFont(dpy(), gc_, font->fid);
  gc_font_ = font;
}


void DrawingEngine::GetTextSize(const string& font, const string& text,
                                int* width, int* ascent, int* descent) {
  XFontStruct* font_info = GetFontInfo(font);
  int tmp_dir, tmp_ascent, tmp_descent;
  XCharStruct overall;
  XTextExtents(font_info, text.c_str(), text.size(),
               &tmp_dir, &tmp_ascent, &tmp_descent, &overall);
  if (width) *width = overall.width;
  if (ascent) *ascent = overall.ascent;
  if (descent) *descent = overall.descent;
}


XFontStruct* DrawingEngine::GetFontInfo(const string& name) {
  map<string, XFontStruct*>::const_iterator it = fonts_.find(name);
  if (it != fonts_.end()) return it->second;
  XFontStruct* font_info = XLoadQueryFont(dpy(), name.c_str());
  CHECK(font_info);
  fonts_.insert(make_pair(name, font_info));
  return font_info;
}


Display* DrawingEngine::dpy() {
  return XServer::Get()->display();
}


int DrawingEngine::scr() {
  return XServer::Get()->screen_num();
}


::Window DrawingEngine::root() {
  return XServer::Get()->root();
}

}  // namespace wham
