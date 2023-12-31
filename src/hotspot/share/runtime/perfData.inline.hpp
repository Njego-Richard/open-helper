/*
 * Copyright (c) 2002, 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_RUNTIME_PERFDATA_INLINE_HPP
#define SHARE_VM_RUNTIME_PERFDATA_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/perfData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/growableArray.hpp"

inline int PerfDataList::length() {
  return _set->length();
}

inline void PerfDataList::append(PerfData *p) {
  _set->append(p);
}

inline void PerfDataList::remove(PerfData *p) {
  _set->remove(p);
}

inline PerfData* PerfDataList::at(int index) {
  return _set->at(index);
}

inline int PerfDataManager::count() {
  return _all->length();
}

inline int PerfDataManager::sampled_count() {
  return _sampled->length();
}

inline int PerfDataManager::constants_count() {
  return _constants->length();
}

inline bool PerfDataManager::exists(const char* name) {
  return _all->contains(name);
}

#endif // SHARE_VM_RUNTIME_PERFDATA_INLINE_HPP
