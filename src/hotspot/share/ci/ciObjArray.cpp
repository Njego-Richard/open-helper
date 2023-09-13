/*
 * Copyright (c) 1999, 2015, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciNullObject.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciObjArray.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciUtilities.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/objArrayOop.inline.hpp"

// ciObjArray
//
// This class represents an objArrayOop in the HotSpot virtual
// machine.

ciObject* ciObjArray::obj_at(int index) {
  VM_ENTRY_MARK;
  objArrayOop array = get_objArrayOop();
  if (index < 0 || index >= array->length()) return NULL;
  oop o = array->obj_at(index);
  if (o == NULL) {
    return ciNullObject::make();
  } else {
    return CURRENT_ENV->get_object(o);
  }
}
