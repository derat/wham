// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "window-classifier.h"

using namespace wham;

class WindowClassifierTestSuite : public CxxTest::TestSuite {
 public:
  void testWindowConfigSet_Merge() {
    WindowConfigSet configs;
    TS_ASSERT(configs.configs_.empty());

    // Merge a single config.
    WindowConfig config1("default", 10, 20);
    configs.MergeConfig(config1);
    TS_ASSERT_EQUALS(configs.configs_.size(), 1U);
    WindowConfig stored_config = *(configs.configs_[0].get());
    TS_ASSERT_EQUALS(stored_config.name, config1.name);
    TS_ASSERT_EQUALS(stored_config.width, config1.width);
    TS_ASSERT_EQUALS(stored_config.height, config1.height);

    // When we merge a second config with a new name, it should be
    // appended.
    WindowConfig config2("foo", 30, 40);
    configs.MergeConfig(config2);
    TS_ASSERT_EQUALS(configs.configs_.size(), 2U);
    stored_config = *(configs.configs_[0].get());
    TS_ASSERT_EQUALS(stored_config.name, config1.name);
    TS_ASSERT_EQUALS(stored_config.width, config1.width);
    TS_ASSERT_EQUALS(stored_config.height, config1.height);
    stored_config = *(configs.configs_[1].get());
    TS_ASSERT_EQUALS(stored_config.name, config2.name);
    TS_ASSERT_EQUALS(stored_config.width, config2.width);
    TS_ASSERT_EQUALS(stored_config.height, config2.height);

    // If we merge a config with the same name as the first config, it
    // should replace the first config.
    WindowConfig config3("default", 50, 60);
    configs.MergeConfig(config3);
    TS_ASSERT_EQUALS(configs.configs_.size(), 2U);
    stored_config = *(configs.configs_[0].get());
    TS_ASSERT_EQUALS(stored_config.name, config3.name);
    TS_ASSERT_EQUALS(stored_config.width, config3.width);
    TS_ASSERT_EQUALS(stored_config.height, config3.height);
    stored_config = *(configs.configs_[1].get());
    TS_ASSERT_EQUALS(stored_config.name, config2.name);
    TS_ASSERT_EQUALS(stored_config.width, config2.width);
    TS_ASSERT_EQUALS(stored_config.height, config2.height);

    // Throw in a quick test of the Clear() method.
    configs.Clear();
    TS_ASSERT(configs.configs_.empty());
  }

  void testWindowConfigSet_misc() {
    WindowConfigSet configs;
    TS_ASSERT_EQUALS(configs.NumConfigs(), 0U);

    WindowConfig config("default", 100, 100);
    configs.MergeConfig(config);
    TS_ASSERT_EQUALS(configs.NumConfigs(), 1U);

    WindowConfig config2("foo", 100, 100);
    configs.MergeConfig(config2);
    TS_ASSERT_EQUALS(configs.NumConfigs(), 2U);

    configs.Clear();
    TS_ASSERT_EQUALS(configs.NumConfigs(), 0U);
  }

  void testWindowConfigSet_CycleActiveConfig() {
    WindowConfigSet configs;

    WindowConfig config("first", 100, 100);
    configs.MergeConfig(config);
    WindowConfig config2("second", 100, 100);
    configs.MergeConfig(config2);

    // Initially, the first config should be active.
    const WindowConfig* active = configs.GetActiveConfig();
    CHECK(active != NULL);
    TS_ASSERT_EQUALS(active->name, "first");

    // If we cycle forward, the second config will be active.
    configs.CycleActiveConfig(true);
    active = configs.GetActiveConfig();
    CHECK(active != NULL);
    TS_ASSERT_EQUALS(active->name, "second");

    // Wrap around if we cycle forward again.
    configs.CycleActiveConfig(true);
    active = configs.GetActiveConfig();
    CHECK(active != NULL);
    TS_ASSERT_EQUALS(active->name, "first");

    // We should also wrap when we cycle backward.
    configs.CycleActiveConfig(false);
    active = configs.GetActiveConfig();
    CHECK(active != NULL);
    TS_ASSERT_EQUALS(active->name, "second");

    // One more step backward should bring us to the first config again.
    configs.CycleActiveConfig(false);
    active = configs.GetActiveConfig();
    CHECK(active != NULL);
    TS_ASSERT_EQUALS(active->name, "first");
  }

