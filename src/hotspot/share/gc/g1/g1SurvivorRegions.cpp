/*
 * Copyright (c) 2015, 2016, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/g1SurvivorRegions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/heapRegion.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/growableArray.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"

G1SurvivorRegions::G1SurvivorRegions() : _regions(new (ResourceObj::C_HEAP, mtGC) GrowableArray<HeapRegion*>(8, true, mtGC)) {}

void G1SurvivorRegions::add(HeapRegion* hr) {
  assert(hr->is_survivor(), "should be flagged as survivor region");
  _regions->append(hr);
}

uint G1SurvivorRegions::length() const {
  return (uint)_regions->length();
}

void G1SurvivorRegions::convert_to_eden() {
  for (GrowableArrayIterator<HeapRegion*> it = _regions->begin();
       it != _regions->end();
       ++it) {
    HeapRegion* hr = *it;
    hr->set_eden_pre_gc();
  }
  clear();
}

void G1SurvivorRegions::clear() {
  _regions->clear();
}

