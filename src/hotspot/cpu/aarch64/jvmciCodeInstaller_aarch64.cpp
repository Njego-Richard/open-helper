/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jvmci/jvmciCodeInstaller.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jvmci/jvmciRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jvmci/jvmciCompilerToVM.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jvmci/jvmciJavaClasses.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/sharedRuntime.hpp"
#include "vmreg_aarch64.inline.hpp"

jint CodeInstaller::pd_next_offset(NativeInstruction* inst, jint pc_offset, Handle method, TRAPS) {
  if (inst->is_call() || inst->is_jump() || inst->is_blr()) {
    return pc_offset + NativeCall::instruction_size;
  } else if (inst->is_general_jump()) {
    return pc_offset + NativeGeneralJump::instruction_size;
  } else if (NativeInstruction::is_adrp_at((address)inst)) {
    // adrp; add; blr
    return pc_offset + 3 * NativeInstruction::instruction_size;
  } else {
    JVMCI_ERROR_0("unsupported type of instruction for call site");
  }
}

void CodeInstaller::pd_patch_OopConstant(int pc_offset, Handle constant, TRAPS) {
  address pc = _instructions->start() + pc_offset;
#ifdef ASSERT
  {
    NativeInstruction *insn = nativeInstruction_at(pc);
    if (HotSpotObjectConstantImpl::compressed(constant)) {
      // Mov narrow constant: movz n << 16, movk
      assert(Instruction_aarch64::extract(insn->encoding(), 31, 21) == 0b11010010101 &&
             nativeInstruction_at(pc+4)->is_movk(), "wrong insn in patch");
    } else {
      // Move wide constant: movz n, movk, movk.
      assert(nativeInstruction_at(pc+4)->is_movk()
             && nativeInstruction_at(pc+8)->is_movk(), "wrong insn in patch");
    }
  }
#endif // ASSERT
  Handle obj(THREAD, HotSpotObjectConstantImpl::object(constant));
  jobject value = JNIHandles::make_local(obj());
  MacroAssembler::patch_oop(pc, (address)obj());
  int oop_index = _oop_recorder->find_index(value);
  RelocationHolder rspec = oop_Relocation::spec(oop_index);
  _instructions->relocate(pc, rspec);
}

void CodeInstaller::pd_patch_MetaspaceConstant(int pc_offset, Handle constant, TRAPS) {
  address pc = _instructions->start() + pc_offset;
  if (HotSpotMetaspaceConstantImpl::compressed(constant)) {
    narrowKlass narrowOop = record_narrow_metadata_reference(_instructions, pc, constant, CHECK);
    MacroAssembler::patch_narrow_klass(pc, narrowOop);
    TRACE_jvmci_3("relocating (narrow metaspace constant) at " PTR_FORMAT "/0x%x", p2i(pc), narrowOop);
  } else {
    NativeMovConstReg* move = nativeMovConstReg_at(pc);
    void* reference = record_metadata_reference(_instructions, pc, constant, CHECK);
    move->set_data((intptr_t) reference);
    TRACE_jvmci_3("relocating (metaspace constant) at " PTR_FORMAT "/" PTR_FORMAT, p2i(pc), p2i(reference));
  }
}

void CodeInstaller::pd_patch_DataSectionReference(int pc_offset, int data_offset, TRAPS) {
  address pc = _instructions->start() + pc_offset;
  NativeInstruction* inst = nativeInstruction_at(pc);
  if (inst->is_adr_aligned() || inst->is_ldr_literal()
      || (NativeInstruction::maybe_cpool_ref(pc))) {
    address dest = _constants->start() + data_offset;
    _instructions->relocate(pc, section_word_Relocation::spec((address) dest, CodeBuffer::SECT_CONSTS));
    TRACE_jvmci_3("relocating at " PTR_FORMAT " (+%d) with destination at %d", p2i(pc), pc_offset, data_offset);
  } else {
    JVMCI_ERROR("unknown load or move instruction at " PTR_FORMAT, p2i(pc));
  }
}

