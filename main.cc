#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "util.h"
#include "window-manager.h"

using namespace std;
using namespace wham;

const char* EventTypeToName(int type) {
  switch (type) {
    case KeyPress: return "KeyPress";
    case KeyRelease: return "KeyRelease";
    case ButtonPress: return "ButtonPress";
    case ButtonRelease: return "ButtonRelease";
    case MotionNotify: return "MotionNotify";
    case EnterNotify: return "EnterNotify";
    case LeaveNotify: return "LeaveNotify";
    case FocusIn: return "FocusIn";
    case FocusOut: return "FocusOut";
    case KeymapNotify: return "KeymapNotify";
    case Expose: return "Expose";
    case GraphicsExpose: return "GraphicsExpose";
    case NoExpose: return "NoExpose";
    case VisibilityNotify: return "VisibilityNotify";
    case CreateNotify: return "CreateNotify";
    case DestroyNotify: return "DestroyNotify";
    case UnmapNotify: return "UnmapNotify";
    case MapNotify: return "MapNotify";
    case MapRequest: return "MapRequest";
    case ReparentNotify: return "ReparentNotify";
    case ConfigureNotify: return "ConfigureNotify";
    case ConfigureRequest: return "ConfigureRequest";
    case GravityNotify: return "GravityNotify";
    case ResizeRequest: return "ResizeRequest";
    case CirculateNotify: return "CirculateNotify";
    case CirculateRequest: return "CirculateRequest";
    case PropertyNotify: return "PropertyNotify";
    case SelectionClear: return "SelectionClear";
    case SelectionRequest: return "SelectionRequest";
    case SelectionNotify: return "SelectionNotify";
    case ColormapNotify: return "ColormapNotify";
    case ClientMessage: return "ClientMessage";
    case MappingNotify: return "MappingNotify";
    default: return "Unknown event";
  }
}

int main(int argc, char** argv) {
  Display* display = XOpenDisplay(NULL);
  if (display == NULL) {
    LOG << "Can't open display " << XDisplayName(NULL);
    return 1;
  }
  int screen_num = DefaultScreen(display);

  WindowManager window_manager(display);

  XSelectInput(display,
               RootWindow(display, screen_num),
               SubstructureNotifyMask);

  XEvent event;
  while (true) {
    XNextEvent(display, &event);
    LOG << EventTypeToName(event.type);
    switch (event.type) {
      case CreateNotify:
        {
          XCreateWindowEvent& e = event.xcreatewindow;
          LOG << "  window=" << static_cast<int>(e.window)
              << " parent=" << static_cast<int>(e.parent)
              << " x=" << e.x << " y=" << e.y
              << " width=" << e.width << " height=" << e.height
              << " border=" << e.border_width
              << " override=" << e.override_redirect;
          window_manager.AddWindow(e.window);
        }
        break;
      case DestroyNotify:
        {
          XDestroyWindowEvent& e = event.xdestroywindow;
          LOG << "  window=" << static_cast<int>(e.window);
          window_manager.RemoveWindow(e.window);
        }
        break;
      case ConfigureNotify:
        {
          XConfigureEvent& e = event.xconfigure;
          LOG << "  window=" << static_cast<int>(e.window)
              << " x=" << e.x << " y=" << e.y
              << " width=" << e.width << " height=" << e.height
              << " border=" << e.border_width
              << " above=" << static_cast<int>(e.above)
              << " override=" << e.override_redirect;
        }
        break;
    }
  }

  return 0;
}
