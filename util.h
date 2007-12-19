// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>

using namespace std;

#define LOG wham::Logger(__FILE__, __LINE__)

// FIXME: do something extra here
#define ERR wham::Logger(__FILE__, __LINE__)

#define CHECK(x) \
  if (!(x)) { \
    wham::Logger(__FILE__, __LINE__) \
        << "Assertion \"" #x "\" failed; exiting"; \
    exit(1); \
  }

class UtilTestSuite;

namespace wham {

class Logger {
 public:
  Logger(const string& filename, int line_num);
  ~Logger();

  Logger& operator<<(const string& msg);
  Logger& operator<<(int num);

 private:
  // Has the input so far ended with a newline?
  bool newline_seen_;
};  // class Logger


// A reference-counted pointer class.
template<class T>
class ref_ptr {
 public:
  ref_ptr(T* ptr=NULL)
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

 private:
  friend class ::UtilTestSuite;

  void add_ref() {
    if (refs_ != NULL) (*refs_)++;
  }
  void del_ref() {
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

}  // namespace wham

#endif  // __UTIL_H__
