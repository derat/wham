// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "command.h"

#include "util.h"

namespace wham {

Command::Info Command::info_[] = {
  { "close_window",          CLOSE_WINDOW,          NO_ARG },
  { "create_anchor",         CREATE_ANCHOR,         NO_ARG },
  { "exec",                  EXEC,                  STRING_ARG },
  { "switch_nearest_anchor", SWITCH_NEAREST_ANCHOR, DIRECTION_ARG },
  { "switch_nth_anchor",     SWITCH_NTH_ANCHOR,     INT_ARG },
  { "switch_nth_window",     SWITCH_NTH_WINDOW,     INT_ARG },
  { "unknown",               UNKNOWN,               NO_ARG },
};

map<string, Command::Type> Command::name_to_type_;

map<Command::Type, string> Command::type_to_name_;

map<Command::Type, Command::ArgType> Command::type_to_arg_type_;

bool Command::initialized_ = false;


Command::Command(const string& name, const vector<string>& args)
    : type_(ToType(name)),
      valid_(true) {
  if (type_ == UNKNOWN) {
    valid_ = false;
    return;
  }

  switch (GetArgType(type_)) {
    case NO_ARG:
      if (!args.empty()) valid_ = false;
      break;
    case INT_ARG:
      if (args.size() != 1 || args[0].empty()) {
        valid_ = false;
      } else {
        char* end_ptr = NULL;
        arg_.i = strtol(args[0].c_str(), &end_ptr, 10);
        if (!end_ptr || *end_ptr != '\0') valid_ = false;
      }
      break;
    case BOOL_ARG:
      if (args.size() != 1 || args[0].empty()) {
        valid_ = false;
      } else if (strcasecmp(args[0].c_str(), "true") == 0 || args[0] == "1") {
        arg_.b = true;
      } else if (strcasecmp(args[0].c_str(), "false") == 0 || args[0] == "0") {
        arg_.b = false;
      } else {
        valid_ = false;
      }
      break;
    case STRING_ARG:
      if (args.size() != 1) {
        valid_ = false;
        arg_.s = NULL;
      } else {
        arg_.s = new string(args[0]);
      }
      break;
    case DIRECTION_ARG:
      if (args.size() != 1) {
        valid_ = false;
      } else {
        if (strcasecmp(args[0].c_str(), "up") == 0) arg_.dir = UP;
        else if (strcasecmp(args[0].c_str(), "down") == 0) arg_.dir = DOWN;
        else if (strcasecmp(args[0].c_str(), "left") == 0) arg_.dir = LEFT;
        else if (strcasecmp(args[0].c_str(), "right") == 0) arg_.dir = RIGHT;
        else valid_ = false;
      }
      break;
    default:
      CHECK(false);
  }
}


Command::Command(const Command& o) {
  *this = o;
}


Command::~Command() {
  if (Valid() && GetArgType(type_) == STRING_ARG) {
    delete arg_.s;
    arg_.s = NULL;
  }
}


Command::Command& Command::operator=(const Command& o) {
  type_ = o.type_;
  valid_ = o.valid_;
  if (Valid() && GetArgType(type_) == STRING_ARG) {
    arg_.s = new string(*o.arg_.s);
  } else {
    arg_ = o.arg_;
  }
  return *this;
}


bool Command::Valid() const {
  return valid_;
}


string Command::ToName(Command::Type type) {
  InitializeStaticData();
  return FindWithDefault(type_to_name_, type, string("unknown"));
}


Command::Type Command::ToType(const string& name) {
  InitializeStaticData();
  return FindWithDefault(name_to_type_, name, UNKNOWN);
}


Command::ArgType Command::GetArgType(Command::Type type) {
  InitializeStaticData();
  return FindWithDefault(type_to_arg_type_, type, NO_ARG);
}


void Command::InitializeStaticData() {
  if (initialized_) return;
  for (int i = 0; info_[i].type != UNKNOWN; ++i) {
    Info& info = info_[i];
    name_to_type_.insert(make_pair(info.name, info.type));
    type_to_name_.insert(make_pair(info.type, info.name));
    type_to_arg_type_.insert(make_pair(info.type, info.arg_type));
  }
  initialized_ = true;
}

}  // namespace wham
