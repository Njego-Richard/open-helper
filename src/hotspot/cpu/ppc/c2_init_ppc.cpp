/*
 * Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, 2018 SAP SE. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/compile.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/node.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"

// Processor dependent initialization of C2 compiler for ppc.

void Compile::pd_compiler2_init() {

  // Power7 and later.
  if (PowerArchitecturePPC64 > 6) {
    if (FLAG_IS_DEFAULT(UsePopCountInstruction)) {
      FLAG_SET_ERGO(bool, UsePopCountInstruction, true);
    }
  }

  if (PowerArchitecturePPC64 == 6) {
    if (FLAG_IS_DEFAULT(InsertEndGroupPPC64)) {
      FLAG_SET_ERGO(bool, InsertEndGroupPPC64, true);
    }
  }

  if (!VM_Version::has_isel() && FLAG_IS_DEFAULT(ConditionalMoveLimit)) {
    FLAG_SET_ERGO(intx, ConditionalMoveLimit, 0);
  }

  if (OptimizeFill) {
    warning("OptimizeFill is not supported on this CPU.");
    FLAG_SET_DEFAULT(OptimizeFill, false);
  }

}
