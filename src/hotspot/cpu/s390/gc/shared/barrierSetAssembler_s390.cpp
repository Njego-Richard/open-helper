/*
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2018, SAP SE. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/barrierSetAssembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/interp_masm.hpp"

#define __ masm->

void BarrierSetAssembler::arraycopy_epilogue(MacroAssembler* masm, DecoratorSet decorators, BasicType type,
                                             Register dst, Register count, bool do_return) {
  if (do_return) { __ z_br(Z_R14); }
}

void BarrierSetAssembler::load_at(MacroAssembler* masm, DecoratorSet decorators, BasicType type,
                                  const Address& addr, Register dst, Register tmp1, Register tmp2, Label *L_handle_null) {
  bool in_heap = (decorators & IN_HEAP) != 0;
  bool in_native = (decorators & IN_NATIVE) != 0;
  bool not_null = (decorators & IS_NOT_NULL) != 0;
  assert(in_heap || in_native, "where?");

  switch (type) {
  case T_ARRAY:
  case T_OBJECT: {
    if (UseCompressedOops && in_heap) {
      __ z_llgf(dst, addr);
      if (L_handle_null != NULL) { // Label provided.
        __ compareU32_and_branch(dst, (intptr_t)0, Assembler::bcondEqual, *L_handle_null);
        __ oop_decoder(dst, dst, false);
      } else {
        __ oop_decoder(dst, dst, !not_null);
      }
    } else {
      __ z_lg(dst, addr);
      if (L_handle_null != NULL) {
        __ compareU64_and_branch(dst, (intptr_t)0, Assembler::bcondEqual, *L_handle_null);
      }
    }
    break;
  }
  default: Unimplemented();
  }
}

void BarrierSetAssembler::store_at(MacroAssembler* masm, DecoratorSet decorators, BasicType type,
                                   const Address& addr, Register val, Register tmp1, Register tmp2, Register tmp3) {
  bool in_heap = (decorators & IN_HEAP) != 0;
  bool in_native = (decorators & IN_NATIVE) != 0;
  bool not_null = (decorators & IS_NOT_NULL) != 0;
  assert(in_heap || in_native, "where?");
  assert_different_registers(val, tmp1, tmp2);

  switch (type) {
  case T_ARRAY:
  case T_OBJECT: {
    if (UseCompressedOops && in_heap) {
      if (val == noreg) {
        __ clear_mem(addr, 4);
      } else if (Universe::narrow_oop_mode() == Universe::UnscaledNarrowOop) {
        __ z_st(val, addr);
      } else {
        Register tmp = (tmp1 != Z_R1) ? tmp1 : tmp2; // Avoid tmp == Z_R1 (see oop_encoder).
        __ oop_encoder(tmp, val, !not_null);
        __ z_st(tmp, addr);
      }
    } else {
      if (val == noreg) {
        __ clear_mem(addr, 8);
      } else {
        __ z_stg(val, addr);
      }
    }
    break;
  }
  default: Unimplemented();
  }
}

void BarrierSetAssembler::resolve_jobject(MacroAssembler* masm, Register value, Register tmp1, Register tmp2) {
  NearLabel Ldone;
  __ z_ltgr(tmp1, value);
  __ z_bre(Ldone);          // Use NULL result as-is.

  __ z_nill(value, ~JNIHandles::weak_tag_mask);
  __ z_lg(value, 0, value); // Resolve (untagged) jobject.

  __ verify_oop(value);
  __ bind(Ldone);
}
