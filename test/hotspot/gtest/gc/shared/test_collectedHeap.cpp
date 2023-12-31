/*
 * Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.hpp"
#include "unittest.hpp"

TEST_VM(CollectedHeap, is_in) {
  CollectedHeap* heap = Universe::heap();

  uintptr_t epsilon = (uintptr_t) MinObjAlignment;
  uintptr_t heap_start = (uintptr_t) heap->reserved_region().start();
  uintptr_t heap_end = (uintptr_t) heap->reserved_region().end();

  // Test that NULL is not in the heap.
  ASSERT_FALSE(heap->is_in(NULL)) << "NULL is unexpectedly in the heap";

  // Test that a pointer to before the heap start is reported as outside the heap.
  ASSERT_GE(heap_start, ((uintptr_t) NULL + epsilon))
          << "Sanity check - heap should not start at 0";

  void* before_heap = (void*) (heap_start - epsilon);
  ASSERT_FALSE(heap->is_in(before_heap)) << "before_heap: " << p2i(before_heap)
          << " is unexpectedly in the heap";

  // Test that a pointer to after the heap end is reported as outside the heap.
  ASSERT_LE(heap_end, ((uintptr_t)-1 - epsilon))
          << "Sanity check - heap should not end at the end of address space";

  void* after_heap = (void*) (heap_end + epsilon);
  ASSERT_FALSE(heap->is_in(after_heap)) << "after_heap: " << p2i(after_heap)
          << " is unexpectedly in the heap";
}
