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

#ifndef SHARE_VM_AOT_AOTLOADER_INLLINE_HPP
#define SHARE_VM_AOT_AOTLOADER_INLLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/aot/aotLoader.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/growableArray.hpp"

#if INCLUDE_AOT
GrowableArray<AOTCodeHeap*>* AOTLoader::heaps() { return _heaps; }
GrowableArray<AOTLib*>* AOTLoader::libraries() { return _libraries; }
int AOTLoader::heaps_count() { return heaps()->length(); }
int AOTLoader::libraries_count() { return libraries()->length(); }
void AOTLoader::add_heap(AOTCodeHeap *heap) { heaps()->append(heap); }
void AOTLoader::add_library(AOTLib *lib) { libraries()->append(lib); }
#endif

#endif // SHARE_VM_AOT_AOTLOADER_INLLINE_HPP
