/*
 * Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.
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
#include "classfile/javaClasses.inline.hpp"
#include "jfr/leakprofiler/chains/edge.hpp"
#include "jfr/leakprofiler/utilities/unifiedOop.hpp"

Edge::Edge() : _parent(NULL), _reference(NULL) {}

Edge::Edge(const Edge* parent, const oop* reference) : _parent(parent),
                                                       _reference(reference) {}

const oop Edge::pointee() const {
  return UnifiedOop::dereference(_reference);
}

const oop Edge::reference_owner() const {
  return is_root() ? (oop)NULL : _parent->pointee();
}

size_t Edge::distance_to_root() const {
  size_t depth = 0;
  const Edge* current = _parent;
  while (current != NULL) {
    depth++;
    current = current->parent();
  }
  return depth;
}
