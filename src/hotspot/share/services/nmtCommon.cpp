/*
 * Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/services/nmtCommon.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"

const char* NMTUtil::_memory_type_names[] = {
  "Java Heap",
  "Class",
  "Thread",
  "Thread Stack",
  "Code",
  "GC",
  "Compiler",
  "Internal",
  "Other",
  "Symbol",
  "Native Memory Tracking",
  "Shared class space",
  "Arena Chunk",
  "Test",
  "Tracing",
  "Logging",
  "Arguments",
  "Module",
  "Synchronizer",
  "Safepoint",
  "Unknown"
};


const char* NMTUtil::scale_name(size_t scale) {
  switch(scale) {
    case 1: return "";
    case K: return "KB";
    case M: return "MB";
    case G: return "GB";
  }
  ShouldNotReachHere();
  return NULL;
}

size_t NMTUtil::scale_from_name(const char* scale) {
  assert(scale != NULL, "Null pointer check");
  if (strcasecmp(scale, "1") == 0 || strcasecmp(scale, "b") == 0) {
    return 1;
  } else if (strcasecmp(scale, "kb") == 0 || strcasecmp(scale, "k") == 0) {
    return K;
  } else if (strcasecmp(scale, "mb") == 0 || strcasecmp(scale, "m") == 0) {
    return M;
  } else if (strcasecmp(scale, "gb") == 0 || strcasecmp(scale, "g") == 0) {
    return G;
  } else {
    return 0; // Invalid value
  }
  return K;
}

