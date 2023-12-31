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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/assembler.inline.hpp"
#include "assembler_arm.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/relocInfo.hpp"
#include "nativeInst_arm.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/compressedOops.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/orderAccess.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/safepoint.hpp"

void Relocation::pd_set_data_value(address x, intptr_t o, bool verify_only) {

  NativeMovConstReg* ni = nativeMovConstReg_at(addr());
#if defined(AARCH64) && defined(COMPILER2)
  if (ni->is_movz()) {
    assert(type() == relocInfo::oop_type, "!");
    if (verify_only) {
      uintptr_t d = ni->data();
      guarantee((d >> 32) == 0, "not narrow oop");
      narrowOop no = d;
      oop o = CompressedOops::decode(no);
      guarantee(cast_from_oop<intptr_t>(o) == (intptr_t)x, "instructions must match");
    } else {
      ni->set_data((intptr_t)x);
    }
    return;
  }
#endif
  if (verify_only) {
    guarantee(ni->data() == (intptr_t)(x + o), "instructions must match");
  } else {
    ni->set_data((intptr_t)(x + o));
  }
}

address Relocation::pd_call_destination(address orig_addr) {
  address pc = addr();

  int adj = 0;
  if (orig_addr != NULL) {
    // We just moved this call instruction from orig_addr to addr().
    // This means that, when relative, its target will appear to have grown by addr() - orig_addr.
    adj = orig_addr - pc;
  }

  RawNativeInstruction* ni = rawNativeInstruction_at(pc);

#if (!defined(AARCH64))
  if (NOT_AARCH64(ni->is_add_lr()) AARCH64_ONLY(ni->is_adr_aligned_lr())) {
    // On arm32, skip the optional 'add LR, PC, #offset'
    // (Allowing the jump support code to handle fat_call)
    pc = ni->next_raw_instruction_address();
    ni = nativeInstruction_at(pc);
  }
#endif

  if (AARCH64_ONLY(ni->is_call()) NOT_AARCH64(ni->is_bl())) {
    // For arm32, fat_call are handled by is_jump for the new 'ni',
    // requiring only to support is_bl.
    //
    // For AARCH64, skipping a leading adr is not sufficient
    // to reduce calls to a simple bl.
    return rawNativeCall_at(pc)->destination(adj);
  }

  if (ni->is_jump()) {
    return rawNativeJump_at(pc)->jump_destination(adj);
  }
  ShouldNotReachHere();
  return NULL;
}

void Relocation::pd_set_call_destination(address x) {
  address pc = addr();
  NativeInstruction* ni = nativeInstruction_at(pc);

#if (!defined(AARCH64))
  if (NOT_AARCH64(ni->is_add_lr()) AARCH64_ONLY(ni->is_adr_aligned_lr())) {
    // On arm32, skip the optional 'add LR, PC, #offset'
    // (Allowing the jump support code to handle fat_call)
    pc = ni->next_raw_instruction_address();
    ni = nativeInstruction_at(pc);
  }
#endif

  if (AARCH64_ONLY(ni->is_call()) NOT_AARCH64(ni->is_bl())) {
    // For arm32, fat_call are handled by is_jump for the new 'ni',
    // requiring only to support is_bl.
    //
    // For AARCH64, skipping a leading adr is not sufficient
    // to reduce calls to a simple bl.
    rawNativeCall_at(pc)->set_destination(x);
    return;
  }

  if (ni->is_jump()) { // raw jump
    rawNativeJump_at(pc)->set_jump_destination(x);
    return;
  }
  ShouldNotReachHere();
}


address* Relocation::pd_address_in_code() {
  return (address*)addr();
}

address Relocation::pd_get_address_from_code() {
  return *pd_address_in_code();
}

void poll_Relocation::fix_relocation_after_move(const CodeBuffer* src, CodeBuffer* dest) {
}

void metadata_Relocation::pd_fix_value(address x) {
  assert(! addr_in_const(), "Do not use");
#ifdef AARCH64
#ifdef COMPILER2
  NativeMovConstReg* ni = nativeMovConstReg_at(addr());
  if (ni->is_mov_slow()) {
    return;
  }
#endif
  set_value(x);
#else
  if (!VM_Version::supports_movw()) {
    set_value(x);
#ifdef ASSERT
  } else {
    // the movw/movt data should be correct
    NativeMovConstReg* ni = nativeMovConstReg_at(addr());
    assert(ni->is_movw(), "not a movw");
    // The following assert should be correct but the shared code
    // currently 'fixes' the metadata instructions before the
    // metadata_table is copied in the new method (see
    // JDK-8042845). This means that 'x' (which comes from the table)
    // does not match the value inlined in the code (which is
    // correct). Failure can be temporarily ignored since the code is
    // correct and the table is copied shortly afterward.
    //
    // assert(ni->data() == (int)x, "metadata relocation mismatch");
#endif
  }
#endif // !AARCH64
}
