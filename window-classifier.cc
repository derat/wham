// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "window-classifier.h"

namespace wham {

ref_ptr<WindowClassifier> WindowClassifier::singleton_(new WindowClassifier);

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


bool WindowClassifier::Load(const ParsedConfig::Node& conf) {
  for (vector<ref_ptr<ParsedConfig::Node> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ParsedConfig::Node& node = *(it->get());
    if (node.tokens.empty()) {
      // TODO: Just make this a warning?
      ERROR << "Node with no tokens in \"window_configs\" block";
      return false;
    }
    if (node.tokens[0] == "window" && node.tokens.size() == 1) {
      if (!LoadWindow(node)) return false;
    } else {
      ERROR << "Got token " << node.tokens[0] << " with "
            << (node.tokens.size() - 1) << " parameter(s)";
      return false;
    }
  }
  return true;
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


bool WindowClassifier::LoadWindow(const ParsedConfig::Node& conf) {
  CHECK(conf.tokens.size() == 1U && conf.tokens[0] == "window");

  ref_ptr<WindowCriteriaVector> criteria_vector;

  for (vector<ref_ptr<ParsedConfig::Node> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ParsedConfig::Node& node = *(it->get());

    if (node.tokens.empty()) {
      // TODO: Just make this a warning?
      ERROR << "Node with no tokens in \"window\" block";
      return false;
    }
    if (node.tokens[0] == "criteria") {
      ref_ptr<WindowCriteria> criteria(new WindowCriteria);
      if (!LoadCriteria(node, criteria.get())) return false;
      criteria_vector->push_back(criteria);
    } else if (node.tokens[0] == "config") {
    } else {
      ERROR << "Got token \"" << node.tokens[0] << "\" with "
            << (node.tokens.size() - 1) << " parameter(s)";
      return false;
    }
  }
  return true;
}


bool WindowClassifier::LoadCriteria(const ParsedConfig::Node& conf,
                                    WindowCriteria* criteria) {
  CHECK(criteria);
  CHECK(conf.tokens.size() == 1U && conf.tokens[0] == "criteria");

  for (vector<ref_ptr<ParsedConfig::Node> >::const_iterator it =
         conf.children.begin(); it != conf.children.end(); ++it) {
    const ParsedConfig::Node& node = *(it->get());

    if (node.tokens.size() != 2U) {
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

}  // namespace wham
