// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <map>
#include <string>
#include <vector>

#include "util.h"

using namespace std;

class KeyBindingsTestSuite;

namespace wham {

class Command {
 public:
  enum Type {
    ATTACH_TAGGED_WINDOWS,
    CLOSE_WINDOW,
    CREATE_ANCHOR,
    CYCLE_ANCHOR_GRAVITY,
    CYCLE_WINDOW_CONFIG,
    DISPLAY_WINDOW_PROPS,
    EXEC,
    SET_ATTACH_ANCHOR,
    SWITCH_NEAREST_ANCHOR,
    SWITCH_NTH_ANCHOR,
    SWITCH_NTH_WINDOW,
    TOGGLE_TAG,
    UNKNOWN,
  };

  Command()
      : type_(UNKNOWN),
        valid_(false) {}
  Command(const string& name, const vector<string>& args);
  Command(const Command& o);
  ~Command();

  Command& operator=(const Command& o);

  Type type() const { return type_; }

  bool Valid() const;

  int GetIntArg() const {
    CHECK(GetArgType(type_) == INT_ARG);
    CHECK(Valid());
    return arg_.i;
  }

  bool GetBoolArg() const {
    CHECK(GetArgType(type_) == BOOL_ARG);
    CHECK(Valid());
    return arg_.b;
  }

  const string& GetStringArg() const {
    CHECK(GetArgType(type_) == STRING_ARG);
    CHECK(Valid());
    return *arg_.s;
  }

  // TODO: Move this somewhere more general.
  enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };
  Direction GetDirectionArg() const {
    CHECK(GetArgType(type_) == DIRECTION_ARG);
    CHECK(Valid());
    return arg_.dir;
  }

  string ToString() const {
    return ToName(type_);
  }

  // Get the name of a command type.
  static string ToName(Type type);

  // Look up a command from its name.
  // Returns UNKNOWN for invalid names.
  static Type ToType(const string& name);

 private:
  friend class ::KeyBindingsTestSuite;

  // Different types of arguments that can be required by a command
  enum ArgType {
    NO_ARG,
    INT_ARG,
    BOOL_ARG,
    STRING_ARG,
    DIRECTION_ARG,  // case-insensitive "up", "down", "left", or "right"
  };

  // Get the required number of arguments for a command type.
  static ArgType GetArgType(Type type);

  // Initialize static data.  Does nothing if it's already initialized.
  static void InitializeStaticData();

  // This command's type.
  Type type_;

  // Argument associated with the command.
  // This is ugly... We can't use a C++ string in a union, so we have to
  // use a pointer to one instead and define our own copy constructor and
  // assignment operators that make a copy of the existing pointer.
  union arg_ {
    int i;
    bool b;
    string* s;
    Direction dir;
  } arg_;

  // Is this command valid (that is, does it have a known type and an
  // argument that is suitable for its type)?
  bool valid_;

  // Array containing information about commands.
  struct Info {
    string name;
    Type type;
    ArgType arg_type;
  };
  const static Info info_[];

  // Maps generated from 'info_' and a bool to track whether they've been
  // initialized yet or not.
  static map<string, Type> name_to_type_;
  static map<Type, string> type_to_name_;
  static map<Type, ArgType> type_to_arg_type_;
  static bool initialized_;
};

}  // namespace wham

#endif
