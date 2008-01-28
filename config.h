// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "key-bindings.h"
#include "util.h"
#include "window-classifier.h"

using namespace std;

namespace wham {

struct ParsedConfig;

struct Config {
 public:
  Config()
      : dragging_threshold(1),
        anchor_min_width(200),
        anchor_max_width(800),
        window_border(1) {
  }

  static Config* Get() {
    CHECK(singleton_.get());
    return singleton_.get();
  }

  static void Swap(ref_ptr<Config> new_config) {
    singleton_.swap(new_config);
  }

  bool Load(const ParsedConfig& conf);

  KeyBindings key_bindings;
  WindowClassifier window_classifier;

  int dragging_threshold;

  uint anchor_min_width;
  uint anchor_max_width;

  uint window_border;

  DISALLOW_EVIL_CONSTRUCTORS(Config);

 private:
  static ref_ptr<Config> singleton_;
};

}  // namespace wham

#endif
