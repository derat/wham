// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-classifier.h"

namespace wham {

void WindowConfig::Merge(const WindowConfig& config) {
  // FIXME: write this
}


void WindowConfigSet::Clear() {
  configs_.clear();
}


void WindowConfigSet::MergeConfig(const WindowConfig& config) {
  for (vector<WindowConfig>::iterator it = configs_.begin();
       it != configs_.end(); ++it) {
    if (config.name == it->name) {
      it->Merge(config);
      return;
    }
  }
  configs_.push_back(config);
}


bool WindowCriteria::AddCriterion(
    CriterionType type,
    const string& pattern) {
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
  for (RegexpCriteria::const_iterator it = regexp_criteria_.begin();
       it != regexp_criteria_.end(); ++it) {
    CriterionType type = it->first;
    ref_ptr<pcrecpp::RE> re = it->second;
    if (!re->PartialMatch(GetPropertyForCriterionType(props, type))) {
      return false;
    }
  }

  for (SubstringCriteria::const_iterator it = substr_criteria_.begin();
       it != substr_criteria_.end(); ++it) {
    CriterionType type = it->first;
    const string& pattern = it->second;
    if (GetPropertyForCriterionType(props, type).find(pattern) ==
        string::npos) {
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
    default:
      ERR << "Window property requested for unknown criterion type " << type;
      CHECK(false);
  }
}


bool WindowClassifier::ClassifyWindow(
    const WindowProperties& props,
    WindowConfigSet* config_set) const {
  CHECK(config_set);
  config_set->Clear();

  bool got_match = false;
  for (WindowCriteriaConfigs::const_iterator it =
         criteria_configs_.begin(); it != criteria_configs_.end(); ++it) {
    for (WindowCriteriaSet::const_iterator criteria = it->first.begin();
         criteria != it->first.end(); ++criteria) {
      if ((*criteria)->Matches(props)) {
        config_set->MergeConfig(it->second);
        got_match = true;
        break;
      }
    }
  }

  return got_match;
}

}  // namespace wham
