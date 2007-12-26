// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_CLASSIFIER_H__
#define __WINDOW_CLASSIFIER_H__

#include <pcrecpp.h>
#include <string>
#include <vector>

#include "util.h"

using namespace std;

class WindowClassifierTestSuite;

namespace wham {

// X-supplied information about a window.  This consists largely of window
// manager hints (window and icon name, etc.) and is used to
// classify the window.
struct WindowProperties {
  WindowProperties() {}

  string window_name;  // from XFetchName()
  string icon_name;    // from XGetIconName()
  string command;      // from XGetCommand()
  string app_name;     // from XGetClassHint()
  string app_class;    // from XGetClassHint()
};


// Information about how a given window should be displayed.
struct WindowConfig {
  WindowConfig(const string& name,
               int width,
               int height)
      : name(name),
        width(width),
        height(height) {
  }

  // Merge another config into this one.
  void Merge(const WindowConfig& config);

  // This config's name.
  string name;

  // Desired dimensions of the window.  -1 indicates that the window should
  // be maximized.
  int width;
  int height;
};

typedef vector<ref_ptr<WindowConfig> > WindowConfigVector;


// A set of WindowConfigs.
class WindowConfigSet {
 public:
  WindowConfigSet() {}

  // Remove all configs from the set.
  void Clear() {
    configs_.clear();
  }

  // Merge a window config into the set, either adding it or updating an
  // existing config.
  void MergeConfig(const WindowConfig& config);

  // Merge all configs from another set into this one.
  void Merge(const WindowConfigSet& configs);

  const WindowConfig* GetDefaultConfig() const {
    if (configs_.empty()) return NULL;
    return configs_[0].get();
  }

  int NumConfigs() const { return configs_.size(); }

 private:
  friend class ::WindowClassifierTestSuite;
  WindowConfigVector configs_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowConfigSet);
};


// Stores a set of window criteria.
class WindowCriteria {
 public:
  WindowCriteria() {}

  enum CriterionType {
    CRITERION_TYPE_WINDOW_NAME,
    CRITERION_TYPE_ICON_NAME,
    CRITERION_TYPE_COMMAND,
    CRITERION_TYPE_APP_NAME,
    CRITERION_TYPE_APP_CLASS,
    NUM_CRITERION_TYPES
  };

  // Add a criterion of a particular type.
  // 'pattern' will be interpreted according to its contents:
  // - "/a.*b/": regular expression (partial match)
  // - anything else: substring
  bool AddCriterion(CriterionType type, const string& pattern);

  // Clear all criteria from this set.
  void Reset();

  // Returns true if the passed-in WindowProperties object satisfies this
  // criteria and false otherwise.
  bool Matches(const WindowProperties& props) const;

 private:
  // Get the string corresponding to a criterion type from a set of window
  // properties.
  static const string& GetPropertyForCriterionType(
      const WindowProperties& props, CriterionType type);

  typedef vector<pair<CriterionType, ref_ptr<pcrecpp::RE> > > RegexpCriteria;
  RegexpCriteria regexp_criteria_;

  typedef vector<pair<CriterionType, string> > SubstringCriteria;
  SubstringCriteria substr_criteria_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowCriteria);
};

typedef vector<ref_ptr<WindowCriteria> > WindowCriteriaVector;


// Stores window criteria and associated WindowConfig objects.
class WindowClassifier {
 public:
  WindowClassifier() {}

  void AddConfig(ref_ptr<WindowCriteriaVector> criteria,
                 ref_ptr<WindowConfigVector> configs);

  // Classify a WindowProperties object into list of configs.
  bool ClassifyWindow(const WindowProperties& props,
                      WindowConfigSet* configs) const;

 private:

  typedef vector<pair<ref_ptr<WindowCriteriaVector>,
                      ref_ptr<WindowConfigVector> > >
      WindowCriteriaConfigs;

  WindowCriteriaConfigs criteria_configs_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowClassifier);
};

}  // namespace wham

#endif
