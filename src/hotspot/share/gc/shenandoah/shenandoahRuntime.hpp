/*
 * Copyright (c) 2018, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHRUNTIME_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHRUNTIME_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oopsHierarchy.hpp"

class HeapWord;
class JavaThread;
class oopDesc;

class ShenandoahRuntime : public AllStatic {
public:
  static void arraycopy_barrier_oop_entry(oop* src, oop* dst, size_t length);
  static void arraycopy_barrier_narrow_oop_entry(narrowOop* src, narrowOop* dst, size_t length);

  static void write_ref_field_pre_entry(oopDesc* orig, JavaThread* thread);

  static oopDesc* load_reference_barrier(oopDesc* src, oop* load_addr);
  static oopDesc* load_reference_barrier_narrow(oopDesc* src, narrowOop* load_addr);

  static void shenandoah_clone_barrier(oopDesc* src);
};

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHRUNTIME_HPP
