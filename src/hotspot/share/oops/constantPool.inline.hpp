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

#ifndef SHARE_VM_OOPS_CONSTANTPOOL_INLINE_HPP
#define SHARE_VM_OOPS_CONSTANTPOOL_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/constantPool.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/cpCache.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/orderAccess.hpp"

inline CPSlot ConstantPool::slot_at(int which) const {
  assert(is_within_bounds(which), "index out of bounds");
  assert(!tag_at(which).is_unresolved_klass() && !tag_at(which).is_unresolved_klass_in_error(), "Corrupted constant pool");
  // Uses volatile because the klass slot changes without a lock.
  intptr_t adr = OrderAccess::load_acquire(obj_at_addr(which));
  assert(adr != 0 || which == 0, "cp entry for klass should not be zero");
  return CPSlot(adr);
}

inline Klass* ConstantPool::resolved_klass_at(int which) const {  // Used by Compiler
  guarantee(tag_at(which).is_klass(), "Corrupted constant pool");
  // Must do an acquire here in case another thread resolved the klass
  // behind our back, lest we later load stale values thru the oop.
  CPKlassSlot kslot = klass_slot_at(which);
  assert(tag_at(kslot.name_index()).is_symbol(), "sanity");

  Klass** adr = resolved_klasses()->adr_at(kslot.resolved_klass_index());
  return OrderAccess::load_acquire(adr);
}

inline bool ConstantPool::is_pseudo_string_at(int which) {
  assert(tag_at(which).is_string(), "Corrupted constant pool");
  return slot_at(which).is_pseudo_string();
}

inline oop ConstantPool::pseudo_string_at(int which, int obj_index) {
  assert(is_pseudo_string_at(which), "must be a pseudo-string");
  oop s = resolved_references()->obj_at(obj_index);
  return s;
}

inline oop ConstantPool::pseudo_string_at(int which) {
  assert(is_pseudo_string_at(which), "must be a pseudo-string");
  int obj_index = cp_to_object_index(which);
  oop s = resolved_references()->obj_at(obj_index);
  return s;
}

#endif // SHARE_VM_OOPS_CONSTANTPOOL_INLINE_HPP
