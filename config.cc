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
      window_border(1),
      mouse_primary_button(1),
      mouse_secondary_button(3) {}


Config::~Config() {}


bool Config::Load(const string& filename, vector<ConfigError>* errors) {
  CHECK(errors);

  ConfigNode conf;
  if (!ConfigParser::ParseFromFile(filename, &conf, errors)) {
    string msg = StringPrintf("Couldn't parse file \"%s\"", filename.c_str());
    errors->push_back(ConfigError(msg, 0));
    return false;
  }

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());
    if (node.tokens.empty()) {
      errors->push_back(
          ConfigError("Got empty top-level block", node.line_num));
    } else if (node.tokens[0] == "key_bindings") {
      if (!key_bindings.Load(node, errors)) {
        errors->push_back(
            ConfigError("Couldn't load key bindings", node.line_num));
      }
    } else if (node.tokens[0] == "window") {
      if (!window_classifier->Load(node, errors)) {
        errors->push_back(
            ConfigError("Couldn't load window config", node.line_num));
      }
    } else {
      string msg = StringPrintf("Got unknown top-level block \"%s\"",
                                JoinString(node.tokens, " ").c_str());
      errors->push_back(ConfigError(msg, node.line_num));
    }
  }
  return true;
}

}  // namespace wham
