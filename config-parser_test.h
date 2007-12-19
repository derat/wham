// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include <string>
#include <vector>

#include "config-parser.h"
#include "util.h"

using namespace wham;

class ConfigParserTestSuite : public CxxTest::TestSuite {
 public:
  void testConfigParser_GetTokens() {
    ConfigParser parser;
    vector<string> tokens;
    FILE* file = fopen("testdata/config1.cfg", "r");
    TS_ASSERT(parser.GetTokens(file, &tokens));
    fclose(file);

    for (vector<string>::const_iterator it = tokens.begin();
         it != tokens.end(); ++it) {
      LOG << *it;
    }
  }
};
