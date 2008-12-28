// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-classifier.h"

#include <sstream>

#include "window.h"

namespace wham {

ref_ptr<WindowClassifier> WindowClassifier::singleton_(new WindowClassifier);


void WindowConfig::Merge(const WindowConfig& config) {
  name = config.name;
  // FIXME: Think about this some more.
  if (config.width_type != DIMENSION_APP) {
    width_type = config.width_type;
    width = config.width;
  }
  if (config.height_type != DIMENSION_APP) {
    height_type = config.height_type;
    height = config.height;
  }
}


string WindowConfig::DebugString() const {
  ostringstream out;
  out << "name=" << name
      << " width=" << width
      << " (" << DimensionTypeToStr(width_type) << ")"
      << " height=" << height
      << " (" << DimensionTypeToStr(height_type) << ")";
  return out.str();
}


void WindowConfigSet::MergeConfig(const WindowConfig& config) {
  // FIXME: store map from config name to config ptr so we don't need to
  // iterate through all of them
  for (WindowConfigVector::iterator it = configs_.begin();
       it != configs_.end(); ++it) {
    if (config.name == (*it)->name) {
      (*it)->Merge(config);
      return;
    }
  }
  configs_.push_back(ref_ptr<WindowConfig>(new WindowConfig(config)));
}


void WindowConfigSet::CycleActiveConfig(bool forward) {
  if (configs_.size() <= 1) return;
  active_ = (active_ + configs_.size() + (forward ? 1 : -1)) %
      configs_.size();
}


bool WindowConfigSet::SetActiveConfigByName(const string& name) {
  for (uint i = 0; i < configs_.size(); ++i) {
    if (configs_[i]->name == name) {
      active_ = i;
      return true;
    }
  }
  return false;
}


bool WindowCriteria::AddCriterion(CriterionType type, const string& pattern) {
  if (pattern.size() >= 2 &&
      pattern[0] == '/' &&
      pattern[pattern.size()-1] == '/') {
    ref_ptr<pcrecpp::RE> re(
        new pcrecpp::RE(pattern.substr(1, pattern.size()-2)));
    regexp_criteria_.push_back(make_pair(type, re));
  } else {
    substr_criteria_.push_back(make_pair(type, pattern));
  }
  return true;
}


void WindowCriteria::Reset() {
  regexp_criteria_.clear();
  substr_criteria_.clear();
}


bool WindowCriteria::Matches(const WindowProperties& props) const {
  // We iterate through the different criteria, trying to find one that
  // doesn't match.  We check the fastest match types first.
  for (SubstringCriteria::const_iterator it = substr_criteria_.begin();
       it != substr_criteria_.end(); ++it) {
    CriterionType type = it->first;
    const string& pattern = it->second;
    if (GetPropertyForCriterionType(props, type).find(pattern) ==
        string::npos) {
      return false;
    }
  }

  for (RegexpCriteria::const_iterator it = regexp_criteria_.begin();
       it != regexp_criteria_.end(); ++it) {
    CriterionType type = it->first;
    ref_ptr<pcrecpp::RE> re = it->second;
    if (!re->PartialMatch(GetPropertyForCriterionType(props, type))) {
      return false;
    }
  }

  return true;
}


const string& WindowCriteria::GetPropertyForCriterionType(
    const WindowProperties& props, CriterionType type) {
  static const string true_str = "true";
  static const string false_str = "false";
  switch (type) {
    case CRITERION_TYPE_WINDOW_NAME:
      return props.window_name;
    case CRITERION_TYPE_ICON_NAME:
      return props.icon_name;
    case CRITERION_TYPE_COMMAND:
      return props.command;
    case CRITERION_TYPE_APP_NAME:
      return props.app_name;
    case CRITERION_TYPE_APP_CLASS:
      return props.app_class;
    case CRITERION_TYPE_TRANSIENT:
      return (props.transient_for != NULL) ? true_str : false_str;
    default:
      ERROR << "Window property requested for unknown criterion type " << type;
      CHECK(false);
  }
}


bool WindowClassifier::Load(const ConfigNode& conf,
                            vector<ConfigError>* errors) {
  CHECK(errors);
  CHECK(!conf.tokens.empty());
  CHECK_EQ(conf.tokens[0], "window");

  ref_ptr<WindowCriteriaVector> criteria_vector(new WindowCriteriaVector);
  ref_ptr<WindowConfigVector> config_vector(new WindowConfigVector);

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());

    if (node.tokens.empty()) {
      errors->push_back(ConfigError("Node with no tokens in \"window\" block",
                                    node.line_num));
    } else if (node.tokens[0] == "criteria") {
      ref_ptr<WindowCriteria> criteria(new WindowCriteria);
      // If we're unable to load criteria, we'll probably end up matching
      // windows that we don't want to, so we just ignore the entire block.
      if (!LoadWindowCriteria(node, criteria.get(), errors)) return false;
      criteria_vector->push_back(criteria);
    } else if (node.tokens[0] == "config") {
      ref_ptr<WindowConfig> window_config(new WindowConfig);
      // If we're unable to load a config, things may still be salvageable.
      if (!LoadWindowConfig(node, window_config.get(), errors)) continue;
      config_vector->push_back(window_config);
    } else {
      string msg = StringPrintf("Got token \"%s\" with %d parameter(s)",
                                node.tokens[0].c_str(), node.tokens.size() - 1);
      errors->push_back(ConfigError(msg, node.line_num));
    }
  }
  if (config_vector->empty()) {
    errors->push_back(ConfigError("No window configs defined", conf.line_num));
    return false;
  }
  AddConfig(criteria_vector, config_vector);
  return true;
}