void CodeInstaller::pd_relocate_ForeignCall(NativeInstruction* inst, jlong foreign_call_destination, TRAPS) {
  address pc = (address) inst;
  if (inst->is_call()) {
    NativeCall* call = nativeCall_at(pc);
    call->set_destination((address) foreign_call_destination);
    _instructions->relocate(call->instruction_address(), runtime_call_Relocation::spec());
  } else if (inst->is_jump()) {
    NativeJump* jump = nativeJump_at(pc);
    jump->set_jump_destination((address) foreign_call_destination);
    _instructions->relocate(jump->instruction_address(), runtime_call_Relocation::spec());
  } else if (inst->is_general_jump()) {
    NativeGeneralJump* jump = nativeGeneralJump_at(pc);
    jump->set_jump_destination((address) foreign_call_destination);
    _instructions->relocate(jump->instruction_address(), runtime_call_Relocation::spec());
  } else if (NativeInstruction::is_adrp_at((address)inst)) {
    // adrp; add; blr
    MacroAssembler::pd_patch_instruction_size((address)inst,
                                              (address)foreign_call_destination);
  } else {
    JVMCI_ERROR("unknown call or jump instruction at " PTR_FORMAT, p2i(pc));
  }
  TRACE_jvmci_3("relocating (foreign call) at " PTR_FORMAT, p2i(inst));
}

void CodeInstaller::pd_relocate_JavaMethod(CodeBuffer &cbuf, Handle hotspot_method, jint pc_offset, TRAPS) {
#ifdef ASSERT
  Method* method = NULL;
  // we need to check, this might also be an unresolved method
  if (hotspot_method->is_a(HotSpotResolvedJavaMethodImpl::klass())) {
    method = getMethodFromHotSpotMethod(hotspot_method());
  }
#endif
  switch (_next_call_type) {
    case INLINE_INVOKE:
      break;
    case INVOKEVIRTUAL:
    case INVOKEINTERFACE: {
      assert(method == NULL || !method->is_static(), "cannot call static method with invokeinterface");
      NativeCall* call = nativeCall_at(_instructions->start() + pc_offset);
      _instructions->relocate(call->instruction_address(), virtual_call_Relocation::spec(_invoke_mark_pc));
      call->trampoline_jump(cbuf, SharedRuntime::get_resolve_virtual_call_stub());
      break;
    }
    case INVOKESTATIC: {
      assert(method == NULL || method->is_static(), "cannot call non-static method with invokestatic");
      NativeCall* call = nativeCall_at(_instructions->start() + pc_offset);
      _instructions->relocate(call->instruction_address(), relocInfo::static_call_type);
      call->trampoline_jump(cbuf, SharedRuntime::get_resolve_static_call_stub());
      break;
    }
    case INVOKESPECIAL: {
      assert(method == NULL || !method->is_static(), "cannot call static method with invokespecial");
      NativeCall* call = nativeCall_at(_instructions->start() + pc_offset);
      _instructions->relocate(call->instruction_address(), relocInfo::opt_virtual_call_type);
      call->trampoline_jump(cbuf, SharedRuntime::get_resolve_opt_virtual_call_stub());
      break;
    }
    default:
      JVMCI_ERROR("invalid _next_call_type value");
      break;
  }
}

void CodeInstaller::pd_relocate_poll(address pc, jint mark, TRAPS) {
  switch (mark) {
    case POLL_NEAR:
      JVMCI_ERROR("unimplemented");
      break;
    case POLL_FAR:
      _instructions->relocate(pc, relocInfo::poll_type);
      break;
    case POLL_RETURN_NEAR:
      JVMCI_ERROR("unimplemented");
      break;
    case POLL_RETURN_FAR:
      _instructions->relocate(pc, relocInfo::poll_return_type);
      break;
    default:
      JVMCI_ERROR("invalid mark value");
      break;
  }
}

// convert JVMCI register indices (as used in oop maps) to HotSpot registers
VMReg CodeInstaller::get_hotspot_reg(jint jvmci_reg, TRAPS) {
  if (jvmci_reg < RegisterImpl::number_of_registers) {
    return as_Register(jvmci_reg)->as_VMReg();
  } else {
    jint floatRegisterNumber = jvmci_reg - RegisterImpl::number_of_registers_for_jvmci;
    if (floatRegisterNumber >= 0 && floatRegisterNumber < FloatRegisterImpl::number_of_registers) {
      return as_FloatRegister(floatRegisterNumber)->as_VMReg();
    }
    JVMCI_ERROR_NULL("invalid register number: %d", jvmci_reg);
  }
}

bool CodeInstaller::is_general_purpose_reg(VMReg hotspotRegister) {
  return !hotspotRegister->is_FloatRegister();
}
