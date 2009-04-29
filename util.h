// Copyright 2007 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __UTIL_H__
#define __UTIL_H__

#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <pcrecpp.h>
#include <string>
#include <sys/time.h>
#include <vector>

using namespace std;

#define LOG wham::Logger(__FILE__, __LINE__)

// FIXME: do something extra here
#define ERROR wham::Logger(__FILE__, __LINE__)
#define DEBUG wham::Logger(__FILE__, __LINE__)

#define CHECK(x)                                                               \
  if (!(x)) {                                                                  \
    ERROR << "Assertion \"" #x "\" failed; exiting";                           \
    abort();                                                                   \
  }

// FIXME: this is completely lame; it should print the values without
// evaluating the arguments a second time
#define CHECK_EQ(x, y)                                                         \
  if ((x) != (y)) {                                                            \
    ERROR << "Assertion " #x " == " #y " failed; exiting (\""                  \
          << (x) << "\" != \"" << (y) << "\")";                                \
    abort();                                                                   \
  }

#if 0
template<class A, class B>
inline void CHECK_EQ_func(const char& a_expr,
                          const char& b_expr,
                          const A& a,
                          const B& b) {
  if (a != b) {
    ERROR << "Assertion " << a_expr << " == " << b_expr << " failed; "
          << "exiting (\"" << a << "\" != \"" << b << "\")";
    abort();
  }
}

#define CHECK_EQ(a, b) CHECK_EQ_func(#a, #b, a, b);
#endif


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

  // Release and return the pointer.
  // This must be the only reference to it.
  T* release() {
    if (refs_ != NULL) {
      CHECK_EQ(*refs_, 1);
      delete refs_;
      refs_ = NULL;
    }
    T* ptr = ptr_;
    ptr_ = NULL;
    return ptr;
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


// Stacker maintains an ordering of objects (e.g. windows) in which changes
// can be made in faster-than-linear time.
template<class T>
class Stacker {
 public:
  // Get the ordered list of items.
  const list<T>& items() const { return items_; }

  // Add an item on the top of the stack.
  void AddOnTop(T item) {
    if (index_.find(item) != index_.end()) {
      ERROR << "Ignoring request to add already-present item "
            << item << " on top";
      return;
    }
    items_.push_front(item);
    index_.insert(make_pair(item, items_.begin()));
  }

  // Add an item on the bottom of the stack.
  void AddOnBottom(T item) {
    if (index_.find(item) != index_.end()) {
      ERROR << "Ignoring request to add already-present item "
            << item << " on bottom";
      return;
    }
    items_.push_back(item);
    index_.insert(make_pair(item, --(items_.end())));
  }

  // Add 'item' under 'under_item'.  'under_item' must already exist on the
  // stack.
  void AddUnder(T other_item, T item) {
    if (index_.find(item) != index_.end()) {
      ERROR << "Ignoring request to add already-present item "
            << item << " under item " << other_item;
      return;
    }
    typename IteratorMap::iterator other_it = index_.find(other_item);
    if (other_it == index_.end()) {
      ERROR << "Ignoring request to add item " << item
            << " under not-present item " << other_item;
      return;
    }
    // Lists don't support operator+ or operator-, so we need to use ++.
    // Make a copy of the iterator before doing this so that we don't screw
    // up the previous value in the map.
    typename list<T>::iterator new_it = other_it->second;
    typename list<T>::iterator it = items_.insert(++new_it, item);
    index_.insert(make_pair(item, it));
  }

  // Remove an item from the stack.
  void Remove(T item) {
    typename IteratorMap::iterator it = index_.find(item);
    if (it == index_.end()) {
      ERROR << "Ignoring request to remove not-present item " << item;
      return;
    }
    items_.erase(it->second);
    index_.erase(it);
  }

 private:
  // Items stacked from top to bottom.
  list<T> items_;

  typedef map<T, typename list<T>::iterator> IteratorMap;

  // Index into 'items_'.
  IteratorMap index_;
};


template<class K, class V>
V FindWithDefault(const map<K, V>& the_map, const K& key, const V& def) {
  // FIXME: It's dumb that I'm doing two lookups here.  I'm too dense to
  // figure out the syntax for declaring a const_iterator of the templated
  // map class -- the obvious "map<K, V>::const_iterator it" yields
  // "expected `;' before 'it'".
  if (the_map.find(key) == the_map.end()) return def;
  return the_map.find(key)->second;
}


// Get the number of seconds since the epoch.
double GetCurrentTime();


// Fill 'tv' with the time from 'time'.
void FillTimeval(double time, struct timeval* tv);


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
