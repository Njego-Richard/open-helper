/*
 * Copyright (c) 2007, 2016, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/cms/cmsCollectorPolicy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/cms/parNewGeneration.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/adaptiveSizePolicy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/cardTableRS.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectorPolicy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcLocker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcPolicyCounters.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/generationSpec.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/space.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/vmGCOperations.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/universe.hpp"
#include "oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/arguments.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals_extension.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/java.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vmThread.hpp"

//
// ConcurrentMarkSweepPolicy methods
//

void ConcurrentMarkSweepPolicy::initialize_alignments() {
  _space_alignment = _gen_alignment = (uintx)Generation::GenGrain;
  _heap_alignment = compute_heap_alignment();
}
