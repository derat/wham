// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "command.h"

#include "util.h"

namespace wham {

Command::Info Command::info_[] = {
  { "close_window",  CLOSE_WINDOW,  0 },
  { "create_anchor", CREATE_ANCHOR, 0 },
  { "exec",          EXEC,          1 },
  { "switch_anchor", SWITCH_ANCHOR, 1 },
  { "switch_window", SWITCH_WINDOW, 1 },
  { "unknown",       UNKNOWN,       0 },
};

map<string, Command::Type> Command::name_to_type_;

map<Command::Type, string> Command::type_to_name_;

map<Command::Type, uint> Command::type_to_num_args_;

bool Command::initialized_ = false;


bool Command::CheckArgs() const {
  return args.size() == NumArgs(type);
}


string Command::ToName(Command::Type type) {
  InitializeStaticData();
  return FindWithDefault(type_to_name_, type, string("unknown"));
}


Command::Type Command::ToType(const string& name) {
  InitializeStaticData();
  return FindWithDefault(name_to_type_, name, UNKNOWN);
}


uint Command::NumArgs(Command::Type type) {
  InitializeStaticData();
  return FindWithDefault(type_to_num_args_, type, 0U);
}


void Command::InitializeStaticData() {
  if (initialized_) return;
  for (int i = 0; info_[i].type != UNKNOWN; ++i) {
    Info& info = info_[i];
    name_to_type_.insert(make_pair(info.name, info.type));
    type_to_name_.insert(make_pair(info.type, info.name));
    type_to_num_args_.insert(make_pair(info.type, info.num_args));
  }
  initialized_ = true;
}

}  // namespace wham
