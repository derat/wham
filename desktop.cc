// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "desktop.h"
#include "window.h"

namespace wham {

Desktop::Desktop()
    : active_anchor_(NULL),
      active_anchor_index_(0) {
  // Create a few anchors just for testing.
  CreateAnchor("anchor1", 50, 50);
  CreateAnchor("anchor2", 300, 300);
  SetActiveAnchor(anchors_[0].get());
}


void Desktop::CreateAnchor(const string& name, int x, int y) {
  ref_ptr<Anchor> anchor(new Anchor(name, x, y));
  anchors_.push_back(anchor);
  anchor_titlebars_.insert(make_pair(anchor->titlebar(), anchor.get()));
}


Anchor* Desktop::GetAnchorByTitlebar(XWindow* titlebar) const {
  return FindWithDefault(
      anchor_titlebars_, titlebar, static_cast<Anchor*>(NULL));
}


void Desktop::SetActiveAnchor(Anchor* anchor) {
  for (size_t i = 0; i < anchors_.size(); ++i) {
    if (anchor == anchors_[i].get()) {
      active_anchor_ = anchors_[i].get();
      active_anchor_index_ = i;
      return;
    }
  }
  CHECK(false);
}


void Desktop::AddWindow(Window* window) {
  CHECK(!IsTitlebarWindow(window->x_window()));

  // FIXME: Add logic here to determine to which anchor the new
  // window is added, where it appears in the list, and whether it's
  // automatically focused, instead of just adding it at the end of the
  // current anchor.
  CHECK(active_anchor_);
  active_anchor_->AddWindow(window);
  active_anchor_->SetActive(active_anchor_->NumWindows()-1);
  window_anchors_.insert(make_pair(window, active_anchor_));
}


void Desktop::RemoveWindow(Window* window) {
  Anchor* anchor =
      FindWithDefault(window_anchors_, window, static_cast<Anchor*>(NULL));
  if (anchor) {
    anchor->RemoveWindow(window);
    window_anchors_.erase(window);
  }
}


Anchor* Desktop::GetNearestAnchor(const string& direction) const {
  int dx = 0, dy = 0;
  if (direction == "left")       dx = -1;
  else if (direction == "right") dx =  1;
  else if (direction == "up")    dy = -1;
  else if (direction == "down")  dy =  1;
  else return NULL;

  Anchor* nearest = NULL;
  int nearest_dist = INT_MAX;
  for (AnchorVector::const_iterator anchor = anchors_.begin();
       anchor != anchors_.end(); ++anchor) {
    if (anchor->get() == active_anchor_) continue;
    int dist = 0;
    if (dy != 0) {
      dist = (*anchor)->y() - active_anchor_->y();
      if (dy * dist < dy) continue;
      // FIXME: also need to check that the anchor isn't too far to the
      // side of the titlebar
    } else {
      dist = (*anchor)->x() - active_anchor_->x();
      if (dx * dist < dx) continue;
    }
    if (dist < nearest_dist) {
      nearest = anchor->get();
      nearest_dist = dist;
    }
  }
  return nearest;
}

}  // namespace wham