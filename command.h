// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <map>
#include <string>
#include <vector>

#include "util.h"

using namespace std;

namespace wham {

struct Command {
 public:
  enum Type {
    CLOSE_WINDOW,
    CREATE_ANCHOR,
    EXEC,
    SWITCH_WINDOW,
    UNKNOWN,
  };

  Command()
      : type(UNKNOWN),
        args() {}
  Command(const string& name, const vector<string>& args)
      : type(ToType(name)),
        args(args) {}

  string ToString() const {
    return ToName(type);
  }

  bool CheckArgs() const;

  // Get the name of a command type.
  static string ToName(Type type);

  // Look up a command from its name.
  // Returns UNKNOWN for invalid names.
  static Type ToType(const string& name);

  // Get the required number of arguments for a command type.
  static uint NumArgs(Type type);

  Type type;
  vector<string> args;

 private:
  // Initialize static data.  Does nothing if it's already initialized.
  static void InitializeStaticData();

  // Array containing information about commands.
  struct Info {
    string name;
    Type type;
    uint num_args;
  };
  static Info info_[];

  // Maps generated from info_ and a bool to track whether they've been
  // initialized yet or not.
  static map<string, Type> name_to_type_;
  static map<Type, string> type_to_name_;
  static map<Type, uint> type_to_num_args_;
  static bool initialized_;
};

}  // namespace wham

#endif