  void testWindowCriteria_Matches() {
    // Empty criteria should match everything.
    WindowProperties props;
    WindowCriteria crit;
    TS_ASSERT(crit.Matches(props));
    props.window_name = "foo";
    TS_ASSERT(crit.Matches(props));

    // We should get a match if the criteria matches any or all of the
    // properties.
    props.window_name = "window";
    props.icon_name = "icon";
    props.command = "command";

    // Test that substring matching works individually for the different
    // criterion types.
    crit.Reset();
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_WINDOW_NAME, "win");
    TS_ASSERT(crit.Matches(props));

    crit.Reset();
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_ICON_NAME, "icon");
    TS_ASSERT(crit.Matches(props));

    crit.Reset();
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_COMMAND, "mand");
    TS_ASSERT(crit.Matches(props));

    // It should also work when we define multiple criteria.
    crit.Reset();
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_WINDOW_NAME, "window");
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_COMMAND, "command");
    TS_ASSERT(crit.Matches(props));

    // But not if one of the criteria doesn't match.
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_ICON_NAME, "blah");
    TS_ASSERT(!crit.Matches(props));

    // We also shouldn't get a match if we supply a superstring rather than
    // a substring.
    crit.Reset();
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_WINDOW_NAME, "window123");
    TS_ASSERT(!crit.Matches(props));

    // Test regular expression criteria.
    crit.Reset();
    crit.AddCriterion(WindowCriteria::CRITERION_TYPE_WINDOW_NAME, "/wi.*o/");
    TS_ASSERT(crit.Matches(props));
  }

  void testWindowClassifier_ClassifyWindow() {
    WindowClassifier classifier;

    // Add a config matching "rxvt".
    ref_ptr<WindowCriteria> crit(new WindowCriteria);
    crit->AddCriterion(WindowCriteria::CRITERION_TYPE_APP_NAME, "rxvt");
    ref_ptr<WindowCriteriaVector> criteria(new WindowCriteriaVector);
    criteria->push_back(crit);

    ref_ptr<WindowConfigVector> configs(new WindowConfigVector);
    configs->push_back(
        ref_ptr<WindowConfig>(new WindowConfig("default", 100, 200)));

    classifier.AddConfig(criteria, configs);

    // Add a second config matching "xterm".
    crit.reset(new WindowCriteria);
    crit->AddCriterion(WindowCriteria::CRITERION_TYPE_APP_NAME, "xterm");
    criteria.reset(new WindowCriteriaVector);
    criteria->push_back(crit);

    configs.reset(new WindowConfigVector);
    configs->push_back(
        ref_ptr<WindowConfig>(new WindowConfig("default", 300, 400)));

    classifier.AddConfig(criteria, configs);

    // Add a third config matching everything.
    criteria.reset(new WindowCriteriaVector);
    configs.reset(new WindowConfigVector);
    configs->push_back(
        ref_ptr<WindowConfig>(new WindowConfig("foo", 500, 600)));
    classifier.AddConfig(criteria, configs);

    WindowProperties props;
    props.app_name = "rxvt";
    WindowConfigSet matched_configs;
    TS_ASSERT(classifier.ClassifyWindow(props, &matched_configs));
    TS_ASSERT_EQUALS(matched_configs.NumConfigs(), 2U);

    props.app_name = "xterm";
    matched_configs.Clear();
    TS_ASSERT(classifier.ClassifyWindow(props, &matched_configs));
    TS_ASSERT_EQUALS(matched_configs.NumConfigs(), 2U);

    props.app_name = "blah";
    matched_configs.Clear();
    TS_ASSERT(classifier.ClassifyWindow(props, &matched_configs));
    TS_ASSERT_EQUALS(matched_configs.NumConfigs(), 1U);
  }
};
