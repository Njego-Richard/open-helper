/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_CODE_COMPILEDMETHOD_INLINE_HPP
#define SHARE_VM_CODE_COMPILEDMETHOD_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/compiledMethod.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/nativeInst.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/frame.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/orderAccess.hpp"

inline bool CompiledMethod::is_deopt_pc(address pc) { return is_deopt_entry(pc) || is_deopt_mh_entry(pc); }

// When using JVMCI the address might be off by the size of a call instruction.
inline bool CompiledMethod::is_deopt_entry(address pc) {
  return pc == deopt_handler_begin()
#if INCLUDE_JVMCI
    || (is_compiled_by_jvmci() && pc == (deopt_handler_begin() + NativeCall::instruction_size))
#endif
    ;
}

inline void CompiledMethod::release_set_exception_cache(ExceptionCache *ec) {
  OrderAccess::release_store(&_exception_cache, ec);
}

// -----------------------------------------------------------------------------
// CompiledMethod::get_deopt_original_pc
//
// Return the original PC for the given PC if:
// (a) the given PC belongs to a nmethod and
// (b) it is a deopt PC

inline address CompiledMethod::get_deopt_original_pc(const frame* fr) {
  if (fr->cb() == NULL)  return NULL;

  CompiledMethod* cm = fr->cb()->as_compiled_method_or_null();
  if (cm != NULL && cm->is_deopt_pc(fr->pc()))
    return cm->get_original_pc(fr);

  return NULL;
}


// class ExceptionCache methods

inline int ExceptionCache::count() { return OrderAccess::load_acquire(&_count); }

address ExceptionCache::pc_at(int index) {
  assert(index >= 0 && index < count(),"");
  return _pc[index];
}

address ExceptionCache::handler_at(int index) {
  assert(index >= 0 && index < count(),"");
  return _handler[index];
}

// increment_count is only called under lock, but there may be concurrent readers.
inline void ExceptionCache::increment_count() { OrderAccess::release_store(&_count, _count + 1); }


#endif //SHARE_VM_CODE_COMPILEDMETHOD_INLINE_HPP
