/*
 * Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/metaspace.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/metaspace/virtualSpaceList.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/mutex.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/mutexLocker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "unittest.hpp"

using namespace metaspace;


// Test the cheerful multitude of metaspace-contains-functions.
class MetaspaceIsMetaspaceObjTest : public ::testing::Test {
  Mutex* _lock;
  ClassLoaderMetaspace* _ms;

public:

  MetaspaceIsMetaspaceObjTest() : _lock(NULL), _ms(NULL) {}

  virtual void SetUp() {
  }

  virtual void TearDown() {
    delete _ms;
    delete _lock;
  }

  void do_test(Metaspace::MetadataType mdType) {
    _lock = new Mutex(Monitor::native, "gtest-IsMetaspaceObjTest-lock", false, Monitor::_safepoint_check_never);
    {
      MutexLockerEx ml(_lock, Mutex::_no_safepoint_check_flag);
      _ms = new ClassLoaderMetaspace(_lock, Metaspace::StandardMetaspaceType);
    }

    const MetaspaceObj* p = (MetaspaceObj*) _ms->allocate(42, mdType);

    // Test MetaspaceObj::is_metaspace_object
    ASSERT_TRUE(MetaspaceObj::is_valid(p));

    // A misaligned object shall not be recognized
    const MetaspaceObj* p_misaligned = (MetaspaceObj*)((address)p) + 1;
    ASSERT_FALSE(MetaspaceObj::is_valid(p_misaligned));

    // Test VirtualSpaceList::contains and find_enclosing_space
    VirtualSpaceList* list = Metaspace::space_list();
    if (mdType == Metaspace::ClassType && Metaspace::using_class_space()) {
      list = Metaspace::class_space_list();
    }
    ASSERT_TRUE(list->contains(p));
    VirtualSpaceNode* const n = list->find_enclosing_space(p);
    ASSERT_TRUE(n != NULL);
    ASSERT_TRUE(n->contains(p));

    // A misaligned pointer shall be recognized by list::contains
    ASSERT_TRUE(list->contains((address)p) + 1);

    // Now for some bogus values
    ASSERT_FALSE(MetaspaceObj::is_valid((MetaspaceObj*)NULL));

    // Should exercise various paths in MetaspaceObj::is_valid()
    ASSERT_FALSE(MetaspaceObj::is_valid((MetaspaceObj*)1024));
    ASSERT_FALSE(MetaspaceObj::is_valid((MetaspaceObj*)8192));

    MetaspaceObj* p_stack = (MetaspaceObj*) &_lock;
    ASSERT_FALSE(MetaspaceObj::is_valid(p_stack));

    MetaspaceObj* p_heap = (MetaspaceObj*) os::malloc(41, mtInternal);
    ASSERT_FALSE(MetaspaceObj::is_valid(p_heap));
    os::free(p_heap);

    // Test Metaspace::contains_xxx
    ASSERT_TRUE(Metaspace::contains(p));
    ASSERT_TRUE(Metaspace::contains_non_shared(p));

    delete _ms;
    _ms = NULL;
    delete _lock;
    _lock = NULL;
  }

};

TEST_VM_F(MetaspaceIsMetaspaceObjTest, non_class_space) {
  do_test(Metaspace::NonClassType);
}

TEST_VM_F(MetaspaceIsMetaspaceObjTest, class_space) {
  do_test(Metaspace::ClassType);
}

