
/*
 * Copyright (c) 2004, 2015, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_PARALLEL_PSGENERATIONCOUNTERS_HPP
#define SHARE_VM_GC_PARALLEL_PSGENERATIONCOUNTERS_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psVirtualspace.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/generationCounters.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/perfData.hpp"

// A PSGenerationCounter is a holder class for performance counters
// that track a generation

class PSGenerationCounters: public GenerationCounters {
  friend class VMStructs;

 private:
  PSVirtualSpace*      _ps_virtual_space;

 public:
  PSGenerationCounters(const char* name, int ordinal, int spaces,
                       size_t min_capacity, size_t max_capacity, PSVirtualSpace* v);

  void update_all() {
    assert(_virtual_space == NULL, "Only one should be in use");
    _current_size->set_value(_ps_virtual_space->committed_size());
  }
};

#endif // SHARE_VM_GC_PARALLEL_PSGENERATIONCOUNTERS_HPP
