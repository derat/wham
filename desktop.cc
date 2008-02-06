// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "desktop.h"
#include "window.h"

namespace wham {

Desktop::Desktop()
    : active_anchor_(NULL) {
}


Anchor* Desktop::CreateAnchor(const string& name, int x, int y) {
  ref_ptr<Anchor> anchor(new Anchor(name, x, y));
  anchors_.push_back(anchor);
  anchor_titlebars_.insert(make_pair(anchor->titlebar(), anchor.get()));
  if (anchors_.size() == 1U) SetActiveAnchor(anchor.get());
  return anchor.get();
}


void Desktop::AddWindow(Window* window) {
  CHECK(!IsTitlebarWindow(window->x_window()));

  // FIXME: Add logic here to determine to which anchor the new
  // window is added, where it appears in the list, and whether it's
  // automatically focused, instead of just adding it at the end of the
  // current anchor.
  CHECK(active_anchor_);
  active_anchor_->AddWindow(window);
  active_anchor_->SetActive(active_anchor_->windows().size()-1);
  window_anchors_.insert(make_pair(window, active_anchor_));
}


void Desktop::RemoveWindow(Window* window) {
  Anchor* anchor =
      FindWithDefault(window_anchors_, window, static_cast<Anchor*>(NULL));
  if (anchor) {
    anchor->RemoveWindow(window);
    window_anchors_.erase(window);
    if (anchor->windows().empty() && !anchor->persistent()) {
      DestroyAnchor(anchor);
    }
  }
}


Anchor* Desktop::GetAnchorByTitlebar(XWindow* titlebar) const {
  CHECK(titlebar);
  return FindWithDefault(
      anchor_titlebars_, titlebar, static_cast<Anchor*>(NULL));
}


Anchor* Desktop::GetAnchorContainingWindow(Window* window) const {
  CHECK(window);
  return FindWithDefault(
      window_anchors_, window, static_cast<Anchor*>(NULL));
}


void Desktop::SetActiveAnchor(Anchor* anchor) {
  if (anchor == active_anchor_) return;
  CHECK(find(anchors_.begin(), anchors_.end(), anchor) != anchors_.end());
  active_anchor_ = anchor;
  anchor->FocusActiveWindow();
}


Anchor* Desktop::GetNearestAnchor(Command::Direction dir) const {
  int dx = 0, dy = 0;
  if (dir == Command::LEFT)       dx = -1;
  else if (dir == Command::RIGHT) dx =  1;
  else if (dir == Command::UP)    dy = -1;
  else if (dir == Command::DOWN)  dy =  1;
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


void Desktop::DestroyAnchor(Anchor* anchor) {
  CHECK(anchor);
  // FIXME: figure out what should be done wrt closing anchors that still
  // contain windows
  CHECK(anchor->windows().empty());

  DEBUG << "Destroying anchor " << anchor->name();
  anchor_titlebars_.erase(anchor->titlebar());
  int index = GetAnchorIndex(anchor);
  CHECK(index >= 0);
  anchors_.erase(anchors_.begin() + index);

  if (active_anchor_ == anchor) {
    if (anchors_.empty()) {
      active_anchor_ = NULL;
    } else {
      // FIXME: Do something more intelligent here.
      active_anchor_ = anchors_[0].get();
    }
  }
}


int Desktop::GetAnchorIndex(Anchor* anchor) {
  CHECK(anchor);
  for (uint i = 0; i < anchors_.size(); ++i) {
    if (anchors_[i].get() == anchor) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

}  // namespace wham
