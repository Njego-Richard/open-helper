/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef OS_CPU_SOLARIS_SPARC_VM_COUNTTRAILINGZEROS_HPP
#define OS_CPU_SOLARIS_SPARC_VM_COUNTTRAILINGZEROS_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"

inline unsigned count_trailing_zeros(uintx x) {
  assert(x != 0, "precondition");
  // Reduce to mask with ones in all positions below the least
  // significant set bit of x, and zeros otherwise.
  uintx rx = (x - 1) & ~x;      // sub x, 1, rx; andn rx, x, rx;
  // Then count the set bits in the reduction.
  uintx result;
  __asm__(" popc %1, %0\n\t" : "=r" (result) : "r" (rx));
  return result;
}

#endif // include guard
