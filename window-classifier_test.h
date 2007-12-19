// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "window-classifier.h"

using namespace wham;

class WindowClassifierTestSuite : public CxxTest::TestSuite {
 public:
  void testWindowCriteria() {
    // Empty criteria should match everything.
    WindowProperties props("", "", "");
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
};
