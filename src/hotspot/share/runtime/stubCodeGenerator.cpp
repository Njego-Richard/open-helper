/*
 * Copyright (c) 1997, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/codeCache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/compiler/disassembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/forte.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/stubCodeGenerator.hpp"


// Implementation of StubCodeDesc

StubCodeDesc* StubCodeDesc::_list = NULL;
bool          StubCodeDesc::_frozen = false;

StubCodeDesc* StubCodeDesc::desc_for(address pc) {
  StubCodeDesc* p = _list;
  while (p != NULL && !p->contains(pc)) {
    p = p->_next;
  }
  return p;
}

const char* StubCodeDesc::name_for(address pc) {
  StubCodeDesc* p = desc_for(pc);
  return p == NULL ? NULL : p->name();
}


void StubCodeDesc::freeze() {
  assert(!_frozen, "repeated freeze operation");
  _frozen = true;
}

void StubCodeDesc::print_on(outputStream* st) const {
  st->print("%s", group());
  st->print("::");
  st->print("%s", name());
  st->print(" [" INTPTR_FORMAT ", " INTPTR_FORMAT "[ (%d bytes)", p2i(begin()), p2i(end()), size_in_bytes());
}

// Implementation of StubCodeGenerator

StubCodeGenerator::StubCodeGenerator(CodeBuffer* code, bool print_code) {
  _masm = new MacroAssembler(code );
  _print_code = PrintStubCode || print_code;
}

StubCodeGenerator::~StubCodeGenerator() {
  if (PRODUCT_ONLY(_print_code) NOT_PRODUCT(true)) {
    CodeBuffer* cbuf = _masm->code();
    CodeBlob*   blob = CodeCache::find_blob_unsafe(cbuf->insts()->start());
    if (blob != NULL) {
      blob->set_strings(cbuf->strings());
    }
  }
}

void StubCodeGenerator::stub_prolog(StubCodeDesc* cdesc) {
  // default implementation - do nothing
}

void StubCodeGenerator::stub_epilog(StubCodeDesc* cdesc) {
  if (_print_code) {
    CodeStrings cs;
    ptrdiff_t offset = 0;
#ifndef PRODUCT
    // Find the code strings in the outer CodeBuffer.
    CodeBuffer *outer_cbuf = _masm->code_section()->outer();
    cs = outer_cbuf->strings();
    // The offset from the start of the outer CodeBuffer to the start
    // of this stub.
    offset = cdesc->begin() - outer_cbuf->insts()->start();
#endif
    cdesc->print();
    tty->cr();
    Disassembler::decode(cdesc->begin(), cdesc->end(), NULL, cs, offset);
    tty->cr();
  }
}


// Implementation of CodeMark

StubCodeMark::StubCodeMark(StubCodeGenerator* cgen, const char* group, const char* name) {
  _cgen  = cgen;
  _cdesc = new StubCodeDesc(group, name, _cgen->assembler()->pc());
  _cgen->stub_prolog(_cdesc);
  // define the stub's beginning (= entry point) to be after the prolog:
  _cdesc->set_begin(_cgen->assembler()->pc());
}

StubCodeMark::~StubCodeMark() {
  _cgen->assembler()->flush();
  _cdesc->set_end(_cgen->assembler()->pc());
  assert(StubCodeDesc::_list == _cdesc, "expected order on list");
  _cgen->stub_epilog(_cdesc);
  Forte::register_stub(_cdesc->name(), _cdesc->begin(), _cdesc->end());

  if (JvmtiExport::should_post_dynamic_code_generated()) {
    JvmtiExport::post_dynamic_code_generated(_cdesc->name(), _cdesc->begin(), _cdesc->end());
  }
}
