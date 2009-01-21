// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "drawing-engine.h"

#include <cmath>

#include "anchor.h"
#include "config.h"
#include "window.h"
#include "x-server.h"
#include "x-window.h"

namespace wham {

// Macros for looking up style data.
#define s(x) style_->GetString(x)
#define u(x) style_->GetUint(x)
#define c(x) style_->GetColors(x)

ref_ptr<DrawingEngine> DrawingEngine::singleton_(new DrawingEngine);

const DrawingEngine::Style::StringDef DrawingEngine::Style::string_defs_[] = {
  { INACTIVE_ANCHOR__FONT,
    "inactive_anchor.font", "fixed" },
  { ACTIVE_ANCHOR__FONT,
    "active_anchor.font", "fixed" },
  { INVALID_TYPE, NULL, NULL },
};

const DrawingEngine::Style::UintDef DrawingEngine::Style::uint_defs_[] = {
  { INACTIVE_ANCHOR__BORDER_WIDTH,
    "inactive_anchor.border_width", 1 },
  { INACTIVE_ANCHOR__PADDING,
    "inactive_anchor.padding", 2 },
  { INACTIVE_ANCHOR__WINDOW_SPACING,
    "inactive_anchor.window_spacing", 2 },
  { INACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH,
    "inactive_anchor.active_window.border_width", 1 },
  { INACTIVE_ANCHOR__ACTIVE_WINDOW__PADDING,
    "inactive_anchor.active_window.padding", 2 },
  { INACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_WIDTH,
    "inactive_anchor.inactive_window.border_width", 1 },
  { INACTIVE_ANCHOR__INACTIVE_WINDOW__PADDING,
    "inactive_anchor.inactive_window.padding", 2 },
  { ACTIVE_ANCHOR__BORDER_WIDTH,
    "active_anchor.border_width", 1 },
  { ACTIVE_ANCHOR__PADDING,
    "active_anchor.padding", 2 },
  { ACTIVE_ANCHOR__WINDOW_SPACING,
    "active_anchor.window_spacing", 2 },
  { ACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH,
    "active_anchor.active_window.border_width", 1 },
  { ACTIVE_ANCHOR__ACTIVE_WINDOW__PADDING,
    "active_anchor.active_window.padding", 2 },
  { ACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_WIDTH,
    "active_anchor.inactive_window.border_width", 1 },
  { ACTIVE_ANCHOR__INACTIVE_WINDOW__PADDING,
    "active_anchor.inactive_window.padding", 2 },
  { INVALID_TYPE, NULL, 0 },
};

const DrawingEngine::Style::ColorsDef
    DrawingEngine::Style::colors_defs_[] = {
  { INACTIVE_ANCHOR__COLOR,
    "inactive_anchor.color",
    "#999999", "#ffffff", "#bbbbbb", "#bbbbbb", "#666666", "#666666" },
  { INACTIVE_ANCHOR__ACTIVE_WINDOW__COLOR,
    "inactive_anchor.active_window.color",
    "#666666", "#ffffff", "#333333", "#333333", "#888888", "#888888" },
  { INACTIVE_ANCHOR__INACTIVE_WINDOW__COLOR,
    "inactive_anchor.inactive_window.color",
    "#999999", "#222222", "#bbbbbb", "#bbbbbb", "#666666", "#666666" },
  { ACTIVE_ANCHOR__COLOR,
    "active_anchor.color",
    "#bbbbbb", "#ffffff", "#dddddd", "#dddddd", "#999999", "#999999" },
  { ACTIVE_ANCHOR__ACTIVE_WINDOW__COLOR,
    "active_anchor.active_window.color",
    "#3d4479", "#ffffff", "#272c4e", "#272c4e", "#5f6abd", "#5f6abd" },
  { ACTIVE_ANCHOR__INACTIVE_WINDOW__COLOR,
    "active_anchor.inactive_window.color",
    "#bbbbbb", "#555555", "#dddddd", "#dddddd", "#999999", "#999999" },
  { ACTIVE_WINDOW__FRAME_COLOR,
    "active_window.frame_color",
    "#777777", "#777777", "#333333", "#333333", "#aaaaaa", "#aaaaaa" },
  { INACTIVE_WINDOW__FRAME_COLOR,
    "inactive_window.frame",
    "#777777", "#777777", "#333333", "#333333", "#aaaaaa", "#aaaaaa" },
  { INVALID_TYPE, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
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


void DrawingEngine::DrawAnchor(const Anchor& anchor, XWindow* titlebar) {
  // Don't do anything if there's no real X connection.
  if (XServer::Testing()) {
    return;
  }

  InitIfNeeded();
  ::Window win = titlebar->id();

  uint width = 0, height = 0;

  const Config* conf = Config::Get();

  string anchor_name = anchor.attach() ? "[a] " : "";
  anchor_name += "[" + anchor.name() + "]";

  // TODO: This is ugly.  It might be better to create some type of
  // "variations" system, so that there aren't so many duplicated lines of
  // code for active/inactive anchors and windows.
  uint border = 0;
  uint padding = 0;
  uint spacing = 0;
  uint awindow_border = 0;
  uint awindow_padding = 0;
  uint iwindow_border = 0;
  uint iwindow_padding = 0;
  const string* font = NULL;
  const Style::Colors* anchor_colors = NULL;
  const Style::Colors* awindow_colors = NULL;
  const Style::Colors* iwindow_colors = NULL;

  if (anchor.active()) {
    border = u(Style::ACTIVE_ANCHOR__BORDER_WIDTH);
    padding = u(Style::ACTIVE_ANCHOR__PADDING);
    spacing = u(Style::ACTIVE_ANCHOR__WINDOW_SPACING);
    awindow_border = u(Style::ACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH);
    awindow_padding = u(Style::ACTIVE_ANCHOR__ACTIVE_WINDOW__PADDING);
    iwindow_border = u(Style::ACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_WIDTH);
    iwindow_padding = u(Style::ACTIVE_ANCHOR__INACTIVE_WINDOW__PADDING);
    font = &(s(Style::ACTIVE_ANCHOR__FONT));
    anchor_colors = &(c(Style::ACTIVE_ANCHOR__COLOR));
    awindow_colors = &(c(Style::ACTIVE_ANCHOR__ACTIVE_WINDOW__COLOR));
    iwindow_colors = &(c(Style::ACTIVE_ANCHOR__INACTIVE_WINDOW__COLOR));
  } else {
    border = u(Style::INACTIVE_ANCHOR__BORDER_WIDTH);
    padding = u(Style::INACTIVE_ANCHOR__PADDING);
    spacing = u(Style::INACTIVE_ANCHOR__WINDOW_SPACING);
    awindow_border = u(Style::INACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH);
    awindow_padding = u(Style::INACTIVE_ANCHOR__ACTIVE_WINDOW__PADDING);
    iwindow_border = u(Style::INACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_WIDTH);
    iwindow_padding = u(Style::INACTIVE_ANCHOR__INACTIVE_WINDOW__PADDING);
    font = &(s(Style::INACTIVE_ANCHOR__FONT));
    anchor_colors = &(c(Style::INACTIVE_ANCHOR__COLOR));
    awindow_colors = &(c(Style::INACTIVE_ANCHOR__ACTIVE_WINDOW__COLOR));
    iwindow_colors = &(c(Style::INACTIVE_ANCHOR__INACTIVE_WINDOW__COLOR));
  }

  int ascent = 0, descent = 0;
  GetTextSize(*font, kFullHeightString, NULL, &ascent, &descent);
  height = ascent + descent + 2 * padding + 2 * border +
      2 * max(awindow_border, iwindow_border) +
      2 * max(awindow_padding, iwindow_padding);

  // Include the outer border and padding in the total width.
  width = 2 * (border + padding);
  const vector<Window*>& windows = anchor.windows();
  if (windows.empty()) {
    int name_width = 0;
    GetTextSize(*font, anchor_name, &name_width, NULL, NULL);
    width += name_width + 2 * (iwindow_padding + iwindow_border);
  } else {
    int max_title_width = 0;
    for (vector<Window*>::const_iterator window = windows.begin();
         window != windows.end(); ++window) {
      int title_width = 0;
      GetTextSize(*font, (*window)->title(), &title_width, NULL, NULL);
      max_title_width = max(max_title_width, title_width);
    }
    width += max_title_width + 2 * (awindow_padding + awindow_border) +
        (windows.size() - 1) *
        (max_title_width + iwindow_border + iwindow_padding + spacing);
  }
  width = min(max(width, conf->anchor_min_width), conf->anchor_max_width);

  titlebar->SetBorder(0);
  titlebar->Resize(width, height);
  DrawBorders(win, 0, 0, width, height, *anchor_colors, border);

  // FIXME: set all window's backgrounds on init

  if (windows.empty()) {
    DrawBorders(win, border + padding, border + padding,
                width - 2 * (border + padding),
                height - 2 * (border + padding),
                *iwindow_colors, iwindow_border);
    DrawText(win,
             border + padding + iwindow_border + iwindow_padding,
             border + padding + iwindow_border + iwindow_padding + ascent,
             anchor_name, *font, iwindow_colors->fg);
  } else {
    uint width_for_titles =
        width - 2 * (border + padding) - (windows.size() - 1) * spacing;
    float title_width = static_cast<float>(width_for_titles) / windows.size();

    int i = 0;
    for (vector<Window*>::const_iterator window = windows.begin();
         window != windows.end(); ++window, ++i) {
      bool active = (anchor.active_window() == *window);
      int x = border + padding +
          static_cast<int>(roundf(i * (title_width + spacing)));
      int y = border + padding;
      int this_width = border + padding +
          static_cast<int>(roundf(i * (title_width + spacing) + title_width)) -
          x;
      int this_height = height - 2 * (border + padding);

      DrawBorders(win, x, y, this_width, this_height,
                  active ? *awindow_colors : *iwindow_colors,
                  active ? awindow_border : iwindow_border);

      uint gap = active ?
          awindow_border + awindow_padding :
          iwindow_border + iwindow_padding;
      string tags = (anchor.attach() && active) ? "a" : "";
      tags += (*window)->tagged() ? "t" : "";
      string title = tags.empty() ? "" : "[" + tags + "] ";
      title += (*window)->title();
      DrawText(win, x + gap, y + gap + ascent, title, *font,
               active ? awindow_colors->fg : iwindow_colors->fg);
    }
  }
}


void DrawingEngine::DrawWindowFrame(XWindow* frame) {
  // Don't do anything if there's no real X connection.
  if (XServer::Testing()) {
    return;
  }

  InitIfNeeded();
  ::Window win = frame->id();

  const Style::Colors* colors = NULL;
  // FIXME: get color for inactive windows too
  colors = &(c(Style::ACTIVE_WINDOW__FRAME_COLOR));

  DrawBorders(win, 0, 0, frame->width(), frame->height(), *colors, 1);
  // FIXME: draw inside frame with swapped colors
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
        make_pair(def.type,
                  Colors(def.bg, def.fg, def.top,
                         def.left, def.bottom, def.right)));
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


const DrawingEngine::Style::Colors& DrawingEngine::Style::GetColors(
    Type type) const {
  map<Type, Colors>::const_iterator it = colors_.find(type);
  CHECK(it != colors_.end());
  return it->second;
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
                             const string& font,
                             const string& color) {
  ChangeFont(font);
  ChangeColor(color);
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


void DrawingEngine::DrawBorders(::Window win,
                                int x,
                                int y,
                                uint width,
                                uint height,
                                const Style::Colors& colors,
                                uint border_width) {
  DrawBox(win, x, y, width, height, colors.bg);
  if (border_width) {
    DrawBox(win, x, y, width, border_width, colors.top);
    DrawBox(win, x, y + border_width, border_width, height - border_width,
            colors.left);
    DrawBox(win, x + border_width, y + height - border_width,
            width - border_width, border_width, colors.bottom);
    DrawBox(win, x + width - border_width, y + border_width,
            border_width, height - 2 * border_width, colors.right);
  }
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
