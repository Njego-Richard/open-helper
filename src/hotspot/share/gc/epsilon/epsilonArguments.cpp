/*
 * Copyright (c) 2017, 2018, Red Hat, Inc. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/epsilon/epsilonArguments.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/epsilon/epsilonHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/epsilon/epsilonCollectorPolicy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcArguments.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals_extension.hpp"

size_t EpsilonArguments::conservative_max_heap_alignment() {
  return UseLargePages ? os::large_page_size() : os::vm_page_size();
}

void EpsilonArguments::initialize() {
  GCArguments::initialize();

  assert(UseEpsilonGC, "Sanity");

  // Forcefully exit when OOME is detected. Nothing we can do at that point.
  if (FLAG_IS_DEFAULT(ExitOnOutOfMemoryError)) {
    FLAG_SET_DEFAULT(ExitOnOutOfMemoryError, true);
  }

  // Warn users that non-resizable heap might be better for some configurations.
  // We are not adjusting the heap size by ourselves, because it affects startup time.
  if (InitialHeapSize != MaxHeapSize) {
    log_warning(gc)("Consider setting -Xms equal to -Xmx to avoid resizing hiccups");
  }

  // Warn users that AlwaysPreTouch might be better for some configurations.
  // We are not turning this on by ourselves, because it affects startup time.
  if (FLAG_IS_DEFAULT(AlwaysPreTouch) && !AlwaysPreTouch) {
    log_warning(gc)("Consider enabling -XX:+AlwaysPreTouch to avoid memory commit hiccups");
  }

  if (EpsilonMaxTLABSize < MinTLABSize) {
    log_warning(gc)("EpsilonMaxTLABSize < MinTLABSize, adjusting it to " SIZE_FORMAT, MinTLABSize);
    EpsilonMaxTLABSize = MinTLABSize;
  }

  if (!EpsilonElasticTLAB && EpsilonElasticTLABDecay) {
    log_warning(gc)("Disabling EpsilonElasticTLABDecay because EpsilonElasticTLAB is disabled");
    FLAG_SET_DEFAULT(EpsilonElasticTLABDecay, false);
  }

#ifdef COMPILER2
  // Enable loop strip mining: there are still non-GC safepoints, no need to make it worse
  if (FLAG_IS_DEFAULT(UseCountedLoopSafepoints)) {
    FLAG_SET_DEFAULT(UseCountedLoopSafepoints, true);
    if (FLAG_IS_DEFAULT(LoopStripMiningIter)) {
      FLAG_SET_DEFAULT(LoopStripMiningIter, 1000);
    }
  }
#endif
}

CollectedHeap* EpsilonArguments::create_heap() {
  return create_heap_with_policy<EpsilonHeap, EpsilonCollectorPolicy>();
}
