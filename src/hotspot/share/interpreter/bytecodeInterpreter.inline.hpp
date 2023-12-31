/*
 * Copyright (c) 2002, 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_INTERPRETER_BYTECODEINTERPRETER_INLINE_HPP
#define SHARE_VM_INTERPRETER_BYTECODEINTERPRETER_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/bytecodeInterpreter.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/stubRoutines.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"

// This file holds platform-independent bodies of inline functions for the C++ based interpreter

#ifdef CC_INTERP

#ifdef ASSERT
#define VERIFY_OOP(o_) \
      if (VerifyOops) { \
        assert(oopDesc::is_oop_or_null(oop(o_)), "Expected an oop or NULL at " PTR_FORMAT, p2i(oop(o_))); \
        StubRoutines::_verify_oop_count++;  \
      }
#else
#define VERIFY_OOP(o)
#endif

#ifdef ZERO
# include "bytecodeInterpreter_zero.inline.hpp"
#else
#error "Only Zero Bytecode Interpreter is supported"
#endif

#endif // CC_INTERP

#endif // SHARE_VM_INTERPRETER_BYTECODEINTERPRETER_INLINE_HPP
