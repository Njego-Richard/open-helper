/*
 * Copyright (c) 2002, 2015, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_SERIAL_CSPACECOUNTERS_HPP
#define SHARE_VM_GC_SERIAL_CSPACECOUNTERS_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/generationCounters.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/space.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/perfData.hpp"

// A CSpaceCounters is a holder class for performance counters
// that track a space;

class CSpaceCounters: public CHeapObj<mtGC> {
  friend class VMStructs;

 private:
  PerfVariable*      _capacity;
  PerfVariable*      _used;
  PerfVariable*      _max_capacity;

  // Constant PerfData types don't need to retain a reference.
  // However, it's a good idea to document them here.
  // PerfConstant*     _size;

  ContiguousSpace*     _space;
  char*                _name_space;

 public:

  CSpaceCounters(const char* name, int ordinal, size_t max_size,
                 ContiguousSpace* s, GenerationCounters* gc);

  ~CSpaceCounters();

  virtual void update_capacity();
  virtual void update_used();
  virtual void update_all();

  const char* name_space() const        { return _name_space; }
};

class ContiguousSpaceUsedHelper : public PerfLongSampleHelper {
  private:
    ContiguousSpace* _space;

  public:
    ContiguousSpaceUsedHelper(ContiguousSpace* space) : _space(space) { }

    jlong take_sample();
};

#endif // SHARE_VM_GC_SERIAL_CSPACECOUNTERS_HPP
