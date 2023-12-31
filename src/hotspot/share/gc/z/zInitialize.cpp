/*
 * Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.
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

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zAddress.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zBarrierSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zCPU.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zGlobals.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zInitialize.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zLargePages.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zNUMA.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zStat.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zTracer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vm_version.hpp"

ZInitialize::ZInitialize(ZBarrierSet* barrier_set) {
  log_info(gc, init)("Initializing %s", ZGCName);
  log_info(gc, init)("Version: %s (%s)",
                     Abstract_VM_Version::vm_release(),
                     Abstract_VM_Version::jdk_debug_level());

  // Early initialization
  ZAddressMasks::initialize();
  ZNUMA::initialize();
  ZCPU::initialize();
  ZStatValue::initialize();
  ZTracer::initialize();
  ZLargePages::initialize();
  ZBarrierSet::set_barrier_set(barrier_set);
}
