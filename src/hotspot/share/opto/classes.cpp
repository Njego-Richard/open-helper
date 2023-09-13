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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/addnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/arraycopynode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/callnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/castnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/cfgnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/connode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/convertnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/countbitsnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/divnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/intrinsicnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/locknode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/loopnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/machnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/memnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/mathexactnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/movenode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/mulnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/multnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/narrowptrnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/node.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/opaquenode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/rootnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/subnode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/vectornode.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"
#if INCLUDE_ZGC
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/c2/zBarrierSetC2.hpp"
#endif
#if INCLUDE_SHENANDOAHGC
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/c2/shenandoahBarrierSetC2.hpp"
#endif

// ----------------------------------------------------------------------------
// Build a table of virtual functions to map from Nodes to dense integer
// opcode names.
int Node::Opcode() const { return Op_Node; }
#define macro(x) int x##Node::Opcode() const { return Op_##x; }
#define optionalmacro(x)
#include "classes.hpp"
#undef macro
#undef optionalmacro
