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

#ifndef SHARE_GC_Z_ZPAGETABLE_INLINE_HPP
#define SHARE_GC_Z_ZPAGETABLE_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zAddress.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zAddressRangeMap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zPageTable.hpp"

inline ZPage* ZPageTable::get(uintptr_t addr) const {
  return _map.get(addr).page();
}

inline bool ZPageTable::is_relocating(uintptr_t addr) const {
  return _map.get(addr).relocating();
}

inline ZPageTableIterator::ZPageTableIterator(const ZPageTable* pagetable) :
    _iter(&pagetable->_map),
    _prev(NULL) {}

inline bool ZPageTableIterator::next(ZPage** page) {
  ZPageTableEntry entry;

  while (_iter.next(&entry)) {
    ZPage* const next = entry.page();
    if (next != NULL && next != _prev) {
      // Next page found
      *page = _prev = next;
      return true;
    }
  }

  // No more pages
  return false;
}

#endif // SHARE_GC_Z_ZPAGETABLE_INLINE_HPP
