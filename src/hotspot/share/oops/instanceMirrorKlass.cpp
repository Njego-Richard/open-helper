/*
 * Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/javaClasses.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/iterator.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/oopFactory.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceMirrorKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceOop.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/symbol.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"

int InstanceMirrorKlass::_offset_of_static_fields = 0;

int InstanceMirrorKlass::instance_size(Klass* k) {
  if (k != NULL && k->is_instance_klass()) {
    return align_object_size(size_helper() + InstanceKlass::cast(k)->static_field_size());
  }
  return size_helper();
}

instanceOop InstanceMirrorKlass::allocate_instance(Klass* k, TRAPS) {
  // Query before forming handle.
  int size = instance_size(k);
  assert(size > 0, "total object size must be positive: %d", size);

  // Since mirrors can be variable sized because of the static fields, store
  // the size in the mirror itself.
  return (instanceOop)Universe::heap()->class_allocate(this, size, CHECK_NULL);
}

int InstanceMirrorKlass::oop_size(oop obj) const {
  return java_lang_Class::oop_size(obj);
}

int InstanceMirrorKlass::compute_static_oop_field_count(oop obj) {
  Klass* k = java_lang_Class::as_Klass(obj);
  if (k != NULL && k->is_instance_klass()) {
    return InstanceKlass::cast(k)->static_oop_field_count();
  }
  return 0;
}

#if INCLUDE_CDS
void InstanceMirrorKlass::serialize_offsets(SerializeClosure* f) {
  f->do_u4((u4*)&_offset_of_static_fields);
}
#endif
