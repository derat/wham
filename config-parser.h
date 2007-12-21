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

struct ParsedConfig {
  ParsedConfig() {}

  void Dump() {
    root.Dump(0);
  }

  struct Node {
    Node() {}
    void Dump(int level);
    vector<string> tokens;
    vector<ref_ptr<Node> > children;
   private:
    DISALLOW_EVIL_CONSTRUCTORS(Node);
  };

  Node root;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(ParsedConfig);
};

class ConfigParser {
 public:
  //bool ParseFile(const string& filename, ParsedConfig* config);

 private:
  friend class ::ConfigParserTestSuite;

  static const int kMaxTokenLength;

  // Abstract base class for tokenizing input.
  class Tokenizer {
   public:
    Tokenizer();
    virtual ~Tokenizer() {}

    bool GetNextToken(
        char* token,
        int max_token_length,
        bool* terminator,
        bool* error);

   protected:
    virtual bool Valid() = 0;

    virtual int GetCharImpl() = 0;

   private:
    int GetChar() {
      if (have_ungetted_char_) {
        have_ungetted_char_ = false;
        return ungetted_char_;
      }
      return GetCharImpl();
    }

    void UngetChar(int ch) {
      CHECK(!have_ungetted_char_);
      ungetted_char_ = ch;
      have_ungetted_char_ = true;
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
  static bool Parse(Tokenizer* tokenizer, ParsedConfig* config);

  DISALLOW_EVIL_CONSTRUCTORS(ConfigParser);
};

}  // namespace wham

#endif
