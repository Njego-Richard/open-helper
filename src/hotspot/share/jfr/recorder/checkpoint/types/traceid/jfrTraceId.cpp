/*
 * Copyright (c) 2011, 2021, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/classLoaderData.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/symbolTable.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/recorder/checkpoint/types/traceid/jfrTraceId.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/utilities/jfrTypes.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/arrayKlass.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/klass.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceKlass.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/method.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/atomic.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/orderAccess.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vm_version.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/jniHandles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"

 // returns updated value
static traceid atomic_inc(traceid volatile* const dest) {
  assert(VM_Version::supports_cx8(), "invariant");
  traceid compare_value;
  traceid exchange_value;
  do {
    compare_value = OrderAccess::load_acquire(dest);
    exchange_value = compare_value + 1;
  } while (Atomic::cmpxchg(exchange_value, dest, compare_value) != compare_value);
  return exchange_value;
}

static traceid next_class_id() {
  static volatile traceid class_id_counter = MaxJfrEventId + 101; // + 101 is for the void.class primitive
  return atomic_inc(&class_id_counter) << TRACE_ID_SHIFT;
}

static traceid next_thread_id() {
  static volatile traceid thread_id_counter = 0;
  return atomic_inc(&thread_id_counter);
}

static traceid next_module_id() {
  static volatile traceid module_id_counter = 1;
  return atomic_inc(&module_id_counter) << TRACE_ID_SHIFT;
}

static traceid next_package_id() {
  static volatile traceid package_id_counter = 1;
  return atomic_inc(&package_id_counter) << TRACE_ID_SHIFT;
}

static traceid next_class_loader_data_id() {
  static volatile traceid cld_id_counter = 1;
  return atomic_inc(&cld_id_counter) << TRACE_ID_SHIFT;
}

static bool found_jdk_internal_event_klass = false;
static bool found_jdk_jfr_event_klass = false;

static void check_klass(const Klass* klass) {
  assert(klass != NULL, "invariant");
  if (found_jdk_internal_event_klass && found_jdk_jfr_event_klass) {
    return;
  }
  static const Symbol* jdk_internal_event_sym = NULL;
  if (jdk_internal_event_sym == NULL) {
    // setup when loading the first TypeArrayKlass (Universe::genesis) hence single threaded invariant
    jdk_internal_event_sym = SymbolTable::new_permanent_symbol("jdk/internal/event/Event", Thread::current());
  }
  assert(jdk_internal_event_sym != NULL, "invariant");

  static const Symbol* jdk_jfr_event_sym = NULL;
  if (jdk_jfr_event_sym == NULL) {
    // setup when loading the first TypeArrayKlass (Universe::genesis) hence single threaded invariant
    jdk_jfr_event_sym = SymbolTable::new_permanent_symbol("jdk/jfr/Event", Thread::current());
  }
  assert(jdk_jfr_event_sym != NULL, "invariant");
  const Symbol* const klass_name = klass->name();

  if (!found_jdk_internal_event_klass) {
    if (jdk_internal_event_sym == klass_name && klass->class_loader() == NULL) {
      found_jdk_internal_event_klass = true;
      JfrTraceId::tag_as_jdk_jfr_event(klass);
      return;
    }
  }

  if (!found_jdk_jfr_event_klass) {
    if (jdk_jfr_event_sym == klass_name && klass->class_loader() == NULL) {
      found_jdk_jfr_event_klass = true;
      JfrTraceId::tag_as_jdk_jfr_event(klass);
      return;
    }
  }
}

void JfrTraceId::assign(const Klass* klass) {
  assert(klass != NULL, "invariant");
  klass->set_trace_id(next_class_id());
  check_klass(klass);
  const Klass* const super = klass->super();
  if (super == NULL) {
    return;
  }
  if (IS_EVENT_KLASS(super)) {
    tag_as_jdk_jfr_event_sub(klass);
  }
}

void JfrTraceId::assign(const ModuleEntry* module) {
  assert(module != NULL, "invariant");
  module->set_trace_id(next_module_id());
}

void JfrTraceId::assign(const PackageEntry* package) {
  assert(package != NULL, "invariant");
  package->set_trace_id(next_package_id());
}

void JfrTraceId::assign(const ClassLoaderData* cld) {
  assert(cld != NULL, "invariant");
  if (cld->is_anonymous()) {
    cld->set_trace_id(0);
    return;
  }
  cld->set_trace_id(next_class_loader_data_id());
}

traceid JfrTraceId::assign_primitive_klass_id() {
  return next_class_id();
}

traceid JfrTraceId::assign_thread_id() {
  return next_thread_id();
}

// used by CDS / APPCDS as part of "remove_unshareable_info"
void JfrTraceId::remove(const Klass* k) {
  assert(k != NULL, "invariant");
  // Mask off and store the event flags.
  // This mechanism will retain the event specific flags
  // in the archive, allowing for event flag restoration
  // when renewing the traceid on klass revival.
  k->set_trace_id(EVENT_KLASS_MASK(k));
}

// used by CDS / APPCDS as part of "restore_unshareable_info"
void JfrTraceId::restore(const Klass* k) {
  assert(k != NULL, "invariant");
  if (IS_JDK_JFR_EVENT_KLASS(k)) {
    found_jdk_jfr_event_klass = true;
  }
  const traceid event_flags = k->trace_id();
  // get a fresh traceid and restore the original event flags
  k->set_trace_id(next_class_id() | event_flags);
  if (k->is_typeArray_klass()) {
    // the next id is reserved for the corresponding primitive class
    next_class_id();
  }
}

traceid JfrTraceId::get(jclass jc) {
  assert(jc != NULL, "invariant");
  assert(((JavaThread*)Thread::current())->thread_state() == _thread_in_vm, "invariant");
  const oop my_oop = JNIHandles::resolve(jc);
  assert(my_oop != NULL, "invariant");
  return get(java_lang_Class::as_Klass(my_oop));
}

// A mirror representing a primitive class (e.g. int.class) has no reified Klass*,
// instead it has an associated TypeArrayKlass* (e.g. int[].class).
// We can use the TypeArrayKlass* as a proxy for deriving the id of the primitive class.
// The exception is the void.class, which has neither a Klass* nor a TypeArrayKlass*.
// It will use a reserved constant.
static traceid load_primitive(const oop mirror) {
  assert(java_lang_Class::is_primitive(mirror), "invariant");
  const Klass* const tak = java_lang_Class::array_klass_acquire(mirror);
  traceid id;
  if (tak == NULL) {
    // The first klass id is reserved for the void.class
    id = MaxJfrEventId + 101;
  } else {
    id = JfrTraceId::get(tak) + 1;
  }
  return id;
}

traceid JfrTraceId::use(jclass jc, bool raw /* false */) {
  assert(jc != NULL, "invariant");
  assert(((JavaThread*)Thread::current())->thread_state() == _thread_in_vm, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  const Klass* const k = java_lang_Class::as_Klass(mirror);
  return k != NULL ? (raw ? get(k) : use(k)) : load_primitive(mirror);
}

bool JfrTraceId::in_visible_set(const jclass jc) {
  assert(jc != NULL, "invariant");
  assert(((JavaThread*)Thread::current())->thread_state() == _thread_in_vm, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  return in_visible_set(java_lang_Class::as_Klass(mirror));
}

bool JfrTraceId::in_jdk_jfr_event_hierarchy(const jclass jc) {
  assert(jc != NULL, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  return in_jdk_jfr_event_hierarchy(java_lang_Class::as_Klass(mirror));
}

bool JfrTraceId::is_jdk_jfr_event_sub(const jclass jc) {
  assert(jc != NULL, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  return is_jdk_jfr_event_sub(java_lang_Class::as_Klass(mirror));
}

bool JfrTraceId::is_jdk_jfr_event(const jclass jc) {
  assert(jc != NULL, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  return is_jdk_jfr_event(java_lang_Class::as_Klass(mirror));
}

bool JfrTraceId::is_event_host(const jclass jc) {
  assert(jc != NULL, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  return is_event_host(java_lang_Class::as_Klass(mirror));
}

void JfrTraceId::tag_as_jdk_jfr_event_sub(const jclass jc) {
  assert(jc != NULL, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  const Klass* const k = java_lang_Class::as_Klass(mirror);
  tag_as_jdk_jfr_event_sub(k);
  assert(IS_JDK_JFR_EVENT_SUBKLASS(k), "invariant");
}

void JfrTraceId::tag_as_event_host(const jclass jc) {
  assert(jc != NULL, "invariant");
  const oop mirror = JNIHandles::resolve(jc);
  assert(mirror != NULL, "invariant");
  const Klass* const k = java_lang_Class::as_Klass(mirror);
  tag_as_event_host(k);
  assert(IS_EVENT_HOST_KLASS(k), "invariant");
}
