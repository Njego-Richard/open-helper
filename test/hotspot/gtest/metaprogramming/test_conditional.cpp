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

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/metaprogramming/conditional.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/metaprogramming/isSame.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"

class ConditionalTest {
  class A: AllStatic {};
  class B: AllStatic {};

  typedef Conditional<true, A, B>::type A_B_if_true;
  static const bool A_B_if_true_is_A = IsSame<A_B_if_true, A>::value;
  static const bool A_B_if_true_is_B = IsSame<A_B_if_true, B>::value;
  STATIC_ASSERT(A_B_if_true_is_A);
  STATIC_ASSERT(!A_B_if_true_is_B);

  typedef Conditional<false, A, B>::type A_B_if_false;
  static const bool A_B_if_false_is_A = IsSame<A_B_if_false, A>::value;
  static const bool A_B_if_false_is_B = IsSame<A_B_if_false, B>::value;
  STATIC_ASSERT(!A_B_if_false_is_A);
  STATIC_ASSERT(A_B_if_false_is_B);
};
