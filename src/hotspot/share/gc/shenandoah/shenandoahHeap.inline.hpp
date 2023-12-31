/*
 * Copyright (c) 2015, 2020, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAP_INLINE_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAP_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/javaClasses.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/threadLocalAllocBuffer.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/suspendibleThreadSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/markBitMap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahAsserts.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahBarrierSet.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahCollectionSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahCollectionSet.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahForwarding.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahWorkGroup.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeapRegionSet.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeapRegion.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahControlThread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahMarkingContext.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahThreadLocalData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/atomic.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/interfaceSupport.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/prefetch.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/prefetch.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/copy.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"

inline ShenandoahHeap* ShenandoahHeap::heap() {
  assert(_heap != NULL, "Heap is not initialized yet");
  return _heap;
}

inline ShenandoahHeapRegion* ShenandoahRegionIterator::next() {
  size_t new_index = Atomic::add((size_t) 1, &_index);
  // get_region() provides the bounds-check and returns NULL on OOB.
  return _heap->get_region(new_index - 1);
}

inline bool ShenandoahHeap::has_forwarded_objects() const {
  return _gc_state.is_set(HAS_FORWARDED);
}

inline WorkGang* ShenandoahHeap::workers() const {
  return _workers;
}

inline WorkGang* ShenandoahHeap::get_safepoint_workers() {
  return _safepoint_workers;
}

inline size_t ShenandoahHeap::heap_region_index_containing(const void* addr) const {
  uintptr_t region_start = ((uintptr_t) addr);
  uintptr_t index = (region_start - (uintptr_t) base()) >> ShenandoahHeapRegion::region_size_bytes_shift();
  assert(index < num_regions(), "Region index is in bounds: " PTR_FORMAT, p2i(addr));
  return index;
}

inline ShenandoahHeapRegion* const ShenandoahHeap::heap_region_containing(const void* addr) const {
  size_t index = heap_region_index_containing(addr);
  ShenandoahHeapRegion* const result = get_region(index);
  assert(addr >= result->bottom() && addr < result->end(), "Heap region contains the address: " PTR_FORMAT, p2i(addr));
  return result;
}

template <class T>
inline oop ShenandoahHeap::update_with_forwarded_not_null(T* p, oop obj) {
  if (in_collection_set(obj)) {
    shenandoah_assert_forwarded_except(p, obj, is_full_gc_in_progress() || cancelled_gc() || is_degenerated_gc_in_progress());
    obj = ShenandoahBarrierSet::resolve_forwarded_not_null(obj);
    RawAccess<IS_NOT_NULL>::oop_store(p, obj);
  }
#ifdef ASSERT
  else {
    shenandoah_assert_not_forwarded(p, obj);
  }
#endif
  return obj;
}

template <class T>
inline oop ShenandoahHeap::maybe_update_with_forwarded(T* p) {
  T o = RawAccess<>::oop_load(p);
  if (!CompressedOops::is_null(o)) {
    oop obj = CompressedOops::decode_not_null(o);
    return maybe_update_with_forwarded_not_null(p, obj);
  } else {
    return NULL;
  }
}

template <class T>
inline oop ShenandoahHeap::evac_update_with_forwarded(T* p) {
  T o = RawAccess<>::oop_load(p);
  if (!CompressedOops::is_null(o)) {
    oop heap_oop = CompressedOops::decode_not_null(o);
    if (in_collection_set(heap_oop)) {
      oop forwarded_oop = ShenandoahBarrierSet::resolve_forwarded_not_null(heap_oop);
      if (forwarded_oop == heap_oop) {
        forwarded_oop = evacuate_object(heap_oop, Thread::current());
      }
      oop prev = cas_oop(forwarded_oop, p, heap_oop);
      if (prev == heap_oop) {
        return forwarded_oop;
      } else {
        return NULL;
      }
    }
    return heap_oop;
  } else {
    return NULL;
  }
}

inline oop ShenandoahHeap::cas_oop(oop n, oop* addr, oop c) {
  assert(is_aligned(addr, HeapWordSize), "Address should be aligned: " PTR_FORMAT, p2i(addr));
  return (oop) Atomic::cmpxchg(n, addr, c);
}

inline oop ShenandoahHeap::cas_oop(oop n, narrowOop* addr, narrowOop c) {
  narrowOop val = CompressedOops::encode(n);
  return CompressedOops::decode((narrowOop) Atomic::cmpxchg(val, addr, c));
}

inline oop ShenandoahHeap::cas_oop(oop n, narrowOop* addr, oop c) {
  assert(is_aligned(addr, sizeof(narrowOop)), "Address should be aligned: " PTR_FORMAT, p2i(addr));
  narrowOop cmp = CompressedOops::encode(c);
  narrowOop val = CompressedOops::encode(n);
  return CompressedOops::decode((narrowOop) Atomic::cmpxchg(val, addr, cmp));
}

template <class T>
inline oop ShenandoahHeap::maybe_update_with_forwarded_not_null(T* p, oop heap_oop) {
  shenandoah_assert_not_in_cset_loc_except(p, !is_in(p) || is_full_gc_in_progress() || is_degenerated_gc_in_progress());
  shenandoah_assert_correct(p, heap_oop);

  if (in_collection_set(heap_oop)) {
    oop forwarded_oop = ShenandoahBarrierSet::resolve_forwarded_not_null(heap_oop);
    if (forwarded_oop == heap_oop) {
      // E.g. during evacuation.
      return forwarded_oop;
    }

    shenandoah_assert_forwarded_except(p, heap_oop, is_full_gc_in_progress() || is_degenerated_gc_in_progress());
    shenandoah_assert_not_forwarded(p, forwarded_oop);
    shenandoah_assert_not_in_cset_except(p, forwarded_oop, cancelled_gc());

    // If this fails, another thread wrote to p before us, it will be logged in SATB and the
    // reference be updated later.
    oop witness = cas_oop(forwarded_oop, p, heap_oop);

    if (witness != heap_oop) {
      // CAS failed, someone had beat us to it. Normally, we would return the failure witness,
      // because that would be the proper write of to-space object, enforced by strong barriers.
      // However, there is a corner case with arraycopy. It can happen that a Java thread
      // beats us with an arraycopy, which first copies the array, which potentially contains
      // from-space refs, and only afterwards updates all from-space refs to to-space refs,
      // which leaves a short window where the new array elements can be from-space.
      // In this case, we can just resolve the result again. As we resolve, we need to consider
      // the contended write might have been NULL.
      oop result = ShenandoahBarrierSet::resolve_forwarded(witness);
      shenandoah_assert_not_forwarded_except(p, result, (result == NULL));
      shenandoah_assert_not_in_cset_except(p, result, (result == NULL) || cancelled_gc());
      return result;
    } else {
      // Success! We have updated with known to-space copy. We have already asserted it is sane.
      return forwarded_oop;
    }
  } else {
    shenandoah_assert_not_forwarded(p, heap_oop);
    return heap_oop;
  }
}

inline bool ShenandoahHeap::cancelled_gc() const {
  return _cancelled_gc.get() == CANCELLED;
}

inline bool ShenandoahHeap::check_cancelled_gc_and_yield(bool sts_active) {
  if (! (sts_active && ShenandoahSuspendibleWorkers)) {
    return cancelled_gc();
  }

  jbyte prev = _cancelled_gc.cmpxchg(NOT_CANCELLED, CANCELLABLE);
  if (prev == CANCELLABLE || prev == NOT_CANCELLED) {
    if (SuspendibleThreadSet::should_yield()) {
      SuspendibleThreadSet::yield();
    }

    // Back to CANCELLABLE. The thread that poked NOT_CANCELLED first gets
    // to restore to CANCELLABLE.
    if (prev == CANCELLABLE) {
      _cancelled_gc.set(CANCELLABLE);
    }
    return false;
  } else {
    return true;
  }
}

inline bool ShenandoahHeap::try_cancel_gc() {
  while (true) {
    jbyte prev = _cancelled_gc.cmpxchg(CANCELLED, CANCELLABLE);
    if (prev == CANCELLABLE) return true;
    else if (prev == CANCELLED) return false;
    assert(ShenandoahSuspendibleWorkers, "should not get here when not using suspendible workers");
    assert(prev == NOT_CANCELLED, "must be NOT_CANCELLED");
    if (Thread::current()->is_Java_thread()) {
      // We need to provide a safepoint here, otherwise we might
      // spin forever if a SP is pending.
      ThreadBlockInVM sp(JavaThread::current());
      SpinPause();
    }
  }
}

inline void ShenandoahHeap::clear_cancelled_gc() {
  _cancelled_gc.set(CANCELLABLE);
  _oom_evac_handler.clear();
}

inline HeapWord* ShenandoahHeap::allocate_from_gclab(Thread* thread, size_t size) {
  assert(UseTLAB, "TLABs should be enabled");

  PLAB* gclab = ShenandoahThreadLocalData::gclab(thread);
  if (gclab == NULL) {
    assert(!thread->is_Java_thread() && !thread->is_Worker_thread(),
           "Performance: thread should have GCLAB: %s", thread->name());
    // No GCLABs in this thread, fallback to shared allocation
    return NULL;
  }
  HeapWord* obj = gclab->allocate(size);
  if (obj != NULL) {
    return obj;
  }
  // Otherwise...
  return allocate_from_gclab_slow(thread, size);
}

inline oop ShenandoahHeap::evacuate_object(oop p, Thread* thread) {
  if (ShenandoahThreadLocalData::is_oom_during_evac(Thread::current())) {
    // This thread went through the OOM during evac protocol and it is safe to return
    // the forward pointer. It must not attempt to evacuate any more.
    return ShenandoahBarrierSet::resolve_forwarded(p);
  }

  assert(ShenandoahThreadLocalData::is_evac_allowed(thread), "must be enclosed in oom-evac scope");

  size_t size = p->size();

  assert(!heap_region_containing(p)->is_humongous(), "never evacuate humongous objects");

  bool alloc_from_gclab = true;
  HeapWord* copy = NULL;

#ifdef ASSERT
  if (ShenandoahOOMDuringEvacALot &&
      (os::random() & 1) == 0) { // Simulate OOM every ~2nd slow-path call
        copy = NULL;
  } else {
#endif
    if (UseTLAB) {
      copy = allocate_from_gclab(thread, size);
    }
    if (copy == NULL) {
      ShenandoahAllocRequest req = ShenandoahAllocRequest::for_shared_gc(size);
      copy = allocate_memory(req);
      alloc_from_gclab = false;
    }
#ifdef ASSERT
  }
#endif

  if (copy == NULL) {
    control_thread()->handle_alloc_failure_evac(size);

    _oom_evac_handler.handle_out_of_memory_during_evacuation();

    return ShenandoahBarrierSet::resolve_forwarded(p);
  }

  // Copy the object:
  Copy::aligned_disjoint_words((HeapWord*) p, copy, size);

  // Try to install the new forwarding pointer.
  oop copy_val = oop(copy);
  oop result = ShenandoahForwarding::try_update_forwardee(p, copy_val);
  if (result == copy_val) {
    // Successfully evacuated. Our copy is now the public one!
    shenandoah_assert_correct(NULL, copy_val);
    return copy_val;
  }  else {
    // Failed to evacuate. We need to deal with the object that is left behind. Since this
    // new allocation is certainly after TAMS, it will be considered live in the next cycle.
    // But if it happens to contain references to evacuated regions, those references would
    // not get updated for this stale copy during this cycle, and we will crash while scanning
    // it the next cycle.
    //
    // For GCLAB allocations, it is enough to rollback the allocation ptr. Either the next
    // object will overwrite this stale copy, or the filler object on LAB retirement will
    // do this. For non-GCLAB allocations, we have no way to retract the allocation, and
    // have to explicitly overwrite the copy with the filler object. With that overwrite,
    // we have to keep the fwdptr initialized and pointing to our (stale) copy.
    if (alloc_from_gclab) {
      ShenandoahThreadLocalData::gclab(thread)->undo_allocation(copy, size);
    } else {
      fill_with_object(copy, size);
      shenandoah_assert_correct(NULL, copy_val);
    }
    shenandoah_assert_correct(NULL, result);
    return result;
  }
}

inline bool ShenandoahHeap::requires_marking(const void* entry) const {
  return !_marking_context->is_marked(oop(entry));
}

inline bool ShenandoahHeap::in_collection_set(oop p) const {
  assert(collection_set() != NULL, "Sanity");
  return collection_set()->is_in(p);
}

inline bool ShenandoahHeap::in_collection_set_loc(void* p) const {
  assert(collection_set() != NULL, "Sanity");
  return collection_set()->is_in_loc(p);
}

inline bool ShenandoahHeap::is_stable() const {
  return _gc_state.is_clear();
}

inline bool ShenandoahHeap::is_idle() const {
  return _gc_state.is_unset(MARKING | EVACUATION | UPDATEREFS);
}

inline bool ShenandoahHeap::is_concurrent_mark_in_progress() const {
  return _gc_state.is_set(MARKING);
}

inline bool ShenandoahHeap::is_evacuation_in_progress() const {
  return _gc_state.is_set(EVACUATION);
}

inline bool ShenandoahHeap::is_gc_in_progress_mask(uint mask) const {
  return _gc_state.is_set(mask);
}

inline bool ShenandoahHeap::is_degenerated_gc_in_progress() const {
  return _degenerated_gc_in_progress.is_set();
}

inline bool ShenandoahHeap::is_full_gc_in_progress() const {
  return _full_gc_in_progress.is_set();
}

inline bool ShenandoahHeap::is_full_gc_move_in_progress() const {
  return _full_gc_move_in_progress.is_set();
}

inline bool ShenandoahHeap::is_update_refs_in_progress() const {
  return _gc_state.is_set(UPDATEREFS);
}

template<class T>
inline void ShenandoahHeap::marked_object_iterate(ShenandoahHeapRegion* region, T* cl) {
  marked_object_iterate(region, cl, region->top());
}

template<class T>
inline void ShenandoahHeap::marked_object_iterate(ShenandoahHeapRegion* region, T* cl, HeapWord* limit) {
  assert(! region->is_humongous_continuation(), "no humongous continuation regions here");

  ShenandoahMarkingContext* const ctx = complete_marking_context();
  assert(ctx->is_complete(), "sanity");

  MarkBitMap* mark_bit_map = ctx->mark_bit_map();
  HeapWord* tams = ctx->top_at_mark_start(region);

  size_t skip_bitmap_delta = 1;
  HeapWord* start = region->bottom();
  HeapWord* end = MIN2(tams, region->end());

  // Step 1. Scan below the TAMS based on bitmap data.
  HeapWord* limit_bitmap = MIN2(limit, tams);

  // Try to scan the initial candidate. If the candidate is above the TAMS, it would
  // fail the subsequent "< limit_bitmap" checks, and fall through to Step 2.
  HeapWord* cb = mark_bit_map->getNextMarkedWordAddress(start, end);

  intx dist = ShenandoahMarkScanPrefetch;
  if (dist > 0) {
    // Batched scan that prefetches the oop data, anticipating the access to
    // either header, oop field, or forwarding pointer. Not that we cannot
    // touch anything in oop, while it still being prefetched to get enough
    // time for prefetch to work. This is why we try to scan the bitmap linearly,
    // disregarding the object size. However, since we know forwarding pointer
    // preceeds the object, we can skip over it. Once we cannot trust the bitmap,
    // there is no point for prefetching the oop contents, as oop->size() will
    // touch it prematurely.

    // No variable-length arrays in standard C++, have enough slots to fit
    // the prefetch distance.
    static const int SLOT_COUNT = 256;
    guarantee(dist <= SLOT_COUNT, "adjust slot count");
    HeapWord* slots[SLOT_COUNT];

    int avail;
    do {
      avail = 0;
      for (int c = 0; (c < dist) && (cb < limit_bitmap); c++) {
        Prefetch::read(cb, oopDesc::mark_offset_in_bytes());
        slots[avail++] = cb;
        cb += skip_bitmap_delta;
        if (cb < limit_bitmap) {
          cb = mark_bit_map->getNextMarkedWordAddress(cb, limit_bitmap);
        }
      }

      for (int c = 0; c < avail; c++) {
        assert (slots[c] < tams,  "only objects below TAMS here: "  PTR_FORMAT " (" PTR_FORMAT ")", p2i(slots[c]), p2i(tams));
        assert (slots[c] < limit, "only objects below limit here: " PTR_FORMAT " (" PTR_FORMAT ")", p2i(slots[c]), p2i(limit));
        oop obj = oop(slots[c]);
        assert(oopDesc::is_oop(obj), "sanity");
        assert(ctx->is_marked(obj), "object expected to be marked");
        cl->do_object(obj);
      }
    } while (avail > 0);
  } else {
    while (cb < limit_bitmap) {
      assert (cb < tams,  "only objects below TAMS here: "  PTR_FORMAT " (" PTR_FORMAT ")", p2i(cb), p2i(tams));
      assert (cb < limit, "only objects below limit here: " PTR_FORMAT " (" PTR_FORMAT ")", p2i(cb), p2i(limit));
      oop obj = oop(cb);
      assert(oopDesc::is_oop(obj), "sanity");
      assert(ctx->is_marked(obj), "object expected to be marked");
      cl->do_object(obj);
      cb += skip_bitmap_delta;
      if (cb < limit_bitmap) {
        cb = mark_bit_map->getNextMarkedWordAddress(cb, limit_bitmap);
      }
    }
  }

  // Step 2. Accurate size-based traversal, happens past the TAMS.
  // This restarts the scan at TAMS, which makes sure we traverse all objects,
  // regardless of what happened at Step 1.
  HeapWord* cs = tams;
  while (cs < limit) {
    assert (cs >= tams, "only objects past TAMS here: "   PTR_FORMAT " (" PTR_FORMAT ")", p2i(cs), p2i(tams));
    assert (cs < limit, "only objects below limit here: " PTR_FORMAT " (" PTR_FORMAT ")", p2i(cs), p2i(limit));
    oop obj = oop(cs);
    assert(oopDesc::is_oop(obj), "sanity");
    assert(ctx->is_marked(obj), "object expected to be marked");
    int size = obj->size();
    cl->do_object(obj);
    cs += size;
  }
}

template <class T>
class ShenandoahObjectToOopClosure : public ObjectClosure {
  T* _cl;
public:
  ShenandoahObjectToOopClosure(T* cl) : _cl(cl) {}

  void do_object(oop obj) {
    obj->oop_iterate(_cl);
  }
};

template <class T>
class ShenandoahObjectToOopBoundedClosure : public ObjectClosure {
  T* _cl;
  MemRegion _bounds;
public:
  ShenandoahObjectToOopBoundedClosure(T* cl, HeapWord* bottom, HeapWord* top) :
    _cl(cl), _bounds(bottom, top) {}

  void do_object(oop obj) {
    obj->oop_iterate(_cl, _bounds);
  }
};

template<class T>
inline void ShenandoahHeap::marked_object_oop_iterate(ShenandoahHeapRegion* region, T* cl, HeapWord* top) {
  if (region->is_humongous()) {
    HeapWord* bottom = region->bottom();
    if (top > bottom) {
      region = region->humongous_start_region();
      ShenandoahObjectToOopBoundedClosure<T> objs(cl, bottom, top);
      marked_object_iterate(region, &objs);
    }
  } else {
    ShenandoahObjectToOopClosure<T> objs(cl);
    marked_object_iterate(region, &objs, top);
  }
}

inline ShenandoahHeapRegion* const ShenandoahHeap::get_region(size_t region_idx) const {
  if (region_idx < _num_regions) {
    return _regions[region_idx];
  } else {
    return NULL;
  }
}

inline void ShenandoahHeap::mark_complete_marking_context() {
  _marking_context->mark_complete();
}

inline void ShenandoahHeap::mark_incomplete_marking_context() {
  _marking_context->mark_incomplete();
}

inline ShenandoahMarkingContext* ShenandoahHeap::complete_marking_context() const {
  assert (_marking_context->is_complete()," sanity");
  return _marking_context;
}

inline ShenandoahMarkingContext* ShenandoahHeap::marking_context() const {
  return _marking_context;
}

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAP_INLINE_HPP
