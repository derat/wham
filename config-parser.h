// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__

#include <cstdio>
#include <string>
#include <vector>

#include "util.h"

using namespace std;

class ConfigParserTestSuite;

namespace wham {

// A structured representation of the tokens parsed from a config.
struct ParsedConfig {
  ParsedConfig() {}

  // Return the parsed config as a string (for testing).
  string Dump() {
    return root.Dump(0);
  }

  // An individual node within a config, possibly containing tokens and
  // references to sub-nodes.
  struct Node {
    Node() {}
    string Dump(int level);
    vector<string> tokens;
    vector<ref_ptr<Node> > children;
   private:
    DISALLOW_EVIL_CONSTRUCTORS(Node);
  };

  // The top-level node in the config.  Contains only children.
  Node root;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(ParsedConfig);
};

// Parses configurations.
class ConfigParser {
 public:
  // Parse a config from 'filename' into 'config'.
  // If 'errors' is non-NULL, errors will be logged there.
  // Returns true on success and false otherwise.
  static bool ParseFromFile(const string& filename,
                            ParsedConfig* config,
                            vector<string>* errors);

 private:
  friend class ::ConfigParserTestSuite;

  enum TokenType {
    TOKEN_LITERAL,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_PERIOD,
    TOKEN_NEWLINE,
    NUM_TOKEN_TYPES
  };

  // Abstract base class for tokenizing input.
  class Tokenizer {
   public:
    Tokenizer();
    virtual ~Tokenizer() {}

    // Get the next token from the stream.
    // Returns true if a token was returned and false otherwise.
    // The token is written to 'token', its type (relevant in the case of a
    // bare token) is written to token_type, and 'error' is set if an error
    // was encountered.  'error' only needs to be checked when false is
    // returned.
    bool GetNextToken(
        string* token,
        TokenType* token_type,
        bool* error);

   protected:
    // Can the input source be read from?  Returns false if there was an
    // error in its initialization.
    virtual bool Valid() = 0;

    virtual int GetCharImpl() = 0;

   private:
    // Get the next character from the input source.
    int GetChar() {
      if (have_ungetted_char_) {
        have_ungetted_char_ = false;
        return ungetted_char_;
      }
      return GetCharImpl();
    }

    // Push a character that was already read into a temporary buffer such
    // that it will be the next character returned by GetChar().  This
    // buffer can only hold a single character.
    void UngetChar(int ch) {
      CHECK(!have_ungetted_char_);
      ungetted_char_ = ch;
      have_ungetted_char_ = true;
    }

    TokenType GetTokenType(const string& token) {
      if (token == "\n") return TOKEN_NEWLINE;
      if (token == "{")  return TOKEN_LEFT_BRACE;
      if (token == "}")  return TOKEN_RIGHT_BRACE;
      if (token == ".")  return TOKEN_PERIOD;
      return TOKEN_LITERAL;
    }

    bool done_;
    bool line_num_;

    int ungetted_char_;
    bool have_ungetted_char_;

    DISALLOW_EVIL_CONSTRUCTORS(Tokenizer);
  };

  // Implementation of Tokenizer that reads from a file.
  class FileTokenizer : public Tokenizer {
   public:
    FileTokenizer(const string& filename)
        : Tokenizer(),
          file_(fopen(filename.c_str(), "r")) {
    }

    ~FileTokenizer() {
      if (file_) fclose(file_);
    }

   protected:
    bool Valid() {
      return (file_ != NULL);
    }

    int GetCharImpl() {
      CHECK(file_);
      return fgetc(file_);
    }

   private:
    FILE* file_;
  };

  // Implementation of Tokenizer that reads from a string.
  class StringTokenizer : public Tokenizer {
   public:
    StringTokenizer(const string& input)
        : Tokenizer(),
          input_(input),
          pos_(0) {
    }

   protected:
    bool Valid() {
      return true;
    }

    int GetCharImpl() {
      if (pos_ >= input_.size()) return EOF;
      return input_[pos_++];
    }

   private:
    string input_;
    size_t pos_;
  };

  // Parse a config.
  // Returns true on success and false otherwise.
  static bool Parse(Tokenizer* tokenizer, ParsedConfig* config);

  DISALLOW_EVIL_CONSTRUCTORS(ConfigParser);
};

}  // namespace wham

#endif
