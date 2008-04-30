// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <map>
#include <pcrecpp.h>
#include <string>
#include <vector>

using namespace std;

#define LOG wham::Logger(__FILE__, __LINE__)

// FIXME: do something extra here
#define ERROR wham::Logger(__FILE__, __LINE__)
#define DEBUG wham::Logger(__FILE__, __LINE__)

#define CHECK(x) \
  if (!(x)) { \
    wham::Logger(__FILE__, __LINE__) \
        << "Assertion \"" #x "\" failed; exiting"; \
    exit(1); \
  }

#define DISALLOW_EVIL_CONSTRUCTORS(class_name) \
  class_name(const class_name&); \
  void operator=(const class_name&)


typedef unsigned int uint;


class UtilTestSuite;

namespace wham {

class Logger {
 public:
  Logger(const string& filename, int line_num);
  ~Logger();

  Logger& operator<<(long v);
  Logger& operator<<(unsigned long v);
  Logger& operator<<(bool v);
  Logger& operator<<(short v);
  Logger& operator<<(unsigned short v);
  Logger& operator<<(int v);
  Logger& operator<<(unsigned int v);
  Logger& operator<<(double v);
  Logger& operator<<(float v);
  Logger& operator<<(long double v);
  Logger& operator<<(const void* v);
  Logger& operator<<(ios_base& (*f)(ios_base&));
  Logger& operator<<(const char* v);
  Logger& operator<<(const string& v);

 private:
  // Has the input so far ended with a newline?
  bool newline_seen_;

  // Format flags at the time we were instatiated
  ios_base::fmtflags orig_format_flags_;

  DISALLOW_EVIL_CONSTRUCTORS(Logger);
};  // class Logger


// A reference-counted pointer class.
template<class T>
class ref_ptr {
 public:
  explicit ref_ptr(T* ptr=NULL)
      : ptr_(ptr),
        refs_(ptr ? new int(0) : NULL) {
    add_ref();
  }
  ref_ptr(const ref_ptr<T>& o)
      : ptr_(o.ptr_),
        refs_(o.refs_) {
    add_ref();
  }
  ~ref_ptr() {
    del_ref();
    ptr_ = NULL;
    refs_ = NULL;
  }
  ref_ptr<T>& operator=(const ref_ptr<T>& o) {
    del_ref();
    ptr_ = o.ptr_;
    refs_ = o.refs_;
    add_ref();
    return *this;
  }

  T* get() const {
    return ptr_;
  }
  T &operator*() const {
    return *ptr_;
  }
  T *operator->() const {
    return ptr_;
  }
  void reset(T* ptr=NULL) {
    del_ref();
    ptr_ = ptr;
    refs_ = ptr ? new int(0) : NULL;
    add_ref();
  }
  void swap(ref_ptr<T>& other) {
    T* tmp_ptr = ptr_;
    ptr_ = other.ptr_;
    other.ptr_ = tmp_ptr;

    int* tmp_refs = refs_;
    refs_ = other.refs_;
    other.refs_ = tmp_refs;
  }
  bool operator==(const ref_ptr<T>& other) const {
    return ptr_ == other.ptr_;
  }
  bool operator==(const T* ptr) const {
    return ptr_ == ptr;
  }

 private:
  friend class ::UtilTestSuite;

  inline void add_ref() {
    if (refs_ != NULL) (*refs_)++;
  }
  inline void del_ref() {
    if (refs_ != NULL) {
      (*refs_)--;
      if (*refs_ == 0) {
        delete ptr_;
        delete refs_;
      }
    }
  }

  T* ptr_;
  int* refs_;
};


template<class K, class V>
V FindWithDefault(const map<K, V>& the_map, const K& key, V def) {
  // FIXME: It's dumb that I'm doing two lookups here.  I'm too dense to
  // figure out the syntax for declaring a const_iterator of the templated
  // map class -- the obvious "map<K, V>::const_iterator it" yields
  // "expected `;' before 'it'".
  if (the_map.find(key) == the_map.end()) return def;
  return the_map.find(key)->second;
}


// Split a string on whitespace, saving the individual pieces to 'parts'.
void SplitString(const string& str, vector<string>* parts);

// Split a string on whitespace, returning the individual pieces as a new
// vector.
vector<string> SplitString(const string& str);

void SplitStringUsing(const string& str,
                      const string& delim,
                      vector<string>* parts);


void JoinString(const vector<string>& parts,
                const string& delim,
                string* output);


string JoinString(const vector<string>& parts, const string& delim);


string StringPrintf(const char* format, ...);

}  // namespace wham

#endif  // __UTIL_H__
