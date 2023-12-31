/*
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/g1BarrierSet.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/g1BarrierSetRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/g1ThreadLocalData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/interfaceSupport.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"

void G1BarrierSetRuntime::write_ref_array_pre_oop_entry(oop* dst, size_t length) {
  G1BarrierSet *bs = barrier_set_cast<G1BarrierSet>(BarrierSet::barrier_set());
  bs->write_ref_array_pre(dst, length, false);
}

void G1BarrierSetRuntime::write_ref_array_pre_narrow_oop_entry(narrowOop* dst, size_t length) {
  G1BarrierSet *bs = barrier_set_cast<G1BarrierSet>(BarrierSet::barrier_set());
  bs->write_ref_array_pre(dst, length, false);
}

void G1BarrierSetRuntime::write_ref_array_post_entry(HeapWord* dst, size_t length) {
  G1BarrierSet *bs = barrier_set_cast<G1BarrierSet>(BarrierSet::barrier_set());
  bs->G1BarrierSet::write_ref_array(dst, length);
}

// G1 pre write barrier slowpath
JRT_LEAF(void, G1BarrierSetRuntime::write_ref_field_pre_entry(oopDesc* orig, JavaThread *thread))
  if (orig == NULL) {
    assert(false, "should be optimized out");
    return;
  }
  assert(oopDesc::is_oop(orig, true /* ignore mark word */), "Error");
  // store the original value that was in the field reference
  G1ThreadLocalData::satb_mark_queue(thread).enqueue(orig);
JRT_END

// G1 post write barrier slowpath
JRT_LEAF(void, G1BarrierSetRuntime::write_ref_field_post_entry(void* card_addr, JavaThread* thread))
  G1ThreadLocalData::dirty_card_queue(thread).enqueue(card_addr);
JRT_END
