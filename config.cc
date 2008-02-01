// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config.h"

#include "config-parser.h"

using namespace std;

namespace wham {

ref_ptr<Config> Config::singleton_(new Config());

bool Config::Load(const ParsedConfig& conf) {
  for (vector<ref_ptr<ParsedConfig::Node> >::const_iterator it =
         conf.root.children.begin();
       it != conf.root.children.end(); ++it) {
    const ParsedConfig::Node& node = *(it->get());
    if (node.tokens.empty()) {
      ERROR << "Got empty top-level node from config";
      return false;
    }
    if (node.tokens.size() == 1 && node.tokens[0] == "key_bindings") {
      if (!key_bindings.Load(node)) {
        ERROR << "Failed to parse bindings";
        return false;
      }
    } else if (node.tokens.size() == 1 && node.tokens[0] == "window_configs") {
      if (!window_classifier.Load(node)) {
        ERROR << "Failed to parse window configs";
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
