// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include <map>
#include <vector>

#include "command.h"
#include "util.h"

using namespace std;

class DesktopTestSuite;

namespace wham {

class Anchor;
class Window;
class XWindow;

// A collection of anchors.
class Desktop {
 public:
  Desktop();

  // Hide or show this desktop (by updating 'visible_' and hiding or
  // showing all anchors on it).
  void Hide();
  void Show();

  // Create a new anchor.
  Anchor* CreateAnchor(const string& name, int x, int y);

  // Add an anchor to this desktop.  Ownership is transferred to the
  // desktop.  The anchor is shown or hidden depending on whether the
  // desktop is visible or hidden, respectively.
  void AddAnchor(Anchor* anchor);

  // Remove an anchor from this desktop.  The anchor is not deleted, but
  // ownership of it is released; the caller is responsible for deleting
  // 'anchor'.  The visibility of the anchor isn't changed; AddAnchor()
  // takes care of that.
  void RemoveAnchor(Anchor *anchor);

  // Add 'window' to the active anchor.
  void AddWindow(Window* window);

  // Add 'window' to 'anchor' on this desktop.
  void AddWindowToAnchor(Window* window, Anchor* anchor);

  // Remove 'window' from an anchor on this desktop, if it's present in
  // one.  Does nothing if it's not.
  void RemoveWindow(Window* window);

  // Look up an anchor based on its titlebar.
  Anchor* GetAnchorByTitlebar(const XWindow* titlebar) const;

  // Get all anchors with a titlebar covering a given position.
  void GetAnchorsAtPosition(int x, int y, vector<Anchor*>* anchors) const;

  // Make the passed-in anchor, which must be on this desktop, be active.
  // Makes the previously-active anchor unactive first.
  void SetActiveAnchor(Anchor* anchor);

  void SetAttachAnchor(Anchor* anchor);

  bool IsTitlebarWindow(const XWindow* xwin) const {
    return anchor_titlebars_.count(xwin);
  }

  const string& name() const { return name_; }
  bool visible() const { return visible_; }
  Anchor* active_anchor() { return active_anchor_; }
  Anchor* attach_anchor() { return attach_anchor_; }

  // Get the anchor nearest to the currently-active anchor in the
  // specified direction.
  Anchor* GetNearestAnchor(Command::Direction dir) const;

  // Returns true if this desktop contains 'anchor' and false otherwise.
  bool HasAnchor(const Anchor* anchor) const;

 private:
  friend class ::DesktopTestSuite;

  // Remove 'anchor' from the desktop and delete it.
  void DestroyAnchor(Anchor* anchor);

  // Get the index of 'anchor' within 'anchors_'.
  // Returns -1 if it's not present.
  int GetAnchorIndex(const Anchor* anchor) const;

  // The desktop's name.
  string name_;

  // Is this desktop visible?
  bool visible_;

  // Anchors contained within this desktop.
  typedef vector<ref_ptr<Anchor> > AnchorVector;
  AnchorVector anchors_;

  // Map from each anchor's titlebar's X window to the anchor itself.
  typedef map<const XWindow*, Anchor*> AnchorTitlebarMap;
  AnchorTitlebarMap anchor_titlebars_;

  // Map from a window present on this desktop to the anchor that contains
  // it.  At most one anchor on a desktop can contain a given window.
  typedef map<Window*, Anchor*> WindowAnchorMap;
  WindowAnchorMap window_anchors_;

  // Currently-active (focused) anchor.
  Anchor* active_anchor_;

  // Anchor to which new windows should be attached by default.
  Anchor* attach_anchor_;

  DISALLOW_EVIL_CONSTRUCTORS(Desktop);
};

}  // namespace wham

#endif
