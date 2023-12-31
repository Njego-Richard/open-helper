/*
 * Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.
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
 */

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/symbolTable.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceKlass.hpp"
#include "unittest.hpp"

// Tests InstanceKlass::package_from_name()
TEST_VM(InstanceKlass, null_symbol) {
  ResourceMark rm;
  TempNewSymbol package_sym = InstanceKlass::package_from_name(NULL, NULL);
  ASSERT_TRUE(package_sym == NULL) << "Wrong package for NULL symbol";
}

// Tests for InstanceKlass::is_class_loader_instance_klass() function
TEST_VM(InstanceKlass, class_loader_class) {
  InstanceKlass* klass = SystemDictionary::ClassLoader_klass();
  ASSERT_TRUE(klass->is_class_loader_instance_klass());
}

TEST_VM(InstanceKlass, string_klass) {
  InstanceKlass* klass = SystemDictionary::String_klass();
  ASSERT_TRUE(!klass->is_class_loader_instance_klass());
}
