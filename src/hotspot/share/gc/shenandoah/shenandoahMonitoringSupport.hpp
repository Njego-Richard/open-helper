/*
 * Copyright (c) 2015, 2017, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHMONITORINGSUPPORT_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHMONITORINGSUPPORT_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"

class GenerationCounters;
class HSpaceCounters;
class ShenandoahHeap;
class CollectorCounters;
class ShenandoahHeapRegionCounters;

class ShenandoahMonitoringSupport : public CHeapObj<mtGC> {
private:
  CollectorCounters*   _partial_counters;
  CollectorCounters*   _full_counters;

  GenerationCounters* _young_counters;
  GenerationCounters* _heap_counters;

  HSpaceCounters* _space_counters;

  ShenandoahHeapRegionCounters* _heap_region_counters;

public:
 ShenandoahMonitoringSupport(ShenandoahHeap* heap);
 CollectorCounters* stw_collection_counters();
 CollectorCounters* full_stw_collection_counters();
 CollectorCounters* concurrent_collection_counters();
 CollectorCounters* partial_collection_counters();
 void update_counters();
};

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHMONITORINGSUPPORT_HPP
