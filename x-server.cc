// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "x-server.h"

#include <algorithm>
#include <ctime>

#include "sys/select.h"
#include "sys/time.h"
#include "X11/Xatom.h"

#include "config.h"
#include "key-bindings.h"
#include "mock-x-window.h"
#include "util.h"
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

  if (testing_) {
    // FIXME: do this more cleanly
    width_ = 1024;
    height_ = 768;
  } else {
    display_ = XOpenDisplay(NULL);
    if (display_ == NULL) {
      ERROR << "Can't open display " << XDisplayName(NULL);
      return false;
    }
    screen_num_ = DefaultScreen(display_);
    root_ = RootWindow(display_, screen_num_);

    // FIXME
    XSynchronize(display_, True);

    ::Window root_ret;
    int x, y;
    uint border_width, depth;
    XGetGeometry(display_, root_, &root_ret, &x, &y,
                 &width_, &height_, &border_width, &depth);

    XSelectInput(display_, root_,
                 SubstructureRedirectMask | StructureNotifyMask);
  }

  initialized_ = true;
  return true;
}


void XServer::RunEventLoop(WindowManager* window_manager) {
  CHECK(window_manager);
  CHECK(initialized_);

  int x11_fd = XConnectionNumber(display_);
  DEBUG << "X11 connection is on fd " << x11_fd;
  // FIXME: need to also use XAddConnectionWatch()?

  while (true) {
    while (XPending(display_)) {
      ProcessEvent(window_manager);
    }

    double now = GetCurrentTime();
    while (!timeout_heap_.empty() &&
           timeout_heap_[0].time <= now) {
      DEBUG << "Running timeout for " << fixed << timeout_heap_[0].time;
      (*timeout_heap_[0].func.get())();
      pop_heap(timeout_heap_.begin(), timeout_heap_.end());
      timeout_heap_.pop_back();
    }

    struct timeval tv;
    struct timeval *timeout_tv = NULL;
    if (!timeout_heap_.empty()) {
      FillTimeval(timeout_heap_[0].time - now, &tv);
      timeout_tv = &tv;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(x11_fd, &fds);
    CHECK(select(x11_fd + 1, &fds, NULL, NULL, timeout_tv) != -1);
  }
}


void XServer::RegisterTimeout(TimeoutFunction *func, double timeout_sec) {
  CHECK(func);
  CHECK(timeout_sec >= 0);

  Timeout timeout(func, GetCurrentTime() + timeout_sec);
  DEBUG << "Registering timeout for " << fixed << timeout.time;
  timeout_heap_.push_back(timeout);
  push_heap(timeout_heap_.begin(), timeout_heap_.end());
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


void XServer::ProcessEvent(WindowManager *window_manager) {
  XEvent event;
  XNextEvent(display_, &event);

  if (event.type == ButtonPress) {
    XButtonEvent& e = event.xbutton;
    DEBUG << "ButtonPress: window=0x" << hex << e.window << dec
          << " x=" << e.x_root << " y=" << e.y_root << " button=" << e.button;
    XWindow* xwin = GetWindow(e.window, false);
    if (xwin) {
      window_manager->HandleButtonPress(xwin, e.x_root, e.y_root, e.button);
    }
  } else if (event.type == ButtonRelease) {
    XButtonEvent& e = event.xbutton;
    DEBUG << "ButtonRelease: window=0x" << hex << e.window << dec
          << " x=" << e.x_root << " y=" << e.y_root << " button=" << e.button;
    XWindow* xwin = GetWindow(e.window, false);
    if (xwin) {
      window_manager->HandleButtonRelease(xwin, e.x_root, e.y_root, e.button);
    }
  } else if (event.type == ConfigureNotify) {
    // We don't care about these.
  } else if (event.type == DestroyNotify) {
    XDestroyWindowEvent& e = event.xdestroywindow;
    DEBUG << "DestroyNotify: window=0x" << hex << e.window;
    XWindow* xwin = GetWindow(e.window, false);
    if (xwin) DeleteWindow(e.window);
  } else if (event.type == EnterNotify) {
    XCrossingEvent& e = event.xcrossing;
    DEBUG << "Enter: window=0x" << hex << e.window;
    XWindow* xwin = GetWindow(e.window, false);
    // This could for a border window that we just deleted.
    if (xwin) window_manager->HandleEnterWindow(xwin);
  } else if (event.type == Expose) {
    // Coalesce expose events for the same window to avoid redrawing the
    // same one more than necessary.
    // TODO: Is this needed?  Doesn't appear to have much of an effect on
    // my computer, but maybe with a slower machine or slower drawing
    // code it could help.
    set<XWindow*> exposed_windows;
    do {
      XExposeEvent& e = event.xexpose;
      DEBUG << "Expose: window=0x" << hex << e.window;
      XWindow* xwin = GetWindow(e.window, false);
      // This could be for a border window that we just deleted.
      if (xwin == NULL) continue;
      exposed_windows.insert(xwin);
    } while (XCheckMaskEvent(display_, ExposureMask, &event) == True);
    for (set<XWindow*>::iterator win = exposed_windows.begin();
         win != exposed_windows.end(); ++win) {
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
  } else if (event.type == MappingNotify) {
    XMappingEvent& e = event.xmapping;
    DEBUG << "MappingNotify";
    XRefreshKeyboardMapping(&e);
  } else if (event.type == MapRequest) {
    XMapRequestEvent& e = event.xmaprequest;
    DEBUG << "MapRequest: window=0x" << hex << e.window;
    XWindow* xwin = GetWindow(e.window, true);
    window_manager->HandleMapRequest(xwin);
  } else if (event.type == MotionNotify) {
    XMotionEvent& e = event.xmotion;
    //DEBUG << "MotionNotify: window=0x" << hex << e.window << dec
    //      << " x=" << e.x_root << " y=" << e.y_root;
    XWindow* xwin = GetWindow(e.window, false);
    if (xwin) window_manager->HandleMotion(xwin, e.x_root, e.y_root);
  } else if (event.type == PropertyNotify) {
    XPropertyEvent& e = event.xproperty;
    XWindow* xwin = GetWindow(e.window, false);
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
      window_manager->HandlePropertyChange(xwin, type);
    }
  } else if (event.type == UnmapNotify) {
    XUnmapEvent& e = event.xunmap;
    DEBUG << "UnmapNotify: window=0x" << hex << e.window;
    XWindow* xwin = GetWindow(e.window, false);
    if (xwin) window_manager->HandleUnmapWindow(xwin);
  } else {
    DEBUG << XEventTypeToName(event.type);
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


KeySym XServer::LowercaseKeysym(KeySym keysym) {
  KeySym lower_keysym = NoSymbol;
  KeySym upper_keysym = NoSymbol;
  XConvertCase(keysym, &lower_keysym, &upper_keysym);
  return lower_keysym;
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
      KeySym keysym = LowercaseKeysym(XStringToKeysym(combo->key.c_str()));
      if (keysym == NoSymbol) {
        ERROR << "Unknown symbol \"" << combo->key << "\" in key binding "
              << binding->ToString() << " for command "
              << binding->command.ToString();
        error_in_combo = true;
        break;
      }

      if (parent_binding == NULL) {
        // If this is the first combo in the sequence, then it needs to go
        // into the top-level map.  If we already have it registered as
        // part of another binding, we'll just use that.
        pair<KeySym, uint> key = make_pair(keysym, mods);
        if (binding_map->find(key) == binding_map->end()) {
          ref_ptr<XKeyBinding> xbinding(
              new XKeyBinding(keysym, mods, 0, Command()));
          binding_map->insert(make_pair(key, xbinding));
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
        for (vector<ref_ptr<XKeyBinding> >::iterator xbinding =
               parent_binding->children.begin();
             xbinding != parent_binding->children.end(); ++xbinding) {
          CHECK_EQ((*xbinding)->inherited_mods, inherited_mods);
          if ((*xbinding)->required_mods == mods &&
              (*xbinding)->keysym == keysym) {
            DEBUG << "Using existing child binding:"
                  << " keysym=" << keysym << " mods=" << mods
                  << " inherited_mods=" << inherited_mods;
            parent_binding = (*xbinding).get();
            found = true;
            break;
          }
        }
        if (!found) {
          ref_ptr<XKeyBinding> xbinding(
              new XKeyBinding(keysym, mods, inherited_mods, Command()));
          parent_binding->children.push_back(xbinding);
          parent_binding = xbinding.get();
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


void XServer::HandleKeyPress(KeySym keysym,
                             uint mods,
                             WindowManager* window_manager) {
  keysym = LowercaseKeysym(keysym);
  XKeyBinding* binding = NULL;

  if (!in_progress_binding_) {
    binding = FindWithDefault(bindings_, XKeyCombo(keysym, mods),
                              ref_ptr<XKeyBinding>(NULL)).get();
  } else {
    KeySym abort_key = LowercaseKeysym(
        XStringToKeysym(Config::Get()->keybinding_abort_key.c_str()));
    if (keysym == abort_key) {
      DEBUG << "Key binding aborted; ungrabbing keyboard";
      XUngrabKeyboard(display_, CurrentTime);
      in_progress_binding_ = NULL;
      return;
    }

    for (vector<ref_ptr<XKeyBinding> >::iterator it =
           in_progress_binding_->children.begin();
         it != in_progress_binding_->children.end(); ++it) {
      // Check that the key press has all of the required modifiers,
      // and no modifiers other than the required or inherited ones.
      if (keysym == (*it)->keysym &&
          (mods & (*it)->required_mods) == (*it)->required_mods &&
          (mods & ~((*it)->inherited_mods | (*it)->required_mods)) == 0) {
        binding = it->get();
        break;
      }
    }
  }

  if (binding == NULL) {
    if (!in_progress_binding_) {
      ERROR << "Ignoring key press without binding (keysym=0x"
            << hex << keysym << " mods=0x" << mods;
    } else {
      DEBUG << "Key binding aborted; ungrabbing keyboard";
      XUngrabKeyboard(display_, CurrentTime);
      in_progress_binding_ = NULL;
    }
    return;
  }

  if (!binding->children.empty()) {
    if (!in_progress_binding_) {
      DEBUG << "Grabbing keyboard";
      XGrabKeyboard(display_, root_, False, GrabModeAsync, GrabModeAsync,
                    CurrentTime);
      // FIXME: Also grab the pointer and change the cursor?  It'd probably
      // make sense for a mouse click to also abort the keyboard grab.
    }
    // FIXME: Reset in_progress_binding_ and ungrab the keyboard before the
    // config is reloaded.
    in_progress_binding_ = binding;
  } else {
    if (in_progress_binding_) {
      DEBUG << "Ungrabbing keyboard";
      XUngrabKeyboard(display_, CurrentTime);
      in_progress_binding_ = NULL;
    }
  }

  if (binding->command.type() != Command::UNKNOWN) {
    window_manager->HandleCommand(binding->command);
  }
}

}  // namespace wham
