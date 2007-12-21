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
  WindowProperties(const string& window_name,
                   const string& icon_name,
                   const string& command)
      : window_name(window_name),
        icon_name(icon_name),
        command(command) {
  }

  string window_name;  // from XGetWMName()
  string icon_name;    // from XGetWMIconName()
  string command;      // from XGetCommand()
};


// Information about how a given window should be displayed.
struct WindowConfig {
  WindowConfig(const string& name,
               bool dimensions_are_in_pixels,
               int desired_width,
               int desired_height)
      : name(name),
        dimensions_are_in_pixels(dimensions_are_in_pixels),
        desired_width(desired_width),
        desired_height(desired_height) {
  }

  // Merge another config into this one.
  void Merge(const WindowConfig& config);

  // This config's name.
  string name;

  // Are window dimensions given in pixels, rather than in terms of the
  // size increments supplied by the window?
  bool dimensions_are_in_pixels;

  // Desired dimensions of the window.  (These are also the maximum
  // dimensions of the window.)  -1 indicates that the window should be
  // maximized.
  int desired_width;
  int desired_height;
};


// A set of WindowConfig objects.
class WindowConfigSet {
 public:
  // Remove all configs from the set.
  void Clear();

  // Merge a window config into the set, either adding it or updating an
  // existing config.
  void MergeConfig(const WindowConfig& config);

 private:
  vector<WindowConfig> configs_;
};


// Stores a set of window criteria.
class WindowCriteria {
 public:
  WindowCriteria() {}

  enum CriterionType {
    CRITERION_TYPE_WINDOW_NAME,
    CRITERION_TYPE_ICON_NAME,
    CRITERION_TYPE_COMMAND,
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


// Stores window criteria and associated WindowConfig objects.
class WindowClassifier {
 public:
  void AddConfig(vector<ref_ptr<WindowCriteria> >& criteria,
                 const WindowConfig& config);

  // Classify a WindowProperties object into a WindowConfigSet.
  bool ClassifyWindow(
      const WindowProperties& props,
      WindowConfigSet* config_set) const;

 private:
  typedef vector<ref_ptr<WindowCriteria> > WindowCriteriaSet;
  typedef vector<pair<WindowCriteriaSet, WindowConfig> >
      WindowCriteriaConfigs;

  WindowCriteriaConfigs criteria_configs_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowClassifier);
};

}  // namespace wham

#endif
