// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "util.h"

#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace wham {

Logger::Logger(const string& filename, int line_num)
    : newline_seen_(false),
      orig_format_flags_(cerr.flags()) {
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);
  char time_str[12];
  strftime(time_str, sizeof(time_str), "%m%d %H%m%S", &tm);
  cerr << time_str << " " << filename << ":" << line_num << "] ";
}

Logger::~Logger() {
  if (!newline_seen_) cerr << "\n";
  cerr.flags(orig_format_flags_);
}

Logger& Logger::operator<<(long v) { cerr << v; return *this; }
Logger& Logger::operator<<(unsigned long v) { cerr << v; return *this; }
Logger& Logger::operator<<(bool v) { cerr << v; return *this; }
Logger& Logger::operator<<(short v) { cerr << v; return *this; }
Logger& Logger::operator<<(unsigned short v) { cerr << v; return *this; }
Logger& Logger::operator<<(int v) { cerr << v; return *this; }
Logger& Logger::operator<<(unsigned int v) { cerr << v; return *this; }
Logger& Logger::operator<<(double v) { cerr << v; return *this; }
Logger& Logger::operator<<(float v) { cerr << v; return *this; }
Logger& Logger::operator<<(long double v) { cerr << v; return *this; }
Logger& Logger::operator<<(const void* v) { cerr << v; return *this; }

Logger& Logger::operator<<(ios_base& (*f)(ios_base&)) {
  cerr << f;
  return *this;
}

Logger& Logger::operator<<(const char* v) {
  return (*this << string(v));
}

Logger& Logger::operator<<(const string& v) {
  newline_seen_ = (!v.empty() && v[v.size()-1] == '\n');
  cerr << v;
  return *this;
}


double GetCurrentTime() {
  struct timeval tv;
  CHECK(gettimeofday(&tv, NULL) == 0);
  return tv.tv_sec + (tv.tv_usec / 1000000.0);
}


void FillTimeval(double time, struct timeval *tv) {
  CHECK(tv);
  tv->tv_sec = static_cast<__time_t>(time);
  tv->tv_usec =
      static_cast<__suseconds_t>(1000000 * (time - static_cast<int>(time)));
}


void SplitString(const string& str, vector<string>* parts) {
  CHECK(parts);
  parts->clear();
  static pcrecpp::RE re("\\s*(\\S+)");
  pcrecpp::StringPiece input(str);
  string part;
  while (re.Consume(&input, &part)) parts->push_back(part);
}


vector<string> SplitString(const string& str) {
  vector<string> parts;
  SplitString(str, &parts);
  return parts;
}


void SplitStringUsing(const string& str,
                      const string& delim,
                      vector<string>* parts) {
  CHECK(parts);
  CHECK(!delim.empty());
  parts->clear();
  size_t start = 0;
  while (start < str.size()) {
    size_t delim_pos = str.find(delim, start);
    if (delim_pos == string::npos) delim_pos = str.size();
    if (delim_pos > start) {
      parts->push_back(str.substr(start, delim_pos - start));
    }
    start = delim_pos + delim.size();
  }
}


string StringPrintf(const char* format, ...) {
  char buffer[1024];  // FIXME: remove magic number?
  va_list argp;
  va_start(argp, format);
  vsnprintf(buffer, sizeof(buffer), format, argp);
  va_end(argp);
  return string(buffer);
}


void JoinString(const vector<string>& parts,
                const string& delim,
                string* output) {
  CHECK(output);
  ostringstream stream;
  bool first = true;
  for (vector<string>::const_iterator part = parts.begin();
       part != parts.end(); ++part) {
    if (!first) stream << delim;
    stream << *part;
    first = true;
  }
  *output = stream.str();
}


string JoinString(const vector<string>& parts, const string& delim) {
  string out;
  JoinString(parts, delim, &out);
  return out;
}

}  // namespace wham
