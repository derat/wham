// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include <map>
#include <vector>

#include "anchor.h"
#include "util.h"

using namespace std;

namespace wham {

class XWindow;

// A collection of anchors.
class Desktop {
 public:
  Desktop();

  // Create a new anchor.
  void CreateAnchor(const string& name, int x, int y);

  void AddWindow(Window* window);
  void RemoveWindow(Window* window);

  // Look up an anchor based on its titlebar.
  Anchor* GetAnchorByTitlebar(XWindow* titlebar) const;

  void SetActiveAnchor(Anchor* anchor);

  bool IsTitlebarWindow(XWindow* x_window) const {
    return anchor_titlebars_.count(x_window);
  }

  Anchor* active_anchor() { return active_anchor_; }

 private:
  // Get the anchor nearest to the currently-active anchor in the
  // specified direction.
  Anchor* GetNearestAnchor(const string& direction) const;

  // Anchors contained within this desktop.
  typedef vector<ref_ptr<Anchor> > AnchorVector;
  AnchorVector anchors_;

  // Map from each anchor's titlebar's X window to the anchor itself.
  typedef map<XWindow*, Anchor*> AnchorTitlebarMap;
  AnchorTitlebarMap anchor_titlebars_;

  typedef map<Window*, Anchor*> WindowAnchorMap;
  WindowAnchorMap window_anchors_;

  // Index into 'anchors_' of the currently-active anchor.
  Anchor* active_anchor_;
  size_t active_anchor_index_;

  DISALLOW_EVIL_CONSTRUCTORS(Desktop);
};

}  // namespace wham

#endif
