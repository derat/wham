// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config-parser.h"

#include <iostream>
#include <sstream>
#include <stack>

#include "util.h"

using namespace std;

namespace wham {

string ParsedConfig::Node::Dump(int level) {
  ostringstream out;
  for (int i = 0; i < level; ++i) out << "  ";
  for (vector<string>::const_iterator token = tokens.begin();
       token != tokens.end(); ++token) {
    if (token != tokens.begin()) out << " ";
    // TODO: should be escaping backslashes and doublequotes here
    out << "\"" << *token << "\"";
  }
  out << "\n";
  for (vector<ref_ptr<Node> >::const_iterator child = children.begin();
       child != children.end(); ++child) {
    out << (*child)->Dump(level + 1);
  }
  return out.str();
}


bool ConfigParser::ParseFromFile(const string& filename,
                                 ParsedConfig* config,
                                 vector<string>* errors) {
  CHECK(config);
  FileTokenizer tokenizer(filename);
  return Parse(&tokenizer, config);
}


ConfigParser::Tokenizer::Tokenizer()
    : done_(false),
      line_num_(1),
      ungetted_char_(0),
      have_ungetted_char_(false) {
}


bool ConfigParser::Tokenizer::GetNextToken(
    string* token, TokenType* token_type, bool* error) {
  CHECK(token);
  CHECK(token_type);
  CHECK(error);

  token->clear();
  *error = false;
  if (done_) return false;

  bool in_single_quote = false;
  bool in_double_quote = false;
  bool in_escape = false;
  bool in_comment = false;
  bool in_token = false;
  bool bare_token = true;
  bool bare_last_char = true;

  int quote_start_line = 0;
  int last_ch = 0;
  int ch = 0;

  while (true) {
    last_ch = ch;
    ch = GetChar();

    // First, handle unterminated quoted strings.
    if ((ch == '\n' || ch == EOF) && (in_single_quote || in_double_quote)) {
      ERROR << "Unclosed quoted string started on line " << quote_start_line;
      *error = true;
      return false;
    }

    // If we're in a comment, ignore any characters devoid of meaning
    // within comments.
    if (in_comment && ch != '\n' && ch != EOF) {
      continue;
    }

    // If we saw a whitespace character or EOF and we're not in a quoted
    // string or escape, then we should emit the in-progress token (if
    // there is one).
    if ((isspace(ch) || ch == EOF) &&
        !in_single_quote && !in_double_quote && !in_escape) {
      if (in_token) {
        if (bare_token) {
          *token_type = GetTokenType(*token);
        } else {
          *token_type = TOKEN_LITERAL;
        }
        // These generate their own tokens, so we'll save them for later.
        if ((ch == '\n' && !in_escape) || ch == EOF) UngetChar(ch);
        bare_token = true;
        return true;
      }

      // Ignore non-significant whitespace that didn't signal the end of a
      // token.
      if (ch != '\n' && ch != EOF) continue;
    }

    // If we got a terminating character but didn't return a token in the
    // previous block, then we're seeing the terminator for the second
    // time, so we should return it as a token.
    if (ch == '\n' || ch == EOF) {
      // When we see a newline, increment the line number and terminate
      // comments.
      if (ch == '\n') {
        line_num_++;
        if (in_comment) in_comment = false;
        // Don't return escaped newlines as terminators.
        if (in_escape) {
          in_escape = false;
          continue;
        }
      }
      if (ch == EOF) done_ = true;
      *token = "\n";
      *token_type = TOKEN_NEWLINE;
      return true;
    }

    // Invariant: At this point we won't see any newlines or EOFs, and if
    // there's a whitespace character, it's in a quoted string or part of
    // an escape.
    CHECK(ch != '\n');
    CHECK(ch != EOF);
    CHECK(!isspace(ch) || in_single_quote || in_double_quote || in_escape);

    // Handle single quotes, double quotes, escapes, and comments.
    if (ch == '\'' && !in_escape && !in_double_quote) {
      in_single_quote = !in_single_quote;
      if (!in_token) in_token = true;
      if (in_single_quote) {
        quote_start_line = line_num_;
        bare_token = false;
      }
      continue;
    }
    if (ch == '"' && !in_escape && !in_single_quote) {
      in_double_quote = !in_double_quote;;
      if (!in_token) in_token = true;
      if (in_double_quote) {
        quote_start_line = line_num_;
        bare_token = false;
      }
      continue;
    }
    if (ch == '\\' && !in_escape) {
      in_escape = true;
      continue;
    }
    if (ch == '/' && !in_escape && !in_single_quote && !in_double_quote &&
        bare_last_char && last_ch == '/') {
      in_comment = true;
      *token = token->substr(0, token->size() - 1);
      if (token->empty() && bare_token) in_token = false;
      continue;
    }

    // At this point, we have a character that we're going to add to the
    // in-progress token.
    bare_last_char = true;
    if (in_escape) {
      // Handle escape sequences.
      in_escape = false;
      switch (ch) {
        case 'f': ch = '\f'; break;
        case 'n': ch = '\n'; break;
        case 'r': ch = '\r'; break;
        case 't': ch = '\t'; break;
        case 'v': ch = '\v'; break;
        // Everything else is left untouched.
      }
      bare_token = false;
      bare_last_char = false;
    }
    *token += static_cast<char>(ch);
    in_token = true;
  }
  CHECK(false);
}


bool ConfigParser::Parse(Tokenizer* tokenizer, ParsedConfig* config) {
  CHECK(tokenizer);
  CHECK(config);

  stack<ParsedConfig::Node*> node_stack;
  node_stack.push(&config->root);

  ParsedConfig::Node* current_node = NULL;

  bool in_concat = false;

  string token;
  TokenType token_type = NUM_TOKEN_TYPES;
  bool error = false;
  while (tokenizer->GetNextToken(&token, &token_type, &error)) {
    if (token_type == TOKEN_NEWLINE) {
      // If we're in a concatenation request, we'll ignore the terminator.
      if (in_concat) continue;
      current_node = NULL;
    } else if (token_type == TOKEN_LEFT_BRACE) {
      // FIXME: report an error if we're concatenating
      CHECK(!in_concat);
      // FIXME: create empty current_node if NULL
      CHECK(current_node);
      node_stack.push(current_node);
      current_node = NULL;
    } else if (token_type == TOKEN_RIGHT_BRACE) {
      // FIXME: report an error if we're concatenating
      CHECK(!in_concat);
      node_stack.pop();
      // FIXME: report an error if node_stack is empty
      CHECK(!node_stack.empty());
      current_node = NULL;
    } else {
      // We treat "." tokens as concatenation requests, unless we're
      // already in a request or are at the begin of a node's tokens.
      if (token_type == TOKEN_PERIOD && !in_concat && current_node) {
        in_concat = true;
        continue;
      }
      if (!current_node) {
        current_node = new ParsedConfig::Node;
        node_stack.top()->children.push_back(
            ref_ptr<ParsedConfig::Node>(current_node));
      }
      if (in_concat) {
        // Invariant: if we're concatenating, we already have a token.
        CHECK(current_node && !current_node->tokens.empty());
        current_node->tokens.back().append(token);
        in_concat = false;
      } else {
        current_node->tokens.push_back(token);
      }
    }
  }
  // FIXME: report error if we reach the end while still waiting for a
  // concatenated string
  CHECK(!in_concat);

  return true;
}

}  // namespace wham
