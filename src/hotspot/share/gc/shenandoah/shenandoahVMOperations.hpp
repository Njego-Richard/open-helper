/*
 * Copyright (c) 2013, 2018, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_VM_GC_SHENANDOAH_VM_OPERATIONS_SHENANDOAH_HPP
#define SHARE_VM_GC_SHENANDOAH_VM_OPERATIONS_SHENANDOAH_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/vmGCOperations.hpp"

// VM_operations for the Shenandoah Collector.
//
// VM_ShenandoahOperation
//   - VM_ShenandoahInitMark: initiate concurrent marking
//   - VM_ShenandoahReferenceOperation:
//       - VM_ShenandoahFinalMarkStartEvac: finish up concurrent marking, and start evacuation
//       - VM_ShenandoahInitUpdateRefs: initiate update references
//       - VM_ShenandoahFinalUpdateRefs: finish up update references
//       - VM_ShenandoahFullGC: do full GC

class VM_ShenandoahOperation : public VM_Operation {
protected:
  uint         _gc_id;
public:
  VM_ShenandoahOperation() : _gc_id(GCId::current()) {};
};

class VM_ShenandoahReferenceOperation : public VM_ShenandoahOperation {
public:
  VM_ShenandoahReferenceOperation() : VM_ShenandoahOperation() {};
  bool doit_prologue();
  void doit_epilogue();
};

class VM_ShenandoahInitMark: public VM_ShenandoahOperation {
public:
  VM_ShenandoahInitMark() : VM_ShenandoahOperation() {};
  VM_Operation::VMOp_Type type() const { return VMOp_ShenandoahInitMark; }
  const char* name()             const { return "Shenandoah Init Marking"; }
  virtual void doit();
};

class VM_ShenandoahFinalMarkStartEvac: public VM_ShenandoahReferenceOperation {
public:
  VM_ShenandoahFinalMarkStartEvac() : VM_ShenandoahReferenceOperation() {};
  VM_Operation::VMOp_Type type() const { return VMOp_ShenandoahFinalMarkStartEvac; }
  const char* name()             const { return "Shenandoah Final Mark and Start Evacuation"; }
  virtual  void doit();
};

class VM_ShenandoahDegeneratedGC: public VM_ShenandoahReferenceOperation {
private:
  // Really the ShenandoahHeap::ShenandoahDegenerationPoint, but casted to int here
  // in order to avoid dependency on ShenandoahHeap
  int _point;
public:
  VM_ShenandoahDegeneratedGC(int point) : VM_ShenandoahReferenceOperation(), _point(point) {};
  VM_Operation::VMOp_Type type() const { return VMOp_ShenandoahDegeneratedGC; }
  const char* name()             const { return "Shenandoah Degenerated GC"; }
  virtual  void doit();
};

class VM_ShenandoahFullGC : public VM_ShenandoahReferenceOperation {
private:
  GCCause::Cause _gc_cause;
public:
  VM_ShenandoahFullGC(GCCause::Cause gc_cause) : VM_ShenandoahReferenceOperation(), _gc_cause(gc_cause) {};
  VM_Operation::VMOp_Type type() const { return VMOp_ShenandoahFullGC; }
  const char* name()             const { return "Shenandoah Full GC"; }
  virtual void doit();
};

class VM_ShenandoahInitUpdateRefs: public VM_ShenandoahOperation {
public:
  VM_ShenandoahInitUpdateRefs() : VM_ShenandoahOperation() {};
  VM_Operation::VMOp_Type type() const { return VMOp_ShenandoahInitUpdateRefs; }
  const char* name()             const { return "Shenandoah Init Update References"; }
  virtual void doit();
};

class VM_ShenandoahFinalUpdateRefs: public VM_ShenandoahOperation {
public:
  VM_ShenandoahFinalUpdateRefs() : VM_ShenandoahOperation() {};
  VM_Operation::VMOp_Type type() const { return VMOp_ShenandoahFinalUpdateRefs; }
  const char* name()             const { return "Shenandoah Final Update References"; }
  virtual void doit();
};

#endif //SHARE_VM_GC_SHENANDOAH_VM_OPERATIONS_SHENANDOAH_HPP
