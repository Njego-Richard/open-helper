/*
 * Copyright (c) 1997, 2016, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/debugInfoRec.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/nmethod.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/pcDesc.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/scopeDesc.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"

PcDesc::PcDesc(int pc_offset, int scope_decode_offset, int obj_decode_offset) {
  _pc_offset           = pc_offset;
  _scope_decode_offset = scope_decode_offset;
  _obj_decode_offset   = obj_decode_offset;
  _flags               = 0;
}

address PcDesc::real_pc(const CompiledMethod* code) const {
  return code->code_begin() + pc_offset();
}

void PcDesc::print(CompiledMethod* code) {
#ifndef PRODUCT
  ResourceMark rm;
  tty->print_cr("PcDesc(pc=" PTR_FORMAT " offset=%x bits=%x):", p2i(real_pc(code)), pc_offset(), _flags);

  if (scope_decode_offset() == DebugInformationRecorder::serialized_null) {
    return;
  }

  for (ScopeDesc* sd = code->scope_desc_at(real_pc(code));
       sd != NULL;
       sd = sd->sender()) {
    sd->print_on(tty);
  }
#endif
}

bool PcDesc::verify(CompiledMethod* code) {
  //Unimplemented();
  return true;
}
