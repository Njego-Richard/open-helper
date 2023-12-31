/*
 * Copyright (c) 2019, Red Hat, Inc. All rights reserved.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/heuristics/shenandoahAdaptiveHeuristics.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/heuristics/shenandoahAggressiveHeuristics.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/heuristics/shenandoahCompactHeuristics.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/heuristics/shenandoahStaticHeuristics.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/mode/shenandoahSATBMode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/logTag.hpp"

void ShenandoahSATBMode::initialize_flags() const {
  SHENANDOAH_ERGO_ENABLE_FLAG(ExplicitGCInvokesConcurrent);
  SHENANDOAH_ERGO_ENABLE_FLAG(ShenandoahImplicitGCInvokesConcurrent);

  // Final configuration checks
  SHENANDOAH_CHECK_FLAG_SET(ShenandoahLoadRefBarrier);
  SHENANDOAH_CHECK_FLAG_UNSET(ShenandoahIUBarrier);
  SHENANDOAH_CHECK_FLAG_SET(ShenandoahSATBBarrier);
  SHENANDOAH_CHECK_FLAG_SET(ShenandoahCASBarrier);
  SHENANDOAH_CHECK_FLAG_SET(ShenandoahCloneBarrier);
}

ShenandoahHeuristics* ShenandoahSATBMode::initialize_heuristics() const {
  if (ShenandoahGCHeuristics != NULL) {
    if (strcmp(ShenandoahGCHeuristics, "aggressive") == 0) {
      return new ShenandoahAggressiveHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "static") == 0) {
      return new ShenandoahStaticHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "adaptive") == 0) {
      return new ShenandoahAdaptiveHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "compact") == 0) {
      return new ShenandoahCompactHeuristics();
    } else {
      vm_exit_during_initialization("Unknown -XX:ShenandoahGCHeuristics option");
    }
  }
  ShouldNotReachHere();
  return NULL;
}
