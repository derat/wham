// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "util.h"
#include "window-classifier.h"
#include "window-properties.h"

using namespace std;

class WindowTestSuite;

namespace wham {

class XWindow;

class Window {
 public:

  Window(XWindow* x_window);
  ~Window() {
    x_window_ = NULL;
  }

  void CycleConfig(bool forward);

  void Move(int x, int y);
  void Resize(uint width, uint height);
  void Map();
  void Unmap();
  void TakeFocus();
  void Raise();
  void MakeSibling(const XWindow& leader);

  // Handle a property change event on this window, reclassifying the
  // window if necessary.  Returns 'true' if the titlebar needs to be
  // redrawn.
  bool HandlePropertyChange(WindowProperties::ChangeType type);

  string title() const { return props_.window_name; }

  int x() const;
  int y() const;
  uint width() const;
  uint height() const;
  const WindowProperties& props() const { return props_; }

  bool tagged() const { return tagged_; }
  void set_tagged(bool tagged) { tagged_ = tagged; }

  XWindow* x_window() const { return x_window_; }
  XWindow* transient_for() const { return props_.transient_for; }

 private:
  friend class ::WindowTestSuite;

  // Classify this window, applying the active config afterwards.
  bool Classify();

  // Apply the active config.
  void ApplyActiveConfig() {
    const WindowConfig* config = configs_.GetActiveConfig();
    CHECK(config);
    ApplyConfig(*config);
  }

  // Apply a config.
  void ApplyConfig(const WindowConfig& config);

  // Update 'props_' with this window's properties.
  bool UpdateProperties(WindowProperties::ChangeType type, bool* changed);

  // A pointer to information about the X window; used for interacting with
  // the X server.
  XWindow* x_window_;  // not owned

  WindowProperties props_;

  WindowConfigSet configs_;

  bool tagged_;

  DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}  // namespace wham

#endif
