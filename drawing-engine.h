// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __DRAWING_ENGINE_H__
#define __DRAWING_ENGINE_H__

#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"

using namespace std;

namespace wham {

class Anchor;   // from anchor.h
class Window;   // from window.h
class XWindow;  // from x-window.h
class XServer;  // from x-server.h

class DrawingEngine {
 public:
  DrawingEngine();

  // Get the current drawing engine.
  static DrawingEngine* Get() {
    CHECK(singleton_.get());
    return singleton_.get();
  }

  // Install a new drawing engine.
  static void Swap(ref_ptr<DrawingEngine> new_drawing_engine) {
    singleton_.swap(new_drawing_engine);
  }

  // Draw an anchor's titlebar.
  void DrawAnchor(const Anchor& anchor, XWindow* titlebar);

  // TODO: For operations that could potentially redraw the same objects
  // multiple times, it'd be nice if the caller could call StartBuffering()
  // first and Finalize() at the end.  When buffering is enabled, methods
  // like DrawAnchor() would just record the fact that the anchor needs to
  // be drawn in a set, and then we'd finally actually draw everything when
  // Finalize() is called.
  void StartBuffering();
  void Finalize();

 private:
  class Style {
   public:
    Style();

    // Enum describing different components of the style.
    enum Type {
      INVALID_TYPE,

      INACTIVE_ANCHOR__BACKGROUND,
      INACTIVE_ANCHOR__FONT,
      INACTIVE_ANCHOR__ACTIVE_WINDOW__BACKGROUND,
      INACTIVE_ANCHOR__ACTIVE_WINDOW__TEXT_COLOR,
      INACTIVE_ANCHOR__INACTIVE_WINDOW__BACKGROUND,
      INACTIVE_ANCHOR__INACTIVE_WINDOW__TEXT_COLOR,

      INACTIVE_ANCHOR__BORDER_WIDTH,
      INACTIVE_ANCHOR__PADDING,
      INACTIVE_ANCHOR__WINDOW_SPACING,
      INACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH,
      INACTIVE_ANCHOR__ACTIVE_WINDOW__PADDING,
      INACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_WIDTH,
      INACTIVE_ANCHOR__INACTIVE_WINDOW__PADDING,

      INACTIVE_ANCHOR__BORDER_COLOR,
      INACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_COLOR,
      INACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_COLOR,

      ACTIVE_ANCHOR__BACKGROUND,
      ACTIVE_ANCHOR__FONT,
      ACTIVE_ANCHOR__ACTIVE_WINDOW__BACKGROUND,
      ACTIVE_ANCHOR__ACTIVE_WINDOW__TEXT_COLOR,
      ACTIVE_ANCHOR__INACTIVE_WINDOW__BACKGROUND,
      ACTIVE_ANCHOR__INACTIVE_WINDOW__TEXT_COLOR,

      ACTIVE_ANCHOR__BORDER_WIDTH,
      ACTIVE_ANCHOR__PADDING,
      ACTIVE_ANCHOR__WINDOW_SPACING,
      ACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_WIDTH,
      ACTIVE_ANCHOR__ACTIVE_WINDOW__PADDING,
      ACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_WIDTH,
      ACTIVE_ANCHOR__INACTIVE_WINDOW__PADDING,

      ACTIVE_ANCHOR__BORDER_COLOR,
      ACTIVE_ANCHOR__ACTIVE_WINDOW__BORDER_COLOR,
      ACTIVE_ANCHOR__INACTIVE_WINDOW__BORDER_COLOR,
    };

    struct Colors {
      Colors(const string& top,
             const string& left,
             const string& bottom,
             const string& right)
          : top(top),
            left(left),
            bottom(bottom),
            right(right) {}
      string top;
      string left;
      string bottom;
      string right;
    };

    const string& GetString(Type type) const;
    uint GetUint(Type type) const;
    const Colors& GetColors(Type type) const;

   private:
    // Initialize static data.
    static void Init();

    // Struct describing a string component of the style.
    struct StringDef {
      Type type;
      const char* name;
      const char* default_value;
    };

    // Struct describing an unsigned int component of the style.
    struct UintDef {
      Type type;
      const char* name;
      uint default_value;
    };

    // Struct describing a set of four colors belonging to the style.
    struct ColorsDef {
      Type type;
      const char* name;
      const char* top;
      const char* left;
      const char* bottom;
      const char* right;
    };

    // Static information describing components of the style.
    const static StringDef string_defs_[];
    const static UintDef uint_defs_[];
    const static ColorsDef colors_defs_[];

    static bool initialized_;

    map<Type, string> strings_;
    map<Type, uint> uints_;
    map<Type, Colors> colors_;
  };  // class Style

  // Initialize the drawing engine.  Called automatically.  Must be called
  // after the XServer singleton has been initialized.
  void InitIfNeeded();

  void Clear(::Window win);

  // Draw text into a window.
  void DrawText(::Window win,
                int x,
                int y,
                const string& text,
                const string& font,
                const string& color);

  // Draw a line in a window.
  void DrawLine(::Window win,
                int x1,
                int y1,
                int x2,
                int y2,
                const string& color);

  // Draw a filled rectangle in a window.
  void DrawBox(::Window win,
               int x,
               int y,
               uint width,
               uint height,
               const string& color);

  void DrawBorders(::Window win,
                   int x,
                   int y,
                   uint width,
                   uint height,
                   const string& background,
                   const Style::Colors& border_colors,
                   uint border_width);

  // Change the color used by 'gc_'.
  void ChangeColor(const string& name);

  // Change the font used by 'gc_'.
  void ChangeFont(const string& name);

  // Get the size of the text 'text' when written in the font described by
  // 'font'.  The text's width, ascent, and descent are stored at the
  // passed-in pointers if non-NULL.
  void GetTextSize(const string& font, const string& text,
                   int* width, int* ascent, int* descent);

  // Get the font described by the passed-in string, loading it if
  // necessary.
  XFontStruct* GetFontInfo(const string& name);

  static Display* dpy();
  static int scr();
  static ::Window root();

  // Has the object been initialized by calling Init()?
  bool initialized_;

  // Map from font name to font struct.
  map<string, XFontStruct*> fonts_;

  // Map from color name to its pixel value.
  map<string, uint> colors_;

  // GC used for drawing.
  GC gc_;

  // Font currently installed in 'gc_'.
  XFontStruct* gc_font_;

  ref_ptr<Style> style_;

  // Singleton object.
  static ref_ptr<DrawingEngine> singleton_;

  DISALLOW_EVIL_CONSTRUCTORS(DrawingEngine);
};

}  // namespace wham

#endif
