/*
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_SERIAL_SERIALHEAP_HPP
#define SHARE_VM_GC_SERIAL_SERIALHEAP_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/serial/defNewGeneration.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/serial/tenuredGeneration.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/genCollectedHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/growableArray.hpp"

class GenCollectorPolicy;
class GCMemoryManager;
class MemoryPool;
class TenuredGeneration;

class SerialHeap : public GenCollectedHeap {
private:
  MemoryPool* _eden_pool;
  MemoryPool* _survivor_pool;
  MemoryPool* _old_pool;

  virtual void initialize_serviceability();

public:
  static SerialHeap* heap();

  SerialHeap(GenCollectorPolicy* policy);

  virtual Name kind() const {
    return CollectedHeap::Serial;
  }

  virtual const char* name() const {
    return "Serial";
  }

  virtual GrowableArray<GCMemoryManager*> memory_managers();
  virtual GrowableArray<MemoryPool*> memory_pools();

  // override
  virtual bool is_in_closed_subset(const void* p) const {
    return is_in(p);
  }

  DefNewGeneration* young_gen() const {
    assert(_young_gen->kind() == Generation::DefNew, "Wrong generation type");
    return static_cast<DefNewGeneration*>(_young_gen);
  }

  TenuredGeneration* old_gen() const {
    assert(_old_gen->kind() == Generation::MarkSweepCompact, "Wrong generation type");
    return static_cast<TenuredGeneration*>(_old_gen);
  }

  // Apply "cur->do_oop" or "older->do_oop" to all the oops in objects
  // allocated since the last call to save_marks in the young generation.
  // The "cur" closure is applied to references in the younger generation
  // at "level", and the "older" closure to older generations.
  template <typename OopClosureType1, typename OopClosureType2>
  void oop_since_save_marks_iterate(OopClosureType1* cur,
                                    OopClosureType2* older);

};

#endif // SHARE_VM_GC_CMS_CMSHEAP_HPP
