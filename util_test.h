// Copyright 2007 Daniel Erat <dan@erat.org>
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

    ptr = ptr;
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

    int* i2 = new int(5);
    ref_ptr<int> ptr4(i2);
    ptr4.swap(ptr2);
    TS_ASSERT_EQUALS(ptr2.ptr_, i2);
    TS_ASSERT_EQUALS(ptr3.ptr_, i);
    TS_ASSERT_EQUALS(ptr4.ptr_, i);
    TS_ASSERT_EQUALS(ptr3.refs_, ptr4.refs_);
    TS_ASSERT_EQUALS(*ptr2.refs_, 1);
    TS_ASSERT_EQUALS(*ptr3.refs_, 2);

    // Test the release() method.
    int* i3 = new int(2);
    ref_ptr<int> ptr5(i3);
    TS_ASSERT_EQUALS(ptr5.ptr_, i3);
    TS_ASSERT_EQUALS(*ptr5.refs_, 1);
    int* i3_ptr = ptr5.release();
    TS_ASSERT_EQUALS(i3_ptr, i3);
    TS_ASSERT_EQUALS(ptr5.ptr_, static_cast<int*>(NULL));
    TS_ASSERT_EQUALS(ptr5.refs_, static_cast<int*>(NULL));
    delete i3;
  }

  void testSplitString() {
    vector<string> expected;
    vector<string> parts;

    SplitString("", &parts);
    TS_ASSERT_EQUALS(parts, expected);

    SplitString("   ", &parts);
    TS_ASSERT_EQUALS(parts, expected);

    expected.push_back("a");
    expected.push_back("b");
    expected.push_back("c");
    SplitString(" a b c ", &parts);
    TS_ASSERT_EQUALS(parts, expected);
  }

  void testSplitStringUsing() {
    vector<string> expected;
    vector<string> parts;

    SplitStringUsing("", " ", &parts);
    TS_ASSERT_EQUALS(parts, expected);

    SplitStringUsing("   ", " ", &parts);
    TS_ASSERT_EQUALS(parts, expected);

    expected.push_back("abc");
    expected.push_back("def");
    expected.push_back("ghi");
    SplitStringUsing("abc+def+ghi", "+", &parts);
    TS_ASSERT_EQUALS(parts, expected);

    expected.clear();
    expected.push_back("123");
    expected.push_back("456");
    SplitStringUsing("123abc456abc", "abc", &parts);
    TS_ASSERT_EQUALS(parts, expected);
  }
};
