/*
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_GC_Z_ZTHREADLOCALDATA_HPP
#define SHARE_GC_Z_ZTHREADLOCALDATA_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zMarkStack.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/sizes.hpp"

class ZThreadLocalData {
private:
  uintptr_t              _address_bad_mask;
  ZMarkThreadLocalStacks _stacks;

  ZThreadLocalData() :
      _address_bad_mask(0),
      _stacks() {}

  static ZThreadLocalData* data(Thread* thread) {
    return thread->gc_data<ZThreadLocalData>();
  }

public:
  static void create(Thread* thread) {
    new (data(thread)) ZThreadLocalData();
  }

  static void destroy(Thread* thread) {
    data(thread)->~ZThreadLocalData();
  }

  static void set_address_bad_mask(Thread* thread, uintptr_t mask) {
    data(thread)->_address_bad_mask = mask;
  }

  static ZMarkThreadLocalStacks* stacks(Thread* thread) {
    return &data(thread)->_stacks;
  }

  static ByteSize address_bad_mask_offset() {
    return Thread::gc_data_offset() + byte_offset_of(ZThreadLocalData, _address_bad_mask);
  }
};

#endif // SHARE_GC_Z_ZTHREADLOCALDATA_HPP
