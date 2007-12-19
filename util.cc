// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include "util.h"

#include <ctime>
#include <iostream>

namespace wham {

Logger::Logger(const string& filename, int line_num)
    : newline_seen_(false) {
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);
  char time_str[12];
  strftime(time_str, sizeof(time_str), "%m%d %H%m%S", &tm);
  cerr << time_str << " " << filename << ":" << line_num << "] ";
}

Logger::~Logger() {
  if (!newline_seen_) cerr << "\n";
}

Logger& Logger::operator<<(const string& msg) {
  newline_seen_ = (!msg.empty() && msg[msg.size()-1] == '\n');
  cerr << msg;
  return *this;
}

Logger& Logger::operator<<(int num) {
  cerr << num;
  return *this;
}

}  // namespace wham
