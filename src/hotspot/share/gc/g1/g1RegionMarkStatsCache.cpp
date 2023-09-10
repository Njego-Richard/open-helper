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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/g1RegionMarkStatsCache.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"

G1RegionMarkStatsCache::G1RegionMarkStatsCache(G1RegionMarkStats* target, uint max_regions, uint num_cache_entries) :
  _num_stats(max_regions),
  _target(target),
  _num_cache_entries(num_cache_entries),
  _cache_hits(0),
  _cache_misses(0) {

  guarantee(is_power_of_2(num_cache_entries),
            "Number of cache entries must be power of two, but is %u", num_cache_entries);
  _cache = NEW_C_HEAP_ARRAY(G1RegionMarkStatsCacheEntry, _num_cache_entries, mtGC);
  _num_cache_entries_mask = _num_cache_entries - 1;
}

G1RegionMarkStatsCache::~G1RegionMarkStatsCache() {
  FREE_C_HEAP_ARRAY(G1RegionMarkStatsCacheEntry, _cache);
}

// Evict all remaining statistics, returning cache hits and misses.
Pair<size_t, size_t> G1RegionMarkStatsCache::evict_all() {
  for (uint i = 0; i < _num_cache_entries; i++) {
    evict(i);
  }
  return Pair<size_t,size_t>(_cache_hits, _cache_misses);
}

// Reset all cache entries to their default values.
void G1RegionMarkStatsCache::reset() {
  _cache_hits = 0;
  _cache_misses = 0;

  for (uint i = 0; i < _num_cache_entries; i++) {
    _cache[i].clear();
  }
}
