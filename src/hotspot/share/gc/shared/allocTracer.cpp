/*
 * Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/allocTracer.hpp"
#include "jfr/jfrEvents.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"
#if INCLUDE_JFR
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/support/jfrAllocationTracer.hpp"
#endif

void AllocTracer::send_allocation_outside_tlab(Klass* klass, HeapWord* obj, size_t alloc_size, Thread* thread) {
  JFR_ONLY(JfrAllocationTracer tracer(obj, alloc_size, thread);)
  EventObjectAllocationOutsideTLAB event;
  if (event.should_commit()) {
    event.set_objectClass(klass);
    event.set_allocationSize(alloc_size);
    event.commit();
  }
}

void AllocTracer::send_allocation_in_new_tlab(Klass* klass, HeapWord* obj, size_t tlab_size, size_t alloc_size, Thread* thread) {
  JFR_ONLY(JfrAllocationTracer tracer(obj, alloc_size, thread);)
  EventObjectAllocationInNewTLAB event;
  if (event.should_commit()) {
    event.set_objectClass(klass);
    event.set_allocationSize(alloc_size);
    event.set_tlabSize(tlab_size);
    event.commit();
  }
}

void AllocTracer::send_allocation_requiring_gc_event(size_t size, uint gcId) {
  EventAllocationRequiringGC event;
  if (event.should_commit()) {
    event.set_gcId(gcId);
    event.set_size(size);
    event.commit();
  }
}
