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
#include "jfr/jfrEvents.hpp"
#include "jvm.h"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"
#include "oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/arguments.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals_extension.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/flags/jvmFlagConstraintList.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/flags/jvmFlagWriteableList.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/sharedRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/defaultStream.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/ostream.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/stringUtils.hpp"
#ifdef COMPILER1
#include "c1/c1_globals.hpp"
#endif
#if INCLUDE_JVMCI
#include "jvmci/jvmci_globals.hpp"
#endif
#ifdef COMPILER2
#include "opto/c2_globals.hpp"
#endif

VM_FLAGS(MATERIALIZE_DEVELOPER_FLAG, \
         MATERIALIZE_PD_DEVELOPER_FLAG, \
         MATERIALIZE_PRODUCT_FLAG, \
         MATERIALIZE_PD_PRODUCT_FLAG, \
         MATERIALIZE_DIAGNOSTIC_FLAG, \
         MATERIALIZE_PD_DIAGNOSTIC_FLAG, \
         MATERIALIZE_EXPERIMENTAL_FLAG, \
         MATERIALIZE_NOTPRODUCT_FLAG, \
         MATERIALIZE_MANAGEABLE_FLAG, \
         MATERIALIZE_PRODUCT_RW_FLAG, \
         MATERIALIZE_LP64_PRODUCT_FLAG, \
         IGNORE_RANGE, \
         IGNORE_CONSTRAINT, \
         IGNORE_WRITEABLE)

RUNTIME_OS_FLAGS(MATERIALIZE_DEVELOPER_FLAG, \
                 MATERIALIZE_PD_DEVELOPER_FLAG, \
                 MATERIALIZE_PRODUCT_FLAG, \
                 MATERIALIZE_PD_PRODUCT_FLAG, \
                 MATERIALIZE_DIAGNOSTIC_FLAG, \
                 MATERIALIZE_PD_DIAGNOSTIC_FLAG, \
                 MATERIALIZE_NOTPRODUCT_FLAG, \
                 IGNORE_RANGE, \
                 IGNORE_CONSTRAINT, \
                 IGNORE_WRITEABLE)

ARCH_FLAGS(MATERIALIZE_DEVELOPER_FLAG, \
           MATERIALIZE_PRODUCT_FLAG, \
           MATERIALIZE_DIAGNOSTIC_FLAG, \
           MATERIALIZE_EXPERIMENTAL_FLAG, \
           MATERIALIZE_NOTPRODUCT_FLAG, \
           IGNORE_RANGE, \
           IGNORE_CONSTRAINT, \
           IGNORE_WRITEABLE)

MATERIALIZE_FLAGS_EXT
