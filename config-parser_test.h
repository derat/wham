// Copyright 2007 Daniel Erat <dan@erat.org>
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
    TS_ASSERT(CompareTokens("a b", "a", "b", kNewline, NULL));
    TS_ASSERT(CompareTokens(" a   b  ", "a", "b", kNewline, NULL));
    TS_ASSERT(CompareTokens("a b\n c",
                            "a", "b", kNewline, "c", kNewline, NULL));
    TS_ASSERT(CompareTokens("   ", kNewline, NULL));
    TS_ASSERT(CompareTokens("\n a \n ",
                            kNewline, "a", kNewline, kNewline, NULL));
  }

  void testTokenizer_GetNextToken_special() {
    TS_ASSERT(CompareTokens("a . b", "a", kPeriod, "b", kNewline, NULL));
    TS_ASSERT(CompareTokens("a { b", "a", kLeftBrace, "b", kNewline, NULL));
    TS_ASSERT(CompareTokens("a } b", "a", kRightBrace, "b", kNewline, NULL));
  }

  void testTokenizer_GetNextToken_commenting() {
    TS_ASSERT(CompareTokens("# a b c", kNewline, NULL));
    TS_ASSERT(CompareTokens("a # b c", "a", kNewline, NULL));
    TS_ASSERT(CompareTokens("a#b", "a", kNewline, NULL));
    TS_ASSERT(CompareTokens("#a\nb", kNewline, "b", kNewline, NULL));
    TS_ASSERT(CompareTokens("# a\\nb", kNewline, NULL));
    TS_ASSERT(CompareTokens("a # b\nc", "a", kNewline, "c", kNewline, NULL));
  }

  void testTokenizer_GetNextToken_quoting() {
    TS_ASSERT(CompareTokens("\"a b \"", "a b ", kNewline, NULL));
    TS_ASSERT(CompareTokens("a \"b'c\" d", "a", "b'c", "d", kNewline, NULL));
    TS_ASSERT(CompareTokens("a 'b\"c' d", "a", "b\"c", "d", kNewline, NULL));
    TS_ASSERT(CompareTokens(" '' ", "", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \"\" ", "", kNewline, NULL));
    TS_ASSERT(CompareTokens("\"a\"\"b\"", "ab", kNewline, NULL));
    TS_ASSERT(CompareTokens("\"ab\"'cd'", "abcd", kNewline, NULL));
    TS_ASSERT(CompareTokens("\" # \"", " # ", kNewline, NULL));
    TS_ASSERT(CompareTokens("' # '", " # ", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \"{\" ", "{", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \'}\' ", "}", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \".\" ", ".", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \"\". ", ".", kNewline, NULL));
  }

  void testTokenizer_GetNextToken_escaping() {
    TS_ASSERT(CompareTokens("\"a \\\" b\"", "a \" b", kNewline, NULL));
    TS_ASSERT(CompareTokens("'a \\' b'", "a ' b", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \\  ", " ", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\\\a\\\\", "\\a\\", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\a", "a", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \\n ", "\n", kNewline, NULL));
    TS_ASSERT(CompareTokens(" \\t ", "\t", kNewline, NULL));
    TS_ASSERT(CompareTokens("a\\nb", "a\nb", kNewline, NULL));
    TS_ASSERT(CompareTokens("a \\\nb", "a", "b", kNewline, NULL));
    TS_ASSERT(CompareTokens("a \\\nn", "a", "n", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\#", "#", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\'abc\\'", "'abc'", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\\"abc\\\"", "\"abc\"", kNewline, NULL));
    TS_ASSERT(CompareTokens("a \\\n b", "a", "b", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\.", ".", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\{", "{", kNewline, NULL));
    TS_ASSERT(CompareTokens("\\}", "}", kNewline, NULL));
  }

  void testConfigParser_parse() {
    ConfigParser::FileTokenizer tokenizer("testdata/config-parser_test.cfg");
    ParsedConfig config;
    CHECK(ConfigParser::Parse(&tokenizer, &config));
    string parsed = config.Dump();
    //LOG << parsed;
    string golden;
    CHECK(ReadFileToString("testdata/config-parser_test.golden", &golden));
    TS_ASSERT_EQUALS(parsed, golden);
  }

 private:
  static const char* kLeftBrace;
  static const char* kRightBrace;
  static const char* kPeriod;
  static const char* kNewline;

  bool CompareTokens(const string& input, ...) {
    va_list argp;
    va_start(argp, input);
    vector<string> expected_tokens;
    while (const char* token = va_arg(argp, char*)) {
      expected_tokens.push_back(token);
    }
    va_end(argp);

    ConfigParser::StringTokenizer tokenizer(input);
    string token;
    ConfigParser::TokenType token_type = ConfigParser::NUM_TOKEN_TYPES;
    bool error = false;
    size_t num_tokens = 0;

    //LOG << "Testing input \"" << input << "\"";
    while (tokenizer.GetNextToken(&token, &token_type, &error)) {
      if (error) return false;
      CHECK(num_tokens < expected_tokens.size());
      switch (token_type) {
        case ConfigParser::TOKEN_LEFT_BRACE:  token = kLeftBrace; break;
        case ConfigParser::TOKEN_RIGHT_BRACE: token = kRightBrace; break;
        case ConfigParser::TOKEN_PERIOD:      token = kPeriod; break;
        case ConfigParser::TOKEN_NEWLINE:     token = kNewline; break;
        case ConfigParser::TOKEN_LITERAL:     break;
        default:
          ERROR << "Got unknown token type " << token_type;
          CHECK(false);
      }
      TS_ASSERT_EQUALS(token, expected_tokens[num_tokens]);
      num_tokens++;
    }
    TS_ASSERT_EQUALS(num_tokens, expected_tokens.size());
    return true;
  }

  bool ReadFileToString(const string& filename, string* out) {
    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL) return false;
    char buf[1024];
    while (fgets(buf, sizeof(buf), file)) out->append(buf);
    fclose(file);
    return true;
  }
};

const char* ConfigParserTestSuite::kLeftBrace  = "__LEFT_BRACE__";
const char* ConfigParserTestSuite::kRightBrace = "__RIGHT_BRACE__";
const char* ConfigParserTestSuite::kPeriod     = "__PERIOD__";
const char* ConfigParserTestSuite::kNewline    = "__NEWLINE__";
