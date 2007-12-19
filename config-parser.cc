// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config-parser.h"

#include "util.h"

using namespace std;

namespace wham {

const int ConfigParser::kMaxTokenLength = 1024;


bool ConfigParser::GetTokens(FILE* file, vector<string>* tokens) {
  CHECK(file != NULL);
  CHECK(tokens != NULL);

  char token[kMaxTokenLength + 1];
  int token_length = 0;
  int line = 1;

  bool in_single_quote = false;
  bool in_double_quote = false;
  bool in_escape = false;
  bool in_comment = false;

  while (true) {
    int ch = fgetc(file);
    if ((isspace(ch) || ch == EOF) &&
        !in_single_quote && !in_double_quote && !in_escape) {
      if (token_length > 0) {
        CHECK(token_length < sizeof(token));
        token[token_length] = '\0';
        tokens->push_back(string(token));
        token_length = 0;
      }
      if (ch == '\n') {
        line++;
        if (in_comment) in_comment = false;
      }
      if (ch == '\n' || ch == EOF) tokens->push_back("");
      if (ch == EOF) break;
    } else if (in_comment) {
      // Ignore the character unless it's a newline.
      if (ch == '\n') {
        line++;
        if (in_comment) in_comment = false;
      }
    } else if (ch == '\'' && !in_escape && !in_double_quote) {
      in_single_quote = !in_single_quote;
    } else if (ch == '"' && !in_escape && !in_single_quote) {
      in_double_quote = !in_double_quote;;
    } else if (ch == '\\' && !in_escape) {
      in_escape = true;
    } else if (ch == '#' &&
               !in_escape && !in_single_quote && !in_double_quote) {
      in_comment = true;
    } else {
      if (ch == '\n') {
        // Treat escaped newlines as whitespace, not as parts of tokens.
        line++;
      } else {
        if (token_length >= kMaxTokenLength) {
          LOG << "Unable to parse token exceeding " << kMaxTokenLength
              << " characters on line " << line;
          return false;
        }
        CHECK(token_length < sizeof(token));
        token[token_length] = ch;
        token_length++;
      }
      if (in_escape) in_escape = false;
    }
  }

  return true;
}

}  // namespace wham
