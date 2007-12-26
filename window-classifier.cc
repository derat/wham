// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-classifier.h"

namespace wham {

void WindowConfig::Merge(const WindowConfig& config) {
  name = config.name;
  width = config.width;
  height = config.height;
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


void WindowConfigSet::Merge(const WindowConfigSet& configs) {
  for (WindowConfigVector::const_iterator it = configs.configs_.begin();
       it != configs.configs_.end(); ++it) {
    MergeConfig(*(it->get()));
  }
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
      ERR << "Window property requested for unknown criterion type " << type;
      CHECK(false);
  }
}


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

}  // namespace wham
