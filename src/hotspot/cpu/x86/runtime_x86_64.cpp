/*
 * Copyright (c) 2003, 2010, Oracle and/or its affiliates. All rights reserved.
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
#ifdef COMPILER2
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/asm/macroAssembler.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/vmreg.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/interpreter.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/opto/runtime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/sharedRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/stubRoutines.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vframeArray.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"
#include "vmreg_x86.inline.hpp"
#endif


// This file should really contain the code for generating the OptoRuntime
// exception_blob. However that code uses SimpleRuntimeFrame which only
// exists in sharedRuntime_x86_64.cpp. When there is a sharedRuntime_<arch>.hpp
// file and SimpleRuntimeFrame is able to move there then the exception_blob
// code will move here where it belongs.
