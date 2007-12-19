// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__

#include <cstdio>
#include <string>
#include <vector>

using namespace std;

class ConfigParserTestSuite;

namespace wham {

class ConfigParser {
 private:
  friend class ::ConfigParserTestSuite;

  static const int kMaxTokenLength;

  bool GetTokens(FILE* file, vector<string>* tokens);
};

}  // namespace wham

#endif
