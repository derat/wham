// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-classifier.h"

#include <sstream>

namespace wham {

ref_ptr<WindowClassifier> WindowClassifier::singleton_(new WindowClassifier);


string WindowProperties::DebugString() const {
  ostringstream out;
  out << "window_name=\"" << window_name << "\"\n"
      << "icon_name=\"" << icon_name << "\"\n"
      << "command=\"" << command << "\"\n"
      << "app_name=\"" << app_name << "\"\n"
      << "app_class=\"" << app_class << "\"\n"
      << "x=" << x << "\n"
      << "y=" << y << "\n"
      << "width=" << width << "\n"
      << "height=" << height << "\n"
      << "min_width=" << min_width << "\n"
      << "min_height=" << min_height << "\n"
      << "max_width=" << max_width << "\n"
      << "max_height=" << max_height << "\n"
      << "width_inc=" << width_inc << "\n"
      << "height_inc=" << height_inc << "\n"
      << "min_aspect=" << min_aspect << "\n"
      << "max_aspect=" << max_aspect << "\n"
      << "base_width=" << base_width << "\n"
      << "base_height=" << base_height << "\n";
  return out.str();
}


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
    default:
      ERROR << "Window property requested for unknown criterion type " << type;
      CHECK(false);
  }
}


bool WindowClassifier::Load(const ConfigNode& conf) {
  CHECK(!conf.tokens.empty());
  CHECK(conf.tokens[0] == "window");

  ref_ptr<WindowCriteriaVector> criteria_vector(new WindowCriteriaVector);
  ref_ptr<WindowConfigVector> config_vector(new WindowConfigVector);

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());

    if (node.tokens.empty()) {
      // TODO: Just make this a warning?
      ERROR << "Node with no tokens in \"window\" block";
      return false;
    }
    if (node.tokens[0] == "criteria") {
      ref_ptr<WindowCriteria> criteria(new WindowCriteria);
      if (!LoadWindowCriteria(node, criteria.get())) return false;
      criteria_vector->push_back(criteria);
    } else if (node.tokens[0] == "config") {
      ref_ptr<WindowConfig> window_config(new WindowConfig);
      if (!LoadWindowConfig(node, window_config.get())) return false;
      config_vector->push_back(window_config);
    } else {
      ERROR << "Got token \"" << node.tokens[0] << "\" with "
            << (node.tokens.size() - 1) << " parameter(s)";
      return false;
    }
  }
  if (config_vector->empty()) {
    ERROR << "No window configs defined";
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

  return classified;
}


bool WindowClassifier::LoadWindowCriteria(const ConfigNode& conf,
                                          WindowCriteria* criteria) {
  CHECK(criteria);
  CHECK(!conf.tokens.empty());
  CHECK(conf.tokens[0] == "criteria");

  if (conf.tokens.size() != 1) {
    // FIXME: make this a warning?
    ERROR << "criteria node has " << conf.tokens.size() << " tokens";
    return false;
  }

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());

    if (node.tokens.size() != 2) {
      ERROR << "criteria node with " << node.tokens.size()
            << " token(s); expected 2";
      return false;
    }
    WindowCriteria::CriterionType type =
        WindowCriteria::StrToCriterionType(node.tokens[0]);
    if (type == WindowCriteria::CRITERION_TYPE_UNKNOWN) {
      ERROR << "Criterion with unknown type \"" << node.tokens[0] << "\"";
      return false;
    }
    if (!criteria->AddCriterion(type, node.tokens[1])) {
      ERROR << "Unable to add criterion of type " << type
            << " with pattern \"" << node.tokens[1] << "\"";
      return false;
    }
  }
  return true;
}


bool WindowClassifier::LoadWindowConfig(const ConfigNode& conf,
                                        WindowConfig* window_config) {
  CHECK(window_config);
  CHECK(!conf.tokens.empty());
  CHECK(conf.tokens[0] == "config");

  if (conf.tokens.size() != 2) {
    ERROR << "window config node has " << conf.tokens.size()
          << " token(s); expected 2";
    return false;
  }
  window_config->name = conf.tokens[1];

  for (vector<ref_ptr<ConfigNode> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ConfigNode& node = *(it->get());

    if (node.tokens.empty()) {
      // TODO: Just make this a warning?
      ERROR << "Node with no tokens in window config block";
      return false;
    }

    if (node.tokens[0] == "width" && node.tokens.size() == 2) {
      if (!ParseDimensions(node.tokens[1],
                           &(window_config->width_type),
                           &(window_config->width))) {
        return false;
      }
    } else if (node.tokens[0] == "height" && node.tokens.size() == 2) {
      if (!ParseDimensions(node.tokens[1],
                           &(window_config->height_type),
                           &(window_config->height))) {
        return false;
      }
    } else {
      ERROR << "Got token " << node.tokens[0] << " with "
            << (node.tokens.size() - 1) << " parameter(s)";
      return false;
    }
  }
  return true;
}


bool WindowClassifier::ParseDimensions(const string& str,
                                       WindowConfig::DimensionType* type,
                                       uint* dim) {
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
    ERROR << "Unable to parse dimensions \"" << str << "\"";
    return false;
  }
  return true;
}

}  // namespace wham
