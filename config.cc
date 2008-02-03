// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config.h"

#include "config-parser.h"

using namespace std;

namespace wham {

ref_ptr<Config> Config::singleton_(new Config);


Config::Config()
    : window_classifier(new WindowClassifier),
      dragging_threshold(1),
      anchor_min_width(200),
      anchor_max_width(800),
      window_border(1) {}


Config::~Config() {}


bool Config::Load(const ConfigNode& conf) {
  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());
    if (node.tokens.empty()) {
      ERROR << "Got empty top-level node from config";
      return false;
    }
    if (node.tokens[0] == "key_bindings") {
      if (!key_bindings.Load(node)) {
        ERROR << "Failed to parse bindings";
        return false;
      }
    } else if (node.tokens[0] == "window") {
      if (!window_classifier->Load(node)) {
        ERROR << "Failed to parse window";
        return false;
      }
    } else {
      ERROR << "Got unknown node in config";
      return false;
    }
  }
  return true;
}

}  // namespace wham
