// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "util.h"

using namespace std;

namespace wham {

struct Config;

struct Config {
 public:
  Config()
      : dragging_threshold(1),
        anchor_font("fixed"),
        anchor_padding(3),
        anchor_border_width(1),
        anchor_min_width(200),
        anchor_max_width(800),
        anchor_border_color("#9a9cab"),
        anchor_inactive_bg_color("#3d4479"),
        anchor_inactive_text_color("#9a9cab"),
        anchor_active_unfocused_bg_color("#3d4479"),
        anchor_active_unfocused_text_color("#9a9cab"),
        anchor_active_focused_bg_color("#ffd200"),
        anchor_active_focused_text_color("black") {
  }

  static Config* Get() {
    CHECK(singleton_.get());
    return singleton_.get();
  }

  static void Swap(ref_ptr<Config> new_config) {
    singleton_.swap(new_config);
  }

  int dragging_threshold;

  string anchor_font;
  uint anchor_padding;
  uint anchor_border_width;
  uint anchor_min_width;
  uint anchor_max_width;
  string anchor_border_color;

  string anchor_inactive_bg_color;
  string anchor_inactive_text_color;

  string anchor_active_unfocused_bg_color;
  string anchor_active_unfocused_text_color;

  string anchor_active_focused_bg_color;
  string anchor_active_focused_text_color;

  DISALLOW_EVIL_CONSTRUCTORS(Config);

 private:
  static ref_ptr<Config> singleton_;
};

}  // namespace wham

#endif
