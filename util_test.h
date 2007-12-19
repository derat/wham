// Copyright 2007, Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cxxtest/TestSuite.h>

#include "util.h"

using namespace wham;

class UtilTestSuite : public CxxTest::TestSuite {
 public:
  void testRefPtr() {
    ref_ptr<int> ptr;

    TS_ASSERT(ptr.ptr_ == NULL);
    TS_ASSERT(ptr.refs_ == NULL);

    int* i = new int(3);

    ptr.reset(i);
    TS_ASSERT_EQUALS(ptr.ptr_, i);
    TS_ASSERT_EQUALS(*ptr.refs_, 1);

    ref_ptr<int> ptr2 = ptr;

    TS_ASSERT_EQUALS(ptr.ptr_, i);
    TS_ASSERT_EQUALS(ptr2.ptr_, i);
    TS_ASSERT_EQUALS(ptr.refs_, ptr2.refs_);
    TS_ASSERT_EQUALS(*ptr.refs_, 2);

    ptr.reset();
    TS_ASSERT(ptr.ptr_ == NULL);
    TS_ASSERT(ptr.refs_ == NULL);
    TS_ASSERT_EQUALS(ptr2.ptr_, i);
    TS_ASSERT_EQUALS(*ptr2.refs_, 1);

    ref_ptr<int> ptr3(ptr2);
    TS_ASSERT_EQUALS(ptr2.ptr_, i);
    TS_ASSERT_EQUALS(ptr3.ptr_, i);
    TS_ASSERT_EQUALS(ptr2.refs_, ptr3.refs_);
    TS_ASSERT_EQUALS(*ptr2.refs_, 2);
  }
};
