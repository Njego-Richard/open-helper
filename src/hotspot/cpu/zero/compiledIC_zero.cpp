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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/codeCache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/compiledIC.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/icBuffer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/nmethod.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/vtableStubs.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/interpreter.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/linkResolver.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/metadataFactory.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/oopFactory.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/method.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/symbol.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/icache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/sharedRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/stubRoutines.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/events.hpp"


// ----------------------------------------------------------------------------

address CompiledStaticCall::emit_to_interp_stub(CodeBuffer &cbuf, address mark) {
  ShouldNotReachHere(); // Only needed for COMPILER2.
  return NULL;
}

int CompiledStaticCall::to_interp_stub_size() {
  ShouldNotReachHere(); // Only needed for COMPILER2.
  return 0;
}

// Relocation entries for call stub, compiled java to interpreter.
int CompiledStaticCall::reloc_to_interp_stub() {
  ShouldNotReachHere(); // Only needed for COMPILER2.
  return 0;
}

void CompiledDirectStaticCall::set_to_interpreted(const methodHandle& callee, address entry) {
  ShouldNotReachHere(); // Only needed for COMPILER2.
}

void CompiledDirectStaticCall::set_stub_to_clean(static_stub_Relocation* static_stub) {
  ShouldNotReachHere(); // Only needed for COMPILER2.
}

//-----------------------------------------------------------------------------
// Non-product mode code.
#ifndef PRODUCT

void CompiledDirectStaticCall::verify() {
  ShouldNotReachHere(); // Only needed for COMPILER2.
}

#endif // !PRODUCT
