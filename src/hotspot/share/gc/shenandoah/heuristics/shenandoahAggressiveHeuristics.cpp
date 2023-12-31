/*
 * Copyright (c) 2018, Red Hat, Inc. All rights reserved.
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

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/heuristics/shenandoahAggressiveHeuristics.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahCollectionSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeapRegion.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/logTag.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"

ShenandoahAggressiveHeuristics::ShenandoahAggressiveHeuristics() : ShenandoahHeuristics() {
  // Do not shortcut evacuation
  SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahImmediateThreshold, 100);

  // Aggressive evacuates everything, so it needs as much evac space as it can get
  SHENANDOAH_ERGO_ENABLE_FLAG(ShenandoahEvacReserveOverflow);

  // If class unloading is globally enabled, aggressive does unloading even with
  // concurrent cycles.
  if (ClassUnloading) {
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahUnloadClassesFrequency, 1);
  }
}

void ShenandoahAggressiveHeuristics::choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                                           RegionData* data, size_t size,
                                                                           size_t free) {
  for (size_t idx = 0; idx < size; idx++) {
    ShenandoahHeapRegion* r = data[idx]._region;
    if (r->garbage() > 0) {
      cset->add_region(r);
    }
  }
}

bool ShenandoahAggressiveHeuristics::should_start_gc() const {
  log_info(gc)("Trigger: Start next cycle immediately");
  return true;
}

bool ShenandoahAggressiveHeuristics::should_process_references() {
  if (!can_process_references()) return false;
  // Randomly process refs with 50% chance.
  return (os::random() & 1) == 1;
}

bool ShenandoahAggressiveHeuristics::should_unload_classes() {
  if (!can_unload_classes_normal()) return false;
  if (has_metaspace_oom()) return true;
  // Randomly unload classes with 50% chance.
  return (os::random() & 1) == 1;
}
