/*
 * Copyright (c) 2009, 2012, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciClassList.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciMethodHandle.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/ci/ciUtilities.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/javaClasses.hpp"

// ------------------------------------------------------------------
// ciMethodHandle::get_vmtarget
//
// Return: MH.form -> LF.vmentry -> MN.vmtarget
ciMethod* ciMethodHandle::get_vmtarget() const {
  VM_ENTRY_MARK;
  oop form_oop     = java_lang_invoke_MethodHandle::form(get_oop());
  oop vmentry_oop  = java_lang_invoke_LambdaForm::vmentry(form_oop);
  // FIXME: Share code with ciMemberName::get_vmtarget
  Metadata* vmtarget = java_lang_invoke_MemberName::vmtarget(vmentry_oop);
  if (vmtarget->is_method())
    return CURRENT_ENV->get_method((Method*) vmtarget);
  // FIXME: What if the vmtarget is a Klass?
  assert(false, "");
  return NULL;
}
