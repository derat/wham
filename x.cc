// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "x.h"

#include <iostream>
#include "X11/Xatom.h"

#include "key-bindings.h"
#include "mock-x.h"
#include "util.h"
#include "window-classifier.h"
#include "window-manager.h"
#include "window-properties.h"

using namespace std;

namespace wham {

ref_ptr<XServer> XServer::singleton_(new XServer);

bool XServer::testing_ = false;


static const char* XEventTypeToName(int type) {
  switch (type) {
    case ButtonPress: return "ButtonPress";
    case ButtonRelease: return "ButtonRelease";
    case CirculateNotify: return "CirculateNotify";
    case CirculateRequest: return "CirculateRequest";
    case ClientMessage: return "ClientMessage";
    case ColormapNotify: return "ColormapNotify";
    case ConfigureNotify: return "ConfigureNotify";
    case ConfigureRequest: return "ConfigureRequest";
    case CreateNotify: return "CreateNotify";
    case DestroyNotify: return "DestroyNotify";
    case EnterNotify: return "EnterNotify";
    case Expose: return "Expose";
    case FocusIn: return "FocusIn";
    case FocusOut: return "FocusOut";
    case GraphicsExpose: return "GraphicsExpose";
    case GravityNotify: return "GravityNotify";
    case KeymapNotify: return "KeymapNotify";
    case KeyPress: return "KeyPress";
    case KeyRelease: return "KeyRelease";
    case LeaveNotify: return "LeaveNotify";
    case MapNotify: return "MapNotify";
    case MappingNotify: return "MappingNotify";
    case MapRequest: return "MapRequest";
    case MotionNotify: return "MotionNotify";
    case NoExpose: return "NoExpose";
    case PropertyNotify: return "PropertyNotify";
    case ReparentNotify: return "ReparentNotify";
    case ResizeRequest: return "ResizeRequest";
    case SelectionClear: return "SelectionClear";
    case SelectionNotify: return "SelectionNotify";
    case SelectionRequest: return "SelectionRequest";
    case UnmapNotify: return "UnmapNotify";
    case VisibilityNotify: return "VisibilityNotify";
    default: return "Unknown event";
  }
}


XWindow::XWindow(::Window id)
    : id_(id) {
}


XWindow* XWindow::Create(int x, int y, uint width, uint height) {
  ::Window id;
  if (XServer::Testing()) {
    static int win_id = 1;
    id = win_id++;
  } else {
    id = XCreateSimpleWindow(
             dpy(), root(), x, y, width, height, 0 /* border */,
             BlackPixel(dpy(), scr()),
             WhitePixel(dpy(), scr()));
    XSelectInput(dpy(), id,
                 ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
                 ExposureMask | PointerMotionMask | PropertyChangeMask);
  }
  XWindow* win = XServer::Get()->GetWindow(id, true);
  win->x_ = x;
  win->y_ = y;
  win->width_ = width;
  win->height_ = height;
  return win;
}


bool XWindow::UpdateProperties(WindowProperties* props,
                               WindowProperties::ChangeType type) {
  CHECK(props);

  if (type == WindowProperties::WINDOW_NAME_CHANGE) {
    char* window_name = NULL;
    if (!XFetchName(dpy(), id_, &window_name)) {
      ERROR << "XFetchName() failed for 0x" << hex << id_;
      return false;
    }
    props->window_name = window_name ? window_name : "";
    if (window_name) XFree(window_name);
  } else if (type == WindowProperties::ICON_NAME_CHANGE) {
    char* icon_name = NULL;
    if (!XGetIconName(dpy(), id_, &icon_name)) {
      ERROR << "XGetIconName() failed for 0x" << hex << id_;
      return false;
    }
    props->icon_name = icon_name ? icon_name : "";
    if (icon_name) XFree(icon_name);
  } else if (type == WindowProperties::COMMAND_CHANGE) {
    // FIXME: get command with XGetCommand()
  } else if (type == WindowProperties::CLASS_CHANGE) {
    XClassHint class_hint;
    if (!XGetClassHint(dpy(), id_, &class_hint)) {
      ERROR << "XGetClassHint() failed for 0x" << hex << id_;
      return false;
    }
    props->app_name = class_hint.res_name ? class_hint.res_name : "";
    props->app_class = class_hint.res_class ? class_hint.res_class : "";
    if (class_hint.res_name) XFree(class_hint.res_name);
    if (class_hint.res_class) XFree(class_hint.res_class);
  } else if (type == WindowProperties::WM_HINTS_CHANGE) {
    XSizeHints* size_hints = XAllocSizeHints();
    CHECK(size_hints);
    long supplied_hints = 0;
    if (!XGetWMNormalHints(dpy(), id_, size_hints, &supplied_hints)) {
      ERROR << "XGetWMNormalHints() failed for 0x" << hex << id_;
      XFree(size_hints);
      return false;
    }
    if (size_hints->flags & USPosition || size_hints->flags & PPosition) {
      props->x = size_hints->x;
      props->y = size_hints->y;
    }
    if (size_hints->flags & USSize || size_hints->flags & PSize) {
      props->width = size_hints->width;
      props->height = size_hints->height;
    }
    if (size_hints->flags & PMinSize) {
      props->min_width = size_hints->min_width;
      props->min_height = size_hints->min_height;
    }
    if (size_hints->flags & PMaxSize) {
      props->max_width = size_hints->max_width;
      props->max_height = size_hints->max_height;
    }
    if (size_hints->flags & PResizeInc) {
      props->width_inc = size_hints->width_inc;
      props->height_inc = size_hints->height_inc;
    }
    if (size_hints->flags & PAspect) {
      props->min_aspect = static_cast<float>(size_hints->min_aspect.x) /
                          size_hints->min_aspect.y;
      props->max_aspect = static_cast<float>(size_hints->max_aspect.x) /
                          size_hints->max_aspect.y;
    }
    if (size_hints->flags & PBaseSize) {
      props->base_width = size_hints->base_width;
      props->base_height = size_hints->base_height;
    }
    XFree(size_hints);
  } else if (type == WindowProperties::TRANSIENT_CHANGE) {
    // FIXME: handle this
  } else {
    ERROR << "Unable to handle property change of type " << type;
    CHECK(false);
  }

  return true;
}


void XWindow::Move(int x, int y) {
  x_ = x;
  y_ = y;
  XMoveWindow(dpy(), id_, x, y);
}


void XWindow::Resize(uint width, uint height) {
  width_ = width;
  height_ = height;
  XResizeWindow(dpy(), id_, width, height);
}


void XWindow::Unmap() {
  XUnmapWindow(dpy(), id_);
}


void XWindow::Map() {
  XMapWindow(dpy(), id_);
}


void XWindow::SelectEvents() {
  XSelectInput(dpy(), id_, EnterWindowMask | PropertyChangeMask);
}


void XWindow::TakeFocus() {
  XSetInputFocus(dpy(), id_, RevertToPointerRoot, CurrentTime);
}


void XWindow::SetBorder(uint size) {
  XSetWindowBorderWidth(dpy(), id_, size);
}


void XWindow::Raise() {
  XRaiseWindow(dpy(), id_);
}


void XWindow::MakeSibling(const XWindow& leader) {
  DEBUG << "MakeSibling: id=0x" << hex << id_ << " leader=0x" << leader.id();
  XWindowChanges changes;
  changes.sibling = leader.id();
  changes.stack_mode = Below;
  XConfigureWindow(dpy(), id_, CWSibling | CWStackMode, &changes);
}


void XWindow::GetGeometry(int* x,
                          int* y,
                          uint* width,
                          uint* height,
                          uint* border_width) {
  ::Window root;
  int tmp_x, tmp_y;
  uint tmp_width, tmp_height, tmp_border_width, tmp_depth;
  XGetGeometry(dpy(), id_, &root, &tmp_x, &tmp_y, &tmp_width, &tmp_height,
               &tmp_border_width, &tmp_depth);
  if (x) *x = tmp_x;
  if (y) *y = tmp_y;
  if (width) *width = tmp_width;
  if (height) *height = tmp_height;
  if (border_width) *border_width = tmp_border_width;
}


::Display* XWindow::dpy() {
  return XServer::Get()->display();
}


int XWindow::scr() {
  return XServer::Get()->screen_num();
}


::Window XWindow::root() {
  return XServer::Get()->root();
}


XServer::XServer()
    : display_(NULL),
      screen_num_(-1),
      width_(0),
      height_(0),
      initialized_(false),
      in_progress_binding_(NULL) {
}


void XServer::SetupTesting() {
  XServer::SetTesting(true);
  // Initialize the X server once.
  if (!Get()->initialized_) Get()->Init();
}


bool XServer::Init() {
  CHECK(!initialized_);

  if (!testing_) {
    display_ = XOpenDisplay(NULL);
    if (display_ == NULL) {
      ERROR << "Can't open display " << XDisplayName(NULL);
      return false;
    }
    screen_num_ = DefaultScreen(display_);
    root_ = RootWindow(display_, screen_num_);

    ::Window root_ret;
    int x, y;
    uint border_width, depth;
    XGetGeometry(display_, root_, &root_ret, &x, &y,
                 &width_, &height_, &border_width, &depth);

    XSelectInput(display_, root_, SubstructureNotifyMask);

  }

  initialized_ = true;
  return true;
}


void XServer::RunEventLoop(WindowManager* window_manager) {
  CHECK(window_manager);
  CHECK(initialized_);

  XEvent event;
  while (true) {
    XNextEvent(display_, &event);

    if (event.type == ButtonPress) {
      XButtonEvent& e = event.xbutton;
      DEBUG << "ButtonPress: window=0x" << hex << e.window;
      XWindow* x_window = GetWindow(e.window, false);
      window_manager->HandleButtonPress(x_window, e.x_root, e.y_root);
    } else if (event.type == ButtonRelease) {
      XButtonEvent& e = event.xbutton;
      DEBUG << "ButtonRelease: window=0x" << hex << e.window;
      XWindow* x_window = GetWindow(e.window, false);
      window_manager->HandleButtonRelease(x_window, e.x_root, e.y_root);
    } else if (event.type == ConfigureNotify) {
      /*
      XConfigureEvent& e = event.xconfigure;
      DEBUG << "ConfigureNotify: window=0x" << hex << e.window << dec
            << " x=" << e.x << " y=" << e.y
            << " width=" << e.width << " height=" << e.height
            << " border=" << e.border_width
            << " above=" << static_cast<int>(e.above)
            << " override=" << e.override_redirect;
          */
    } else if (event.type == CreateNotify) {
      XCreateWindowEvent& e = event.xcreatewindow;
      DEBUG << "CreateNotify: window=0x" << hex << e.window
            << " parent=0x" << e.parent << dec
            << " x=" << e.x << " y=" << e.y
            << " width=" << e.width << " height=" << e.height
            << " border=" << e.border_width
            << " override=" << e.override_redirect;
      //XWindow* x_window = GetWindow(e.window, true);
    } else if (event.type == DestroyNotify) {
      XDestroyWindowEvent& e = event.xdestroywindow;
      DEBUG << "DestroyNotify: window=0x" << hex << e.window;
      XWindow* x_window = GetWindow(e.window, false);
      if (x_window) {
        window_manager->HandleDestroyWindow(x_window);
        DeleteWindow(e.window);
      }
    } else if (event.type == EnterNotify) {
      XCrossingEvent& e = event.xcrossing;
      DEBUG << "Enter: window=0x" << hex << e.window;
      XWindow* x_window = GetWindow(e.window, false);
      CHECK(x_window);
      window_manager->HandleEnterWindow(x_window);
    } else if (event.type == Expose) {
      // Coalesce expose events for the same window to avoid redrawing the
      // same one more than necessary.
      // TODO: Is this needed?  Doesn't appear to have much of an effect on
      // my computer, but maybe with a slower machine or slower drawing
      // code it could help.
      set<XWindow*> exposed_windows;
      do {
        XExposeEvent& e = event.xexpose;
        XWindow* x_window = GetWindow(e.window, false);
        CHECK(x_window);
        exposed_windows.insert(x_window);
      } while (XCheckMaskEvent(display_, ExposureMask, &event) == True);
      for (set<XWindow*>::iterator win = exposed_windows.begin();
           win != exposed_windows.end(); ++win) {
        //DEBUG << "Expose: window=0x" << hex << (*win)->id();
        window_manager->HandleExposeWindow(*win);
      }
    } else if (event.type == KeyPress) {
      XKeyEvent& e = event.xkey;
      DEBUG << "KeyPress: window=0x" << hex << e.window << dec
            << " keycode=" << e.keycode << " state=" << e.state;
      HandleKeyPress(XLookupKeysym(&e, 0), e.state, window_manager);
    } else if (event.type == KeyRelease) {
      XKeyEvent& e = event.xkey;
      DEBUG << "KeyRelease: window=0x" << hex << e.window << dec
            << " keycode=" << e.keycode << " state=" << e.state;
    } else if (event.type == MapNotify) {
      XMapEvent& e = event.xmap;
      DEBUG << "MapNotify: window=0x" << hex << e.window << dec
            << " override=" << e.override_redirect;
      if (!e.override_redirect) {
        XWindow* x_window = GetWindow(e.window, true);
        window_manager->HandleMapWindow(x_window);
      }
    } else if (event.type == MotionNotify) {
      XMotionEvent& e = event.xmotion;
      //DEBUG << "MotionNotify: window=0x" << hex << e.window << dec
      //      << " x=" << e.x_root << " y=" << e.y_root;
      XWindow* x_window = GetWindow(e.window, false);
      window_manager->HandleMotion(x_window, e.x_root, e.y_root);
    } else if (event.type == PropertyNotify) {
      XPropertyEvent& e = event.xproperty;
      XWindow* x_window = GetWindow(e.window, false);
      WindowProperties::ChangeType type = WindowProperties::OTHER_CHANGE;
      switch (e.atom) {
        case XA_WM_NAME: type = WindowProperties::WINDOW_NAME_CHANGE; break;
        case XA_WM_ICON_NAME: type = WindowProperties::ICON_NAME_CHANGE; break;
        case XA_WM_COMMAND: type = WindowProperties::COMMAND_CHANGE; break;
        case XA_WM_CLASS: type = WindowProperties::CLASS_CHANGE; break;
        case XA_WM_NORMAL_HINTS:
             type = WindowProperties::WM_HINTS_CHANGE; break;
        case XA_WM_TRANSIENT_FOR:
             type = WindowProperties::TRANSIENT_CHANGE; break;
        default: type = WindowProperties::OTHER_CHANGE;
      }
      DEBUG << "PropertyNotify: window=0x" << hex << e.window << dec
            << " atom=" << e.atom
            << " type=" << WindowProperties::ChangeTypeToStr(type)
            << " state=" << (e.state == PropertyNewValue ?
                             "PropertyNewValue" : "PropertyDeleted");
      if (type != WindowProperties::OTHER_CHANGE) {
        window_manager->HandlePropertyChange(x_window, type);
      }
    } else {
      DEBUG << XEventTypeToName(event.type);
    }
  }
}


XWindow* XServer::GetWindow(::Window id, bool create) {
  XWindowMap::iterator it = windows_.find(id);
  if (it != windows_.end()) return it->second.get();
  if (!create) return NULL;
  ref_ptr<XWindow> window(testing_ ? new MockXWindow(id) : new XWindow(id));
  windows_.insert(make_pair(id, window));
  return window.get();
}


void XServer::DeleteWindow(::Window id) {
  windows_.erase(id);
}


void XServer::RegisterKeyBindings(const KeyBindings& bindings) {
  // Ungrab old bindings, update our map, and grab all of the top-level
  // bindings.
  XUngrabKey(display_, AnyKey, AnyModifier, root_);
  UpdateKeyBindingMap(bindings, &bindings_);
  for (XKeyBindingMap::const_iterator it = bindings_.begin();
       it != bindings_.end(); ++it) {
    KeyCode keycode = XKeysymToKeycode(display_, it->second->keysym);
    XGrabKey(display_, keycode, it->second->required_mods,
             root_, False, GrabModeAsync, GrabModeAsync);
  }
}


bool XServer::GetModifiers(const vector<string>& mods, uint* mod_bits) {
  CHECK(mod_bits);
  *mod_bits = 0U;
  bool result = true;
  for (vector<string>::const_iterator mod = mods.begin();
       mod != mods.end(); ++mod) {
    if (strcasecmp(mod->c_str(), "Shift") == 0) {
      *mod_bits |= ShiftMask;
    } else if (strcasecmp(mod->c_str(), "Control") == 0 ||
               strcasecmp(mod->c_str(), "Ctrl") == 0) {
      *mod_bits |= ControlMask;
    } else if (strcasecmp(mod->c_str(), "Mod1") == 0) {
      *mod_bits |= Mod1Mask;
    } else {
      result = false;
    }
  }
  return result;
}


void XServer::UpdateKeyBindingMap(
    const KeyBindings& bindings, XKeyBindingMap* binding_map) {
  binding_map->clear();

  for (vector<KeyBindings::Binding>::const_iterator binding =
         bindings.bindings().begin();
       binding != bindings.bindings().end(); ++binding) {
    if (binding->combos.empty()) {
      ERROR << "Skipping empty key binding for command "
            << binding->command.ToString();
      continue;
    }

    XKeyBinding* parent_binding = NULL;
    uint inherited_mods = 0;
    bool error_in_combo = false;
    for (vector<KeyBindings::Combo>::const_iterator combo =
           binding->combos.begin();
         combo != binding->combos.end(); ++combo) {
      // Convert the string versions of the modifiers into a bitmask.
      uint mods = 0;
      if (!GetModifiers(combo->mods, &mods)) {
        ERROR << "Unknown modifier in key binding "
              << binding->ToString() << " for command "
              << binding->command.ToString();
        error_in_combo = true;
        break;
      }

      // Convert the string representation of the key into a keysym.
      KeySym keysym = XStringToKeysym(combo->key.c_str());
      if (keysym == NoSymbol) {
        ERROR << "Unknown symbol \"" << combo->key << "\" in key binding "
              << binding->ToString() << " for command "
              << binding->command.ToString();
        error_in_combo = true;
        break;
      }

      // Use the lowercase version of the keysym if one exists.
      KeySym lower_keysym = NoSymbol;
      KeySym upper_keysym = NoSymbol;
      XConvertCase(keysym, &lower_keysym, &upper_keysym);
      keysym = lower_keysym;

      if (parent_binding == NULL) {
        // If this is the first combo in the sequence, then it needs to go
        // into the top-level map.  If we already have it registered as
        // part of another binding, we'll just use that.
        pair<KeySym, uint> key = make_pair(keysym, mods);
        if (binding_map->find(key) == binding_map->end()) {
          ref_ptr<XKeyBinding> x_binding(
              new XKeyBinding(keysym, mods, 0, Command()));
          binding_map->insert(make_pair(key, x_binding));
        }
        parent_binding = binding_map->find(key)->second.get();
        CHECK(parent_binding);
      } else {
        if (parent_binding->command.type() != Command::UNKNOWN) {
          ERROR << "While adding multi-level binding "
                << binding->ToString() << " for "
                << binding->command.ToString()
                << ", removing command "
                << parent_binding->command.ToString()
                << " associated with shorter binding";
          parent_binding->command = Command();
        }

        // Otherwise, we iterate through all of the combos listed as
        // children of the last combo to see if this one's already
        // registered.
        bool found = false;
        for (vector<ref_ptr<XKeyBinding> >::iterator x_binding =
               parent_binding->children.begin();
             x_binding != parent_binding->children.end(); ++x_binding) {
          CHECK((*x_binding)->inherited_mods == inherited_mods);
          if ((*x_binding)->required_mods == mods &&
              (*x_binding)->keysym == keysym) {
            DEBUG << "Using existing child binding:"
                  << " keysym=" << keysym << " mods=" << mods
                  << " inherited_mods=" << inherited_mods;
            parent_binding = (*x_binding).get();
            found = true;
            break;
          }
        }
        if (!found) {
          ref_ptr<XKeyBinding> x_binding(
              new XKeyBinding(keysym, mods, inherited_mods, Command()));
          parent_binding->children.push_back(x_binding);
          parent_binding = x_binding.get();
        }
      }

      inherited_mods |= mods;
    }

    if (!error_in_combo) {
      CHECK(parent_binding);
      if (!parent_binding->children.empty()) {
        ERROR << "Key binding " << binding->ToString() << " already "
              << "has sub-bindings; not adding command "
              << binding->command.ToString();
      } else {
        if (parent_binding->command.type() != Command::UNKNOWN) {
          ERROR << "Rebinding " << binding->ToString() << " from "
                << parent_binding->command.ToString() << " to "
                << binding->command.ToString();
        }
        parent_binding->command = binding->command;
      }
    }
  }
}


void XServer::HandleKeyPress(KeySym keysym, uint mods,
                             WindowManager* window_manager) {
  KeySym keysym_lower = NoSymbol;
  KeySym keysym_upper = NoSymbol;
  XConvertCase(keysym, &keysym_lower, &keysym_upper);

  XKeyBinding* binding =
      FindWithDefault(bindings_, XKeyCombo(keysym_lower, mods),
                      ref_ptr<XKeyBinding>(NULL)).get();
  if (binding == NULL) {
    ERROR << "Ignoring key press without binding";
    return;
  }

  // FIXME: handle multi-level bindings

  if (binding->command.type() != Command::UNKNOWN) {
    window_manager->HandleCommand(binding->command);
  }
}

}  // namespace wham
