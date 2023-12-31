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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/method.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/privilegedStack.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vframe.inline.hpp"

void PrivilegedElement::initialize(vframeStream* vfst, oop context, PrivilegedElement* next, TRAPS) {
  Method* method        = vfst->method();
  _klass                = method->method_holder();
  _privileged_context   = context;
#ifdef CHECK_UNHANDLED_OOPS
  THREAD->allow_unhandled_oop(&_privileged_context);
#endif // CHECK_UNHANDLED_OOPS
  _frame_id             = vfst->frame_id();
  _next                 = next;
  assert(oopDesc::is_oop_or_null(_privileged_context), "must be an oop");
  assert(oopDesc::is_oop_or_null(protection_domain()), "must be an oop");
}

void PrivilegedElement::oops_do(OopClosure* f) {
  PrivilegedElement *cur = this;
  do {
    f->do_oop((oop*) &cur->_privileged_context);
    cur = cur->_next;
  } while(cur != NULL);
}

//-------------------------------------------------------------------------------
#ifndef PRODUCT

void PrivilegedElement::print_on(outputStream* st) const {
  st->print("   " PTR_FORMAT " ", p2i(_frame_id));
  _klass->print_value_on(st);
  if (protection_domain() != NULL) {
    st->print("   ");
    protection_domain()->print_value_on(st);
  }
  st->cr();
}

bool PrivilegedElement::contains(address addr) {
  PrivilegedElement *e = (PrivilegedElement *)addr;
  if (e >= this && e < this+1) return true;

  if (_next != NULL) {
    return _next->contains(addr);
  } else {
    return false;
  }
}

#endif
