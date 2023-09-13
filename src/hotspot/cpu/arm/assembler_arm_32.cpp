/*
 * Copyright (c) 2008, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/assembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/assembler.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciEnv.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/cardTableBarrierSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/interpreter.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/interpreterRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/templateInterpreterGenerator.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/jvm_misc.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/methodHandles.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/biasedLocking.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/interfaceSupport.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/objectMonitor.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/sharedRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/stubRoutines.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/hashtable.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"

#ifdef COMPILER2
// Convert the raw encoding form into the form expected by the
// constructor for Address.
Address Address::make_raw(int base, int index, int scale, int disp, relocInfo::relocType disp_reloc) {
  RelocationHolder rspec = RelocationHolder::none;
  if (disp_reloc != relocInfo::none) {
    rspec = Relocation::spec_simple(disp_reloc);
  }

  Register rindex = as_Register(index);
  if (rindex != PC) {
    assert(disp == 0, "unsupported");
    Address madr(as_Register(base), rindex, lsl, scale);
    madr._rspec = rspec;
    return madr;
  } else {
    assert(scale == 0, "not supported");
    Address madr(as_Register(base), disp);
    madr._rspec = rspec;
    return madr;
  }
}
#endif

void AsmOperand::initialize_rotated_imm(unsigned int imm) {
  for (int shift = 2; shift <= 24; shift += 2) {
    if ((imm & ~(0xff << shift)) == 0) {
      _encoding = 1 << 25 | (32 - shift) << 7 | imm >> shift;
      return;
    }
  }
  assert((imm & 0x0ffffff0) == 0, "too complicated constant: %d (%x)", imm, imm);
  _encoding = 1 << 25 | 4 << 7 | imm >> 28 | imm << 4;
}

bool AsmOperand::is_rotated_imm(unsigned int imm) {
  if ((imm >> 8) == 0) {
    return true;
  }
  for (int shift = 2; shift <= 24; shift += 2) {
    if ((imm & ~(0xff << shift)) == 0) {
      return true;
    }
  }
  if ((imm & 0x0ffffff0) == 0) {
    return true;
  }
  return false;
}
