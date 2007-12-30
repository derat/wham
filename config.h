// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "util.h"

using namespace std;

namespace wham {

struct Config;

extern ref_ptr<Config> config;

struct Config {
 public:
  Config()
      : dragging_threshold(1),
        titlebar_font("fixed"),
        titlebar_padding(3),
        titlebar_border(1),
        titlebar_min_width(200),
        titlebar_max_width(800) {
  }

  static void Swap(ref_ptr<Config> new_config) {
    config.swap(new_config);
  }

  int dragging_threshold;

  string titlebar_font;
  int titlebar_padding;
  int titlebar_border;
  int titlebar_min_width;
  int titlebar_max_width;

  DISALLOW_EVIL_CONSTRUCTORS(Config);
};

}  // namespace wham

#endif
