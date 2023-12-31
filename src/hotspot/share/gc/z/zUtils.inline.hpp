/*
 * Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_GC_Z_ZUTILS_INLINE_HPP
#define SHARE_GC_Z_ZUTILS_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zOop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zUtils.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/align.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/copy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"

inline size_t ZUtils::round_up_power_of_2(size_t value) {
  assert(value != 0, "Invalid value");

  if (is_power_of_2(value)) {
    return value;
  }

  return (size_t)1 << (log2_intptr(value) + 1);
}

inline size_t ZUtils::round_down_power_of_2(size_t value) {
  assert(value != 0, "Invalid value");
  return (size_t)1 << log2_intptr(value);
}

inline size_t ZUtils::bytes_to_words(size_t size_in_bytes) {
  assert(is_aligned(size_in_bytes, BytesPerWord), "Size not word aligned");
  return size_in_bytes >> LogBytesPerWord;
}

inline size_t ZUtils::words_to_bytes(size_t size_in_words) {
  return size_in_words << LogBytesPerWord;
}

inline size_t ZUtils::object_size(uintptr_t addr) {
  return words_to_bytes(ZOop::to_oop(addr)->size());
}

inline void ZUtils::object_copy(uintptr_t from, uintptr_t to, size_t size) {
  Copy::aligned_disjoint_words((HeapWord*)from, (HeapWord*)to, bytes_to_words(size));
}

#endif // SHARE_GC_Z_ZUTILS_INLINE_HPP
