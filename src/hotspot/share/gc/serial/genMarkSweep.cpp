/*
 * Copyright (c) 2001, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/javaClasses.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/stringTable.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/symbolTable.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/vmSymbols.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/codeCache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/icBuffer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/serial/genMarkSweep.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcHeapSummary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcTimer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcTrace.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcTraceTime.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/genCollectedHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/generation.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/genOopClosures.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/modRefBarrierSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/referencePolicy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/referenceProcessorPhaseTimes.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/space.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/strongRootsScope.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/weakProcessor.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceRefKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/jvmtiExport.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/synchronizer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vmThread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/copy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/events.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/stack.inline.hpp"

void GenMarkSweep::invoke_at_safepoint(ReferenceProcessor* rp, bool clear_all_softrefs) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");

  GenCollectedHeap* gch = GenCollectedHeap::heap();
#ifdef ASSERT
  if (gch->soft_ref_policy()->should_clear_all_soft_refs()) {
    assert(clear_all_softrefs, "Policy should have been checked earlier");
  }
#endif

  // hook up weak ref data so it can be used during Mark-Sweep
  assert(ref_processor() == NULL, "no stomping");
  assert(rp != NULL, "should be non-NULL");
  set_ref_processor(rp);
  rp->setup_policy(clear_all_softrefs);

  gch->trace_heap_before_gc(_gc_tracer);

  // When collecting the permanent generation Method*s may be moving,
  // so we either have to flush all bcp data or convert it into bci.
  CodeCache::gc_prologue();

  // Increment the invocation count
  _total_invocations++;

  // Capture used regions for each generation that will be
  // subject to collection, so that card table adjustments can
  // be made intelligently (see clear / invalidate further below).
  gch->save_used_regions();

  allocate_stacks();

  mark_sweep_phase1(clear_all_softrefs);

  mark_sweep_phase2();

  // Don't add any more derived pointers during phase3
#if COMPILER2_OR_JVMCI
  assert(DerivedPointerTable::is_active(), "Sanity");
  DerivedPointerTable::set_active(false);
#endif

  mark_sweep_phase3();

  mark_sweep_phase4();

  restore_marks();

  // Set saved marks for allocation profiler (and other things? -- dld)
  // (Should this be in general part?)
  gch->save_marks();

  deallocate_stacks();

  // If compaction completely evacuated the young generation then we
  // can clear the card table.  Otherwise, we must invalidate
  // it (consider all cards dirty).  In the future, we might consider doing
  // compaction within generations only, and doing card-table sliding.
  CardTableRS* rs = gch->rem_set();
  Generation* old_gen = gch->old_gen();

  // Clear/invalidate below make use of the "prev_used_regions" saved earlier.
  if (gch->young_gen()->used() == 0) {
    // We've evacuated the young generation.
    rs->clear_into_younger(old_gen);
  } else {
    // Invalidate the cards corresponding to the currently used
    // region and clear those corresponding to the evacuated region.
    rs->invalidate_or_clear(old_gen);
  }

  CodeCache::gc_epilogue();
  JvmtiExport::gc_epilogue();

  // refs processing: clean slate
  set_ref_processor(NULL);

  // Update heap occupancy information which is used as
  // input to soft ref clearing policy at the next gc.
  Universe::update_heap_info_at_gc();

  // Update time of last gc for all generations we collected
  // (which currently is all the generations in the heap).
  // We need to use a monotonically non-decreasing time in ms
  // or we will see time-warp warnings and os::javaTimeMillis()
  // does not guarantee monotonicity.
  jlong now = os::javaTimeNanos() / NANOSECS_PER_MILLISEC;
  gch->update_time_of_last_gc(now);

  gch->trace_heap_after_gc(_gc_tracer);
}

void GenMarkSweep::allocate_stacks() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  // Scratch request on behalf of old generation; will do no allocation.
  ScratchBlock* scratch = gch->gather_scratch(gch->old_gen(), 0);

  // $$$ To cut a corner, we'll only use the first scratch block, and then
  // revert to malloc.
  if (scratch != NULL) {
    _preserved_count_max =
      scratch->num_words * HeapWordSize / sizeof(PreservedMark);
  } else {
    _preserved_count_max = 0;
  }

  _preserved_marks = (PreservedMark*)scratch;
  _preserved_count = 0;
}


void GenMarkSweep::deallocate_stacks() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  gch->release_scratch();

  _preserved_mark_stack.clear(true);
  _preserved_oop_stack.clear(true);
  _marking_stack.clear();
  _objarray_stack.clear(true);
}

void GenMarkSweep::mark_sweep_phase1(bool clear_all_softrefs) {
  // Recursively traverse all live objects and mark them
  GCTraceTime(Info, gc, phases) tm("Phase 1: Mark live objects", _gc_timer);

  GenCollectedHeap* gch = GenCollectedHeap::heap();

  // Because follow_root_closure is created statically, cannot
  // use OopsInGenClosure constructor which takes a generation,
  // as the Universe has not been created when the static constructors
  // are run.
  follow_root_closure.set_orig_generation(gch->old_gen());

  // Need new claim bits before marking starts.
  ClassLoaderDataGraph::clear_claimed_marks();

  {
    StrongRootsScope srs(1);

    gch->full_process_roots(&srs,
                            false, // not the adjust phase
                            GenCollectedHeap::SO_None,
                            ClassUnloading, // only strong roots if ClassUnloading
                                            // is enabled
                            &follow_root_closure,
                            &follow_cld_closure);
  }

  // Process reference objects found during marking
  {
    GCTraceTime(Debug, gc, phases) tm_m("Reference Processing", gc_timer());

    ref_processor()->setup_policy(clear_all_softrefs);
    ReferenceProcessorPhaseTimes pt(_gc_timer, ref_processor()->max_num_queues());
    const ReferenceProcessorStats& stats =
      ref_processor()->process_discovered_references(
        &is_alive, &keep_alive, &follow_stack_closure, NULL, &pt);
    pt.print_all_references();
    gc_tracer()->report_gc_reference_stats(stats);
  }

  // This is the point where the entire marking should have completed.
  assert(_marking_stack.is_empty(), "Marking should have completed");

  {
    GCTraceTime(Debug, gc, phases) tm_m("Weak Processing", gc_timer());
    WeakProcessor::weak_oops_do(&is_alive, &do_nothing_cl);
  }

  {
    GCTraceTime(Debug, gc, phases) tm_m("Class Unloading", gc_timer());

    // Unload classes and purge the SystemDictionary.
    bool purged_class = SystemDictionary::do_unloading(gc_timer());

    // Unload nmethods.
    CodeCache::do_unloading(&is_alive, purged_class);

    // Prune dead klasses from subklass/sibling/implementor lists.
    Klass::clean_weak_klass_links(purged_class);
  }

  {
    GCTraceTime(Debug, gc, phases) t("Scrub String Table", gc_timer());
    // Delete entries for dead interned strings.
    StringTable::unlink(&is_alive);
  }

  {
    GCTraceTime(Debug, gc, phases) t("Scrub Symbol Table", gc_timer());
    // Clean up unreferenced symbols in symbol table.
    SymbolTable::unlink();
  }

  gc_tracer()->report_object_count_after_gc(&is_alive);
}


void GenMarkSweep::mark_sweep_phase2() {
  // Now all live objects are marked, compute the new object addresses.

  // It is imperative that we traverse perm_gen LAST. If dead space is
  // allowed a range of dead object may get overwritten by a dead int
  // array. If perm_gen is not traversed last a Klass* may get
  // overwritten. This is fine since it is dead, but if the class has dead
  // instances we have to skip them, and in order to find their size we
  // need the Klass*!
  //
  // It is not required that we traverse spaces in the same order in
  // phase2, phase3 and phase4, but the ValidateMarkSweep live oops
  // tracking expects us to do so. See comment under phase4.

  GenCollectedHeap* gch = GenCollectedHeap::heap();

  GCTraceTime(Info, gc, phases) tm("Phase 2: Compute new object addresses", _gc_timer);

  gch->prepare_for_compaction();
}

class GenAdjustPointersClosure: public GenCollectedHeap::GenClosure {
public:
  void do_generation(Generation* gen) {
    gen->adjust_pointers();
  }
};

void GenMarkSweep::mark_sweep_phase3() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();

  // Adjust the pointers to reflect the new locations
  GCTraceTime(Info, gc, phases) tm("Phase 3: Adjust pointers", gc_timer());

  // Need new claim bits for the pointer adjustment tracing.
  ClassLoaderDataGraph::clear_claimed_marks();

  // Because the closure below is created statically, we cannot
  // use OopsInGenClosure constructor which takes a generation,
  // as the Universe has not been created when the static constructors
  // are run.
  adjust_pointer_closure.set_orig_generation(gch->old_gen());

  {
    StrongRootsScope srs(1);

    gch->full_process_roots(&srs,
                            true,  // this is the adjust phase
                            GenCollectedHeap::SO_AllCodeCache,
                            false, // all roots
                            &adjust_pointer_closure,
                            &adjust_cld_closure);
  }

  gch->gen_process_weak_roots(&adjust_pointer_closure);

  adjust_marks();
  GenAdjustPointersClosure blk;
  gch->generation_iterate(&blk, true);
}

class GenCompactClosure: public GenCollectedHeap::GenClosure {
public:
  void do_generation(Generation* gen) {
    gen->compact();
  }
};

void GenMarkSweep::mark_sweep_phase4() {
  // All pointers are now adjusted, move objects accordingly

  // It is imperative that we traverse perm_gen first in phase4. All
  // classes must be allocated earlier than their instances, and traversing
  // perm_gen first makes sure that all Klass*s have moved to their new
  // location before any instance does a dispatch through it's klass!

  // The ValidateMarkSweep live oops tracking expects us to traverse spaces
  // in the same order in phase2, phase3 and phase4. We don't quite do that
  // here (perm_gen first rather than last), so we tell the validate code
  // to use a higher index (saved from phase2) when verifying perm_gen.
  GenCollectedHeap* gch = GenCollectedHeap::heap();

  GCTraceTime(Info, gc, phases) tm("Phase 4: Move objects", _gc_timer);

  GenCompactClosure blk;
  gch->generation_iterate(&blk, true);
}
