/*
 * Copyright (c) 2003, 2018, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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

#ifndef CPU_ZERO_VM_INTERP_MASM_ZERO_HPP
#define CPU_ZERO_VM_INTERP_MASM_ZERO_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/codeBuffer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/invocationCounter.hpp"

// This file specializes the assember with interpreter-specific macros

class InterpreterMacroAssembler : public MacroAssembler {
 public:
  InterpreterMacroAssembler(CodeBuffer* code) : MacroAssembler(code) {}

 public:
  RegisterOrConstant delayed_value_impl(intptr_t* delayed_value_addr,
                                        Register  tmp,
                                        int       offset) {
    ShouldNotCallThis();
    return RegisterOrConstant();
  }
};

#endif // CPU_ZERO_VM_INTERP_MASM_ZERO_HPP
