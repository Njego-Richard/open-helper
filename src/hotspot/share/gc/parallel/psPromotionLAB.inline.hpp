/*
 * Copyright (c) 2014, 2015, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_PARALLEL_PSPROMOTIONLAB_INLINE_HPP
#define SHARE_VM_GC_PARALLEL_PSPROMOTIONLAB_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psPromotionLAB.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/align.hpp"

HeapWord* PSYoungPromotionLAB::allocate(size_t size) {
  // Can't assert this, when young fills, we keep the LAB around, but flushed.
  // assert(_state != flushed, "Sanity");
  HeapWord* obj = CollectedHeap::align_allocation_or_fail(top(), end(), SurvivorAlignmentInBytes);
  if (obj == NULL) {
    return NULL;
  }

  HeapWord* new_top = obj + size;
  // The 'new_top>obj' check is needed to detect overflow of obj+size.
  if (new_top > obj && new_top <= end()) {
    set_top(new_top);
    assert(is_aligned(obj, SurvivorAlignmentInBytes) && is_object_aligned(new_top),
           "checking alignment");
    return obj;
  } else {
    set_top(obj);
    return NULL;
  }
}

#endif // SHARE_VM_GC_PARALLEL_PSPROMOTIONLAB_INLINE_HPP