// FIXME: remove this?
void WindowClassifier::AddConfig(ref_ptr<WindowCriteriaVector> criteria,
                                 ref_ptr<WindowConfigVector> configs) {
  criteria_configs_.push_back(make_pair(criteria, configs));
}


bool WindowClassifier::ClassifyWindow(
    const WindowProperties& props,
    WindowConfigSet* configs) const {
  CHECK(configs);

  // Get the name of the previously-active config so we can continue using
  // it if possible after reclassification.
  const WindowConfig* prev_config = configs->GetActiveConfig();
  string prev_config_name = prev_config ? prev_config->name : "";
  configs->Clear();

  bool classified = false;
  for (WindowCriteriaConfigs::const_iterator it =
         criteria_configs_.begin(); it != criteria_configs_.end(); ++it) {
    bool got_match = false;
    for (WindowCriteriaVector::const_iterator criteria = it->first->begin();
         criteria != it->first->end(); ++criteria) {
      if ((*criteria)->Matches(props)) {
        got_match = true;
        break;
      }
    }
    // Pairs with empty criteria sets should match everything.
    if (got_match || it->first->empty()) {
      for (WindowConfigVector::const_iterator config = it->second->begin();
           config != it->second->end(); ++config) {
        configs->MergeConfig(*(config->get()));
      }
      classified = true;
    }
  }

  if (classified) configs->SetActiveConfigByName(prev_config_name);

  return classified;
}


bool WindowClassifier::LoadWindowCriteria(const ConfigNode& conf,
                                          WindowCriteria* criteria,
                                          vector<ConfigError>* errors) {
  CHECK(criteria);
  CHECK(errors);
  CHECK_EQ(conf.tokens.size(), 1);
  CHECK_EQ(conf.tokens[0], "criteria");

  if (conf.tokens.size() != 1) {
    errors->push_back(
        ConfigError("Got unexpected arguments to \"criteria\"", conf.line_num));
    return false;
  }

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());

    if (node.tokens.size() != 2) {
      string msg = StringPrintf("Criteria node with %d token(s); expected 2",
                                node.tokens.size());
      errors->push_back(ConfigError(msg, node.line_num));
      return false;
    }
    WindowCriteria::CriterionType type =
        WindowCriteria::StrToCriterionType(node.tokens[0]);
    if (type == WindowCriteria::CRITERION_TYPE_UNKNOWN) {
      string msg = StringPrintf("Criterion with unknown type \"%s\"",
                                node.tokens[0].c_str());
      errors->push_back(ConfigError(msg, node.line_num));
      return false;
    }
    if (!criteria->AddCriterion(type, node.tokens[1])) {
      string msg = StringPrintf("Unable to add criterion of type %d "
                                "with pattern \"%s\"",
                                type, node.tokens[1].c_str());
      errors->push_back(ConfigError(msg, node.line_num));
      return false;
    }
  }
  return true;
}


bool WindowClassifier::LoadWindowConfig(const ConfigNode& conf,
                                        WindowConfig* window_config,
                                        vector<ConfigError>* errors) {
  CHECK(window_config);
  CHECK(errors);
  CHECK(!conf.tokens.empty());
  CHECK_EQ(conf.tokens[0], "config");

  if (conf.tokens.size() != 2) {
    string msg = StringPrintf("Window config node with %d token(s); "
                              "expected 2 (\"config\" and config name)",
                              conf.tokens.size());
    errors->push_back(ConfigError(msg, conf.line_num));
    return false;
  }
  window_config->name = conf.tokens[1];

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());

    if (node.tokens.empty()) {
      errors->push_back(ConfigError(
          "Node with no tokens in window config block", conf.line_num));
      return false;
    }

    if (node.tokens[0] == "width" && node.tokens.size() == 2) {
      if (!ParseDimension(node.tokens[1],
                          &(window_config->width_type),
                          &(window_config->width))) {
        string msg = StringPrintf("Unable to parse width dimensions \"%s\"",
                                  node.tokens[1].size());
        errors->push_back(ConfigError(msg, node.line_num));
        return false;
      }
    } else if (node.tokens[0] == "height" && node.tokens.size() == 2) {
      if (!ParseDimension(node.tokens[1],
                          &(window_config->height_type),
                          &(window_config->height))) {
        string msg = StringPrintf("Unable to parse height dimensions \"%s\"",
                                  node.tokens[1].size());
        errors->push_back(ConfigError(msg, node.line_num));
        return false;
      }
    } else {
      string msg = StringPrintf("Got unknown token \"%s\" with %d parameter(s)",
                                node.tokens[0].c_str(),
                                (node.tokens.size() - 1));
      errors->push_back(ConfigError(msg, node.line_num));
      return false;
    }
  }
  return true;
}


bool WindowClassifier::ParseDimension(const string& str,
                                      WindowConfig::DimensionType* type,
                                      uint* dim) {
  CHECK(type);
  CHECK(dim);

  static pcrecpp::RE num_re("(?i)(\\d+)(u?)");

  string unit;
  if (str == "app") {
    *type = WindowConfig::DIMENSION_APP;
    *dim = 0;
  } else if (str == "*") {
    *type = WindowConfig::DIMENSION_MAX;
    *dim = 0;
  } else if (num_re.FullMatch(str, dim, &unit)) {
    *type = unit.empty() ?
            WindowConfig::DIMENSION_PIXELS :
            WindowConfig::DIMENSION_UNITS;
  } else {
    return false;
  }
  return true;
}

}  // namespace wham
