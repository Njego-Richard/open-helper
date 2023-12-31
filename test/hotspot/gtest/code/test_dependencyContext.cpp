/*
 * Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
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
 *
 */

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/dependencyContext.hpp"
#include "unittest.hpp"

class TestDependencyContext {
 public:
  nmethod* _nmethods[3];

  intptr_t _dependency_context;

  DependencyContext dependencies() {
    DependencyContext depContext(&_dependency_context);
    return depContext;
  }

  TestDependencyContext() : _dependency_context(DependencyContext::EMPTY) {
    CodeCache_lock->lock_without_safepoint_check();

    _nmethods[0] = reinterpret_cast<nmethod*>(0x8 * 0);
    _nmethods[1] = reinterpret_cast<nmethod*>(0x8 * 1);
    _nmethods[2] = reinterpret_cast<nmethod*>(0x8 * 2);

    dependencies().add_dependent_nmethod(_nmethods[2]);
    dependencies().add_dependent_nmethod(_nmethods[1]);
    dependencies().add_dependent_nmethod(_nmethods[0]);
  }

  ~TestDependencyContext() {
    dependencies().wipe();
    CodeCache_lock->unlock();
  }

  static bool has_stale_entries(DependencyContext ctx) {
    return ctx.has_stale_entries();
  }

  static bool find_stale_entries(DependencyContext ctx) {
    return ctx.find_stale_entries();
  }
};

static void test_remove_dependent_nmethod(int id, bool delete_immediately) {
  TestDependencyContext c;
  DependencyContext depContext = c.dependencies();
  ASSERT_FALSE(TestDependencyContext::find_stale_entries(depContext));
  ASSERT_FALSE(TestDependencyContext::has_stale_entries(depContext));

  nmethod* nm = c._nmethods[id];
  depContext.remove_dependent_nmethod(nm, delete_immediately);

  if (!delete_immediately) {
    ASSERT_TRUE(TestDependencyContext::find_stale_entries(depContext));
    ASSERT_TRUE(TestDependencyContext::has_stale_entries(depContext));
    ASSERT_TRUE(depContext.is_dependent_nmethod(nm));
    depContext.expunge_stale_entries();
  }

  ASSERT_FALSE(TestDependencyContext::find_stale_entries(depContext));
  ASSERT_FALSE(TestDependencyContext::has_stale_entries(depContext));
  ASSERT_FALSE(depContext.is_dependent_nmethod(nm));
}

TEST_VM(code, dependency_context) {
  test_remove_dependent_nmethod(0, false);
  test_remove_dependent_nmethod(1, false);
  test_remove_dependent_nmethod(2, false);

  test_remove_dependent_nmethod(0, true);
  test_remove_dependent_nmethod(1, true);
  test_remove_dependent_nmethod(2, true);
}
