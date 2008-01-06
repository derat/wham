// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "x.h"

#include <iostream>

#include "key-bindings.h"
#include "mock-x.h"
#include "util.h"
#include "window-classifier.h"
#include "window-manager.h"

using namespace std;

namespace wham {

ref_ptr<XServer> XServer::singleton_(NULL);

bool XWindow::testing_ = false;


static const char* XEventTypeToName(int type) {
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


XWindow::XWindow(::Window id)
    : id_(id) {
  if (!testing_) {
    // TODO: move this to a separate, virtual method
    XSetWindowBorderWidth(XServer::Get()->display(), id_, 0);
  }
}


XWindow* XWindow::Create(int x, int y, uint width, uint height) {
  ::Window win;
  if (testing_) {
    static int win_id = 1;
    win = win_id++;
  } else {
    win = XCreateSimpleWindow(
              XServer::Get()->display(), XServer::Get()->root(),
              x, y, width, height, 0 /* border */,
              BlackPixel(XServer::Get()->display(),
                         XServer::Get()->screen_num()),
              WhitePixel(XServer::Get()->display(),
                         XServer::Get()->screen_num()));
    XSelectInput(XServer::Get()->display(), win,
                 ButtonPressMask | ButtonReleaseMask | ExposureMask |
                 PointerMotionMask | PropertyChangeMask);
  }
  return XServer::Get()->GetWindow(win, true);
}


bool XWindow::GetProperties(WindowProperties* props) {
  char* window_name = NULL;
  if (!XFetchName(XServer::Get()->display(), id_, &window_name)) {
    ERROR << "XFetchName() failed for 0x" << hex << id_;
    return false;
  }
  props->window_name = window_name ? window_name : "";
  if (window_name) XFree(window_name);

  char* icon_name = NULL;
  if (!XGetIconName(XServer::Get()->display(), id_, &icon_name)) {
    ERROR << "XGetIconName() failed for 0x" << hex << id_;
    return false;
  }
  props->icon_name = icon_name ? icon_name : "";
  if (icon_name) XFree(icon_name);

  // FIXME: get command with XGetCommand()

  XClassHint class_hint;
  if (!XGetClassHint(XServer::Get()->display(), id_, &class_hint)) {
    ERROR << "XGetClassHint() failed for 0x" << hex << id_;
    return false;
  }
  props->app_name = class_hint.res_name ? class_hint.res_name : "";
  props->app_class = class_hint.res_class ? class_hint.res_class : "";
  if (class_hint.res_name) XFree(class_hint.res_name);
  if (class_hint.res_class) XFree(class_hint.res_class);

  return true;
}


void XWindow::Move(int x, int y) {
  XMoveWindow(XServer::Get()->display(), id_, x, y);
}


void XWindow::Resize(uint width, uint height) {
  XResizeWindow(XServer::Get()->display(), id_, width, height);
}


void XWindow::Unmap() {
  XUnmapWindow(XServer::Get()->display(), id_);
}


void XWindow::Map() {
  XMapWindow(XServer::Get()->display(), id_);
}


void XWindow::SelectEvents() {
  XSelectInput(XServer::Get()->display(), id_, EnterWindowMask);
}


void XWindow::TakeFocus() {
  XSetInputFocus(XServer::Get()->display(), id_, RevertToPointerRoot, CurrentTime);
}


XServer::XServer()
    : display_(NULL),
      screen_num_(-1),
      initialized_(false),
      in_progress_binding_(NULL) {
}


bool XServer::Init() {
  CHECK(!initialized_);

  if (!XWindow::testing_) {
    display_ = XOpenDisplay(NULL);
    if (display_ == NULL) {
      ERROR << "Can't open display " << XDisplayName(NULL);
      return false;
    }
    screen_num_ = DefaultScreen(display_);

    root_ = RootWindow(display_, screen_num_);

    GC gc = XCreateGC(display_, root_, 0, NULL);
    XSetForeground(display_, gc, BlackPixel(display_, screen_num_));
    gcs_.insert(make_pair("black", gc));

    gc = XCreateGC(display_, root_, 0, NULL);
    XSetForeground(display_, gc, WhitePixel(display_, screen_num_));
    gcs_.insert(make_pair("white", gc));

    // debugging
    //XSynchronize(display_, True);

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
    switch (event.type) {
      case ButtonPress:
        {
          XButtonEvent& e = event.xbutton;
          DEBUG << "ButtonPress: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          window_manager->HandleButtonPress(x_window, e.x_root, e.y_root);
        }
        break;
      case ButtonRelease:
        {
          XButtonEvent& e = event.xbutton;
          DEBUG << "ButtonRelease: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          window_manager->HandleButtonRelease(x_window, e.x_root, e.y_root);
        }
        break;
      case ConfigureNotify:
        {
          /*
          XConfigureEvent& e = event.xconfigure;
          DEBUG << "ConfigureNotify: window=0x" << hex << e.window << dec
                << " x=" << e.x << " y=" << e.y
                << " width=" << e.width << " height=" << e.height
                << " border=" << e.border_width
                << " above=" << static_cast<int>(e.above)
                << " override=" << e.override_redirect;
              */
        }
        break;
      case CreateNotify:
        {
          XCreateWindowEvent& e = event.xcreatewindow;
          DEBUG << "CreateNotify: window=0x" << hex << e.window
                << " parent=0x" << e.parent << dec
                << " x=" << e.x << " y=" << e.y
                << " width=" << e.width << " height=" << e.height
                << " border=" << e.border_width
                << " override=" << e.override_redirect;
          XWindow* x_window = GetWindow(e.window, true);
          window_manager->HandleCreateWindow(x_window);
        }
        break;
      case DestroyNotify:
        {
          XDestroyWindowEvent& e = event.xdestroywindow;
          DEBUG << "DestroyNotify: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          CHECK(x_window);
          window_manager->HandleDestroyWindow(x_window);
        }
        break;
      case EnterNotify:
        {
          XCrossingEvent& e = event.xcrossing;
          DEBUG << "Enter: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          CHECK(x_window);
          window_manager->HandleEnterWindow(x_window);
        }
        break;
      case Expose:
        {
          XExposeEvent& e = event.xexpose;
          //DEBUG << "Expose: window=0x" << hex << e.window;
          XWindow* x_window = GetWindow(e.window, false);
          CHECK(x_window);
          window_manager->HandleExposeWindow(x_window);
        }
        break;
      case KeyPress:
        {
          XKeyEvent& e = event.xkey;
          DEBUG << "KeyPress: window=0x" << hex << e.window << dec
                << " keycode=" << e.keycode << " state=" << e.state;
          HandleKeyPress(XLookupKeysym(&e, 0), e.state, window_manager);
        }
        break;
      case KeyRelease:
        {
          XKeyEvent& e = event.xkey;
          DEBUG << "KeyRelease: window=0x" << hex << e.window << dec
                << " keycode=" << e.keycode << " state=" << e.state;
        }
        break;
      case MotionNotify:
        {
          XMotionEvent& e = event.xmotion;
          //DEBUG << "MotionNotify: window=0x" << hex << e.window << dec
          //      << " x=" << e.x_root << " y=" << e.y_root;
          XWindow* x_window = GetWindow(e.window, false);
          window_manager->HandleMotion(x_window, e.x_root, e.y_root);
        }
        break;
      default:
        DEBUG << XEventTypeToName(event.type);
    }
  }
}


XWindow* XServer::GetWindow(::Window id, bool create) {
  XWindowMap::iterator it = windows_.find(id);
  if (it != windows_.end()) return it->second.get();
  if (!create) return NULL;
  ref_ptr<XWindow> window(
      XWindow::testing_ ? new MockXWindow(id) : new XWindow(id));
  windows_.insert(make_pair(id, window));
  return window.get();
}


GC XServer::GetGC(const string& name) {
  return FindWithDefault(gcs_, name, default_gc_);
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
