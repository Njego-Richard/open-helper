/*
 * Copyright (c) 2011, 2016, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.

 */

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "unittest.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/linkedlist.hpp"

class Integer : public StackObj {
 private:
  int _value;
 public:

  Integer(int i) : _value(i) {
  }

  int value() const {
    return _value;
  }

  bool equals(const Integer& i) const {
    return _value == i.value();
  }

  static int compare(const Integer& i1, const Integer& i2) {
    return i1.value() - i2.value();
  }
};

static void check_list_values(const int* expected, const LinkedList<Integer>* list) {
  LinkedListNode<Integer>* head = list->head();
  int index = 0;
  while (head != NULL) {
    ASSERT_EQ(expected[index], head->peek()->value())
            << "Unexpected value at index " << index;
    head = head->next();
    index++;
  }
}

const Integer one(1), two(2), three(3), four(4), five(5), six(6);

// Test regular linked list
TEST(LinkedList, simple) {
  LinkedListImpl<Integer, ResourceObj::C_HEAP, mtTest> ll;

  ASSERT_TRUE(ll.is_empty()) << "Start with empty list";

  ll.add(six);
  ASSERT_TRUE(!ll.is_empty()) << "Should not be empty";

  Integer* i = ll.find(six);
  ASSERT_TRUE(i != NULL) << "Should find it";
  ASSERT_EQ(six.value(), i->value()) << "Should be 6";

  i = ll.find(three);
  ASSERT_TRUE(i == NULL) << "Not in the list";

  LinkedListNode<Integer>* node = ll.find_node(six);
  ASSERT_TRUE(node != NULL) << "6 is in the list";

  ll.insert_after(three, node);
  ll.insert_before(one, node);
  int expected[3] = {1, 6, 3};
  check_list_values(expected, &ll);
}

// Test sorted linked list
TEST(SortedLinkedList, simple) {
  LinkedListImpl<Integer, ResourceObj::C_HEAP, mtTest> ll;
  ll.add(one);
  ll.add(six);
  ll.add(three);
  ll.add(two);
  ll.add(four);
  ll.add(five);

  SortedLinkedList<Integer, Integer::compare, ResourceObj::C_HEAP, mtTest> sl;
  ASSERT_TRUE(sl.is_empty()) << "Start with empty list";

  size_t ll_size = ll.size();
  sl.move(&ll);
  size_t sl_size = sl.size();

  ASSERT_EQ(ll_size, sl_size) << "Should be the same size";
  ASSERT_TRUE(ll.is_empty()) << "No more entries";

  // sorted result
  int sorted_result[] = {1, 2, 3, 4, 5, 6};
  check_list_values(sorted_result, &sl);
  if (HasFatalFailure()) {
    return;
  }

  LinkedListNode<Integer>* node = sl.find_node(four);
  ASSERT_TRUE(node != NULL) << "4 is in the list";
  sl.remove_before(node);
  sl.remove_after(node);
  int remains[] = {1, 2, 4, 6};
  check_list_values(remains, &sl);
}
