// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __WINDOW_CLASSIFIER_H__
#define __WINDOW_CLASSIFIER_H__

#include <pcrecpp.h>
#include <string>
#include <vector>

#include "config-parser.h"
#include "util.h"

using namespace std;

class WindowClassifierTestSuite;

namespace wham {

class WindowProperties;

// Information about how a given window should be displayed.
struct WindowConfig {
  WindowConfig()
      : name("default"),
        width_type(DIMENSION_APP),
        height_type(DIMENSION_APP),
        width(0),
        height(0) {
  }
  WindowConfig(const string& name,
               int width,
               int height)
      : name(name),
        width_type(DIMENSION_PIXELS),
        height_type(DIMENSION_PIXELS),
        width(width),
        height(height) {
  }

  // Merge another config into this one.
  void Merge(const WindowConfig& config);

  string DebugString() const;

  // This config's name.
  string name;

  // A bit of extra information about the window dimensions stored in this
  // config.
  enum DimensionType {
    DIMENSION_PIXELS, // absolute pixels
    DIMENSION_UNITS,  // app-supplied units
    DIMENSION_APP,    // just use the size supplied by the app
    DIMENSION_MAX,    // maximize the window
  };

  static string DimensionTypeToStr(DimensionType type) {
    if (type == DIMENSION_PIXELS) return "pixels";
    if (type == DIMENSION_UNITS)  return "units";
    if (type == DIMENSION_APP)    return "app";
    if (type == DIMENSION_MAX)    return "max";
    return "unknown";
  }

  DimensionType width_type;
  DimensionType height_type;

  // Desired dimensions of the window.
  uint width;
  uint height;
};

typedef vector<ref_ptr<WindowConfig> > WindowConfigVector;


// A set of WindowConfigs.
class WindowConfigSet {
 public:
  WindowConfigSet()
      : configs_(),
        active_(0) {
  }

  // Remove all configs from the set.
  void Clear() {
    configs_.clear();
    active_ = 0;
  }

  // Merge a window config into the set, either adding it or updating an
  // existing config.
  void MergeConfig(const WindowConfig& config);

  // Get the currently-active config, or NULL if no configs are present.
  const WindowConfig* GetActiveConfig() const {
    if (configs_.empty()) return NULL;
    CHECK(active_ < configs_.size());
    return configs_[active_].get();
  }

  // Cycle the active config either forward or backward, wrapping if
  // necessary.
  void CycleActiveConfig(bool forward) {
    if (configs_.size() <= 1) return;
    active_ = (active_ + configs_.size() + (forward ? 1 : -1)) %
        configs_.size();
  }

  // Get the number of configs in this set.
  size_t NumConfigs() const { return configs_.size(); }

 private:
  friend class ::WindowClassifierTestSuite;

  // Configs contained in the set
  WindowConfigVector configs_;

  // Index into 'configs_' of the currently-active config
  size_t active_;

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
    CRITERION_TYPE_UNKNOWN,
  };

  static CriterionType StrToCriterionType(const string& str) {
    if (str == "window_name") return CRITERION_TYPE_WINDOW_NAME;
    if (str == "icon_name")   return CRITERION_TYPE_ICON_NAME;
    if (str == "command")     return CRITERION_TYPE_COMMAND;
    if (str == "app_name")    return CRITERION_TYPE_APP_NAME;
    if (str == "app_class")   return CRITERION_TYPE_APP_CLASS;
    return CRITERION_TYPE_UNKNOWN;
  }

  // Add a criterion of a particular type.
  // 'pattern' will be interpreted according to its contents:
  // - "/a.*b/": regular expression (partial match)
  // - anything else: substring
  bool AddCriterion(CriterionType type, const string& pattern);

  // Clear all criteria from this set.
  void Reset();

  // Does 'props' satisfy all of these criteria?
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

  // Get the current classifier.
  static WindowClassifier* Get() {
    CHECK(singleton_.get());
    return singleton_.get();
  }

  // Install a new classifier.
  static void Swap(ref_ptr<WindowClassifier> new_classifier) {
    singleton_.swap(new_classifier);
  }

  // Load a "window" node from a parsed config.
  bool Load(const ConfigNode& conf);

  void AddConfig(ref_ptr<WindowCriteriaVector> criteria,
                 ref_ptr<WindowConfigVector> configs);

  // Classify a WindowProperties object into list of configs.
  bool ClassifyWindow(const WindowProperties& props,
                      WindowConfigSet* configs) const;

 private:
  friend class ::WindowClassifierTestSuite;

  static bool LoadWindowCriteria(
      const ConfigNode& conf, WindowCriteria* criteria);
  static bool LoadWindowConfig(
      const ConfigNode& conf, WindowConfig* window_config);

  static bool ParseDimensions(const string& str,
                              WindowConfig::DimensionType* type,
                              uint* dim);

  // FIXME: Think about this some more.  Is there a good reason to allow
  // multiple criteria sets here, or does it just add useless flexibility?
  typedef vector<pair<ref_ptr<WindowCriteriaVector>,
                      ref_ptr<WindowConfigVector> > >
      WindowCriteriaConfigs;
  WindowCriteriaConfigs criteria_configs_;

  // Singleton object.
  static ref_ptr<WindowClassifier> singleton_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowClassifier);
};

}  // namespace wham

#endif
