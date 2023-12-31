/*
 * Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/classLoaderData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/heapInspection.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/metadataFactory.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/metaspaceClosure.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/oopFactory.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/annotations.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/typeArrayOop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/ostream.hpp"

// Allocate annotations in metadata area
Annotations* Annotations::allocate(ClassLoaderData* loader_data, TRAPS) {
  return new (loader_data, size(), MetaspaceObj::AnnotationsType, THREAD) Annotations();
}

// helper
void Annotations::free_contents(ClassLoaderData* loader_data, Array<AnnotationArray*>* p) {
  if (p != NULL) {
    for (int i = 0; i < p->length(); i++) {
      MetadataFactory::free_array<u1>(loader_data, p->at(i));
    }
    MetadataFactory::free_array<AnnotationArray*>(loader_data, p);
  }
}

void Annotations::deallocate_contents(ClassLoaderData* loader_data) {
  if (class_annotations() != NULL) {
    MetadataFactory::free_array<u1>(loader_data, class_annotations());
  }
  free_contents(loader_data, fields_annotations());

  if (class_type_annotations() != NULL) {
    MetadataFactory::free_array<u1>(loader_data, class_type_annotations());
  }
  free_contents(loader_data, fields_type_annotations());
}

// Copy annotations to JVM call or reflection to the java heap.
// The alternative to creating this array and adding to Java heap pressure
// is to have a hashtable of the already created typeArrayOops
typeArrayOop Annotations::make_java_array(AnnotationArray* annotations, TRAPS) {
  if (annotations != NULL) {
    int length = annotations->length();
    typeArrayOop copy = oopFactory::new_byteArray(length, CHECK_NULL);
    for (int i = 0; i< length; i++) {
      copy->byte_at_put(i, annotations->at(i));
    }
    return copy;
  } else {
    return NULL;
  }
}

void Annotations::metaspace_pointers_do(MetaspaceClosure* it) {
  log_trace(cds)("Iter(Annotations): %p", this);
  it->push(&_class_annotations);
  it->push(&_fields_annotations);
  it->push(&_class_type_annotations);
  it->push(&_fields_type_annotations); // FIXME: need a test case where _fields_type_annotations != NULL
}

void Annotations::print_value_on(outputStream* st) const {
  st->print("Anotations(" INTPTR_FORMAT ")", p2i(this));
}

#if INCLUDE_SERVICES
// Size Statistics

julong Annotations::count_bytes(Array<AnnotationArray*>* p) {
  julong bytes = 0;
  if (p != NULL) {
    for (int i = 0; i < p->length(); i++) {
      bytes += KlassSizeStats::count_array(p->at(i));
    }
    bytes += KlassSizeStats::count_array(p);
  }
  return bytes;
}

void Annotations::collect_statistics(KlassSizeStats *sz) const {
  sz->_annotations_bytes = sz->count(this);
  sz->_class_annotations_bytes = sz->count(class_annotations());
  sz->_class_type_annotations_bytes = sz->count(class_type_annotations());
  sz->_fields_annotations_bytes = count_bytes(fields_annotations());
  sz->_fields_type_annotations_bytes = count_bytes(fields_type_annotations());

  sz->_annotations_bytes +=
      sz->_class_annotations_bytes +
      sz->_class_type_annotations_bytes +
      sz->_fields_annotations_bytes +
      sz->_fields_type_annotations_bytes;

  sz->_ro_bytes += sz->_annotations_bytes;
}
#endif // INCLUDE_SERVICES

#define BULLET  " - "

#ifndef PRODUCT
void Annotations::print_on(outputStream* st) const {
  st->print(BULLET"class_annotations            "); class_annotations()->print_value_on(st);
  st->print(BULLET"fields_annotations           "); fields_annotations()->print_value_on(st);
  st->print(BULLET"class_type_annotations       "); class_type_annotations()->print_value_on(st);
  st->print(BULLET"fields_type_annotations      "); fields_type_annotations()->print_value_on(st);
}
#endif // PRODUCT
