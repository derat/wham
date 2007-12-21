// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include <cstdarg>
#include <string>
#include <vector>

#include "config-parser.h"
#include "util.h"

using namespace wham;

class ConfigParserTestSuite : public CxxTest::TestSuite {
 public:
  void testTokenizer_GetNextToken_simple() {
    TS_ASSERT(CompareTokens("a b", "a", "b", kTerm, NULL));
    TS_ASSERT(CompareTokens(" a   b  ", "a", "b", kTerm, NULL));
    TS_ASSERT(CompareTokens("a b\n c", "a", "b", kTerm, "c", kTerm, NULL));
  }

  void testTokenizer_GetNextToken_commenting() {
    TS_ASSERT(CompareTokens("# a b c", kTerm, NULL));
    TS_ASSERT(CompareTokens("a # b c", "a", kTerm, NULL));
    TS_ASSERT(CompareTokens("a#b", "a", kTerm, NULL));
    TS_ASSERT(CompareTokens("#a\nb", kTerm, "b", kTerm, NULL));
    TS_ASSERT(CompareTokens("# a\\nb", kTerm, NULL));
    TS_ASSERT(CompareTokens("a # b\nc", "a", kTerm, "c", kTerm, NULL));
  }

  void testTokenizer_GetNextToken_quoting() {
    TS_ASSERT(CompareTokens("\"a b \"", "a b ", kTerm, NULL));
    TS_ASSERT(CompareTokens("a \"b'c\" d", "a", "b'c", "d", kTerm, NULL));
    TS_ASSERT(CompareTokens("a 'b\"c' d", "a", "b\"c", "d", kTerm, NULL));
    TS_ASSERT(CompareTokens(" '' ", "", kTerm, NULL));
    TS_ASSERT(CompareTokens(" \"\" ", "", kTerm, NULL));
    TS_ASSERT(CompareTokens("\"a\"\"b\"", "ab", kTerm, NULL));
    TS_ASSERT(CompareTokens("\"ab\"'cd'", "abcd", kTerm, NULL));
    TS_ASSERT(CompareTokens("\" # \"", " # ", kTerm, NULL));
    TS_ASSERT(CompareTokens("' # '", " # ", kTerm, NULL));
  }

  void testTokenizer_GetNextToken_escaping() {
    TS_ASSERT(CompareTokens("\"a \\\" b\"", "a \" b", kTerm, NULL));
    TS_ASSERT(CompareTokens("'a \\' b'", "a ' b", kTerm, NULL));
    TS_ASSERT(CompareTokens(" \\  ", " ", kTerm, NULL));
    TS_ASSERT(CompareTokens("\\\\a\\\\", "\\a\\", kTerm, NULL));
    TS_ASSERT(CompareTokens("\\a", "\\a", kTerm, NULL));
    TS_ASSERT(CompareTokens("\\.", "\\.", kTerm, NULL));
    TS_ASSERT(CompareTokens(" \\n ", "\n", kTerm, NULL));
    TS_ASSERT(CompareTokens(" \\t ", "\t", kTerm, NULL));
    TS_ASSERT(CompareTokens("a\\nb", "a\nb", kTerm, NULL));
    TS_ASSERT(CompareTokens("a \\\nb", "a", "b", kTerm, NULL));
    TS_ASSERT(CompareTokens("a \\\nn", "a", "n", kTerm, NULL));
    TS_ASSERT(CompareTokens("\\#", "#", kTerm, NULL));
    TS_ASSERT(CompareTokens("\\'abc\\'", "'abc'", kTerm, NULL));
    TS_ASSERT(CompareTokens("\\\"abc\\\"", "\"abc\"", kTerm, NULL));
    TS_ASSERT(CompareTokens("a \\\n b", "a", "b", kTerm, NULL));
  }

  void testConfigParser_parse() {
    ConfigParser::FileTokenizer tokenizer("testdata/config1.cfg");
    ParsedConfig config;
    CHECK(ConfigParser::Parse(&tokenizer, &config));
    config.Dump();
  }

 private:
  static const char* kTerm;

  bool CompareTokens(const string& input, ...) {
    va_list argp;
    va_start(argp, input);
    vector<string> expected_tokens;
    while (const char* token = va_arg(argp, char*)) {
      expected_tokens.push_back(token);
    }
    va_end(argp);

    ConfigParser::StringTokenizer tokenizer(input);
    char token[1024];
    bool term = false;
    bool error = false;
    size_t num_tokens = 0;

    //LOG << "Testing input \"" << input << "\"";
    while (tokenizer.GetNextToken(token, sizeof(token), &term, &error)) {
      if (error) return false;
      CHECK(num_tokens < expected_tokens.size());
      if (term) snprintf(token, sizeof(token), kTerm);
      TS_ASSERT_EQUALS(token, expected_tokens[num_tokens]);
      num_tokens++;
    }
    TS_ASSERT_EQUALS(num_tokens, expected_tokens.size());
    return true;
  }
};

const char* ConfigParserTestSuite::kTerm = "__TERM__";
