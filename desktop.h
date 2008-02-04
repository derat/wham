// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include <map>
#include <vector>

#include "anchor.h"
#include "command.h"
#include "util.h"

using namespace std;

class DesktopTestSuite;

namespace wham {

class XWindow;

// A collection of anchors.
class Desktop {
 public:
  Desktop();

  // Create a new anchor.
  Anchor* CreateAnchor(const string& name, int x, int y);

  void AddWindow(Window* window);
  void RemoveWindow(Window* window);

  // Look up an anchor based on its titlebar.
  // FIXME: Why can't I make 'titlebar' const?
  Anchor* GetAnchorByTitlebar(XWindow* titlebar) const;

  // Find the anchor containing the passed-in window.
  // Returns NULL if no such anchor exists.
  Anchor* GetAnchorContainingWindow(Window* window) const;

  void SetActiveAnchor(Anchor* anchor);

  bool IsTitlebarWindow(XWindow* x_window) const {
    return anchor_titlebars_.count(x_window);
  }

  Anchor* active_anchor() { return active_anchor_; }

  // Get the anchor nearest to the currently-active anchor in the
  // specified direction.
  Anchor* GetNearestAnchor(Command::Direction dir) const;

 private:
  friend class ::DesktopTestSuite;

  // Anchors contained within this desktop.
  typedef vector<ref_ptr<Anchor> > AnchorVector;
  AnchorVector anchors_;

  // Map from each anchor's titlebar's X window to the anchor itself.
  typedef map<XWindow*, Anchor*> AnchorTitlebarMap;
  AnchorTitlebarMap anchor_titlebars_;

  // Map from a window present on this desktop to the anchor that contains
  // it.  At most one anchor on a desktop can contain a given window.
  typedef map<Window*, Anchor*> WindowAnchorMap;
  WindowAnchorMap window_anchors_;

  // Currently-active anchor.
  Anchor* active_anchor_;

  DISALLOW_EVIL_CONSTRUCTORS(Desktop);
};

}  // namespace wham

#endif
