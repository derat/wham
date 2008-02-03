// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "key-bindings.h"
#include "util.h"
#include "window-classifier.h"

using namespace std;

namespace wham {

struct ConfigNode;

struct Config {
 public:
  Config();
  ~Config();

  static Config* Get() {
    CHECK(singleton_.get());
    return singleton_.get();
  }

  static void Swap(ref_ptr<Config> new_config) {
    singleton_.swap(new_config);
  }

  bool Load(const ConfigNode& conf);

  KeyBindings key_bindings;
  ref_ptr<WindowClassifier> window_classifier;

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
