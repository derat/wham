// Copyright 2008 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "desktop.h"

#include <algorithm>
#include <climits>

#include "anchor.h"
#include "window.h"

namespace wham {

Desktop::Desktop()
    : visible_(false),
      active_anchor_(NULL),
      attach_anchor_(NULL) {
  static int num = 0;
  name_ = StringPrintf("desktop%d", num);
  num++;
}


void Desktop::Hide() {
  visible_ = false;
  for (vector<ref_ptr<Anchor> >::const_iterator anchor = anchors_.begin();
       anchor != anchors_.end(); ++anchor) {
    (*anchor)->Hide();
  }
}


void Desktop::Show() {
  visible_ = true;
  for (vector<ref_ptr<Anchor> >::const_iterator anchor = anchors_.begin();
       anchor != anchors_.end(); ++anchor) {
    (*anchor)->Show();
  }
}


Anchor* Desktop::CreateAnchor(const string& name, int x, int y) {
  Anchor* anchor = new Anchor(name, x, y);
  DEBUG << "Created anchor " << anchor->DebugString();
  AddAnchor(anchor);
  return anchor;
}


void Desktop::AddAnchor(Anchor* anchor) {
  DEBUG << "Adding anchor " << anchor->DebugString()
        << " to desktop " << DebugString();
  CHECK(anchor);
  CHECK(anchor->desktop() == NULL);

  anchor->set_desktop(this);
  anchors_.push_back(ref_ptr<Anchor>(anchor));
  anchor_titlebars_.insert(make_pair(anchor->titlebar(), anchor));
  if (anchors_.size() == 1U) {
    SetActiveAnchor(anchor);
    SetAttachAnchor(anchor);
  }

  if (visible_) {
    anchor->Show();
  } else {
    anchor->Hide();
  }
}


void Desktop::RemoveAnchor(Anchor *anchor) {
  DEBUG << "Removing anchor " << anchor->DebugString()
        << " from desktop " << DebugString();
  CHECK(anchor);
  CHECK(anchor->desktop() == this);

  // Choose a new anchor before we remove this one.
  // FIXME: add more intelligent logic for choosing the new anchor
  if (active_anchor_ == anchor || attach_anchor_ == anchor) {
    Anchor* replacement = anchors_.size() == 1 ? NULL :
        (anchors_[0].get() != anchor ? anchors_[0].get() : anchors_[1].get());
    if (active_anchor_ == anchor) SetActiveAnchor(replacement);
    if (attach_anchor_ == anchor) SetAttachAnchor(replacement);
  }

  anchor->set_desktop(NULL);
  anchor_titlebars_.erase(anchor->titlebar());
  int index = GetAnchorIndex(anchor);
  CHECK(index >= 0);
  anchors_[index].release();
  anchors_.erase(anchors_.begin() + index);
}


void Desktop::AddWindow(Window* window) {
  // FIXME: Add logic here to determine to where the window appears in the
  // list and whether it's automatically focused, instead of just adding it
  // at the end.
  AddWindowToAnchor(window, attach_anchor_);
}


void Desktop::AddWindowToAnchor(Window* window, Anchor* anchor) {
  CHECK(window);
  CHECK(anchor);
  CHECK(!IsTitlebarWindow(window->xwin()));
  anchor->AddWindow(window);
  window_anchors_.insert(make_pair(window, anchor));
}


void Desktop::RemoveWindow(Window* window) {
  Anchor* anchor =
      FindWithDefault(window_anchors_, window, static_cast<Anchor*>(NULL));
  if (anchor) {
    anchor->RemoveWindow(window);
    window_anchors_.erase(window);
    // FIXME: let the anchor do this
    if (anchor->windows().empty() && anchor->temporary()) {
      DestroyAnchor(anchor);
    }
  }
}


Anchor* Desktop::GetAnchorByTitlebar(const XWindow* titlebar) const {
  CHECK(titlebar);
  return FindWithDefault(
      anchor_titlebars_, titlebar, static_cast<Anchor*>(NULL));
}


void Desktop::GetAnchorsAtPosition(
    int x, int y, vector<Anchor*>* anchors) const {
  CHECK(anchors);
  anchors->clear();
  for (AnchorVector::const_iterator anchor = anchors_.begin();
       anchor != anchors_.end(); ++anchor) {
    if ((*anchor)->TitlebarIsOverPoint(x, y)) {
      anchors->push_back((*anchor).get());
    }
  }
}


void Desktop::SetActiveAnchor(Anchor* anchor) {
  if (anchor == active_anchor_) return;
  if (active_anchor_) active_anchor_->SetActive(false);
  active_anchor_ = anchor;
  if (anchor) {
    CHECK(find(anchors_.begin(), anchors_.end(), anchor) != anchors_.end());
    anchor->SetActive(true);
  }
}


void Desktop::SetAttachAnchor(Anchor* anchor) {
  if (anchor == attach_anchor_) return;
  if (attach_anchor_) attach_anchor_->SetAttach(false);
  attach_anchor_ = anchor;
  if (anchor) {
    CHECK(find(anchors_.begin(), anchors_.end(), anchor) != anchors_.end());
    anchor->SetAttach(true);
  }
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


bool Desktop::HasAnchor(const Anchor* anchor) const {
  CHECK(anchor);
  return GetAnchorIndex(anchor) != -1;
}


string Desktop::DebugString() const {
  return StringPrintf("%p (%s)", this, name_.c_str());
}


void Desktop::DestroyAnchor(Anchor* anchor) {
  CHECK(anchor);
  DEBUG << "Destroying anchor " << anchor->DebugString();
  // FIXME: figure out what should be done wrt closing anchors that still
  // contain windows
  CHECK(anchor->windows().empty());
  RemoveAnchor(anchor);
  delete anchor;
}


int Desktop::GetAnchorIndex(const Anchor* anchor) const {
  CHECK(anchor);
  for (uint i = 0; i < anchors_.size(); ++i) {
    if (anchors_[i].get() == anchor) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

}  // namespace wham
