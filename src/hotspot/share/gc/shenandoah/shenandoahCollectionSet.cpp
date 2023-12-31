/*
 * Copyright (c) 2016, 2018, Red Hat, Inc. All rights reserved.
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

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahCollectionSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeapRegion.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeapRegionSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahUtils.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/atomic.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/services/memTracker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/copy.hpp"

ShenandoahCollectionSet::ShenandoahCollectionSet(ShenandoahHeap* heap, ReservedSpace space, char* heap_base) :
  _map_size(heap->num_regions()),
  _region_size_bytes_shift(ShenandoahHeapRegion::region_size_bytes_shift()),
  _map_space(space),
  _cset_map(_map_space.base() + ((uintx)heap_base >> _region_size_bytes_shift)),
  _biased_cset_map(_map_space.base()),
  _heap(heap),
  _garbage(0),
  _used(0),
  _region_count(0),
  _current_index(0) {

  // The collection set map is reserved to cover the entire heap *and* zero addresses.
  // This is needed to accept in-cset checks for both heap oops and NULLs, freeing
  // high-performance code from checking for NULL first.
  //
  // Since heap_base can be far away, committing the entire map would waste memory.
  // Therefore, we only commit the parts that are needed to operate: the heap view,
  // and the zero page.
  //
  // Note: we could instead commit the entire map, and piggyback on OS virtual memory
  // subsystem for mapping not-yet-written-to pages to a single physical backing page,
  // but this is not guaranteed, and would confuse NMT and other memory accounting tools.

  MemTracker::record_virtual_memory_type(_map_space.base(), mtGC);

  size_t page_size = (size_t)os::vm_page_size();

  if (!_map_space.special()) {
    // Commit entire pages that cover the heap cset map.
    char* bot_addr = align_down(_cset_map, page_size);
    char* top_addr = align_up(_cset_map + _map_size, page_size);
    os::commit_memory_or_exit(bot_addr, pointer_delta(top_addr, bot_addr, 1), false,
                              "Unable to commit collection set bitmap: heap");

    // Commit the zero page, if not yet covered by heap cset map.
    if (bot_addr != _biased_cset_map) {
      os::commit_memory_or_exit(_biased_cset_map, page_size, false,
                                "Unable to commit collection set bitmap: zero page");
    }
  }

  Copy::zero_to_bytes(_cset_map, _map_size);
  Copy::zero_to_bytes(_biased_cset_map, page_size);
}

void ShenandoahCollectionSet::add_region(ShenandoahHeapRegion* r) {
  assert(ShenandoahSafepoint::is_at_shenandoah_safepoint(), "Must be at a safepoint");
  assert(Thread::current()->is_VM_thread(), "Must be VMThread");
  assert(!is_in(r), "Already in collection set");
  _cset_map[r->index()] = 1;
  _region_count++;
  _garbage += r->garbage();
  _used += r->used();

  // Update the region status too. State transition would be checked internally.
  r->make_cset();
}

void ShenandoahCollectionSet::clear() {
  assert(ShenandoahSafepoint::is_at_shenandoah_safepoint(), "Must be at a safepoint");
  Copy::zero_to_bytes(_cset_map, _map_size);

#ifdef ASSERT
  for (size_t index = 0; index < _heap->num_regions(); index ++) {
    assert (!_heap->get_region(index)->is_cset(), "should have been cleared before");
  }
#endif

  _garbage = 0;
  _used = 0;

  _region_count = 0;
  _current_index = 0;
}

ShenandoahHeapRegion* ShenandoahCollectionSet::claim_next() {
  size_t num_regions = _heap->num_regions();
  if (_current_index >= (jint)num_regions) {
    return NULL;
  }

  jint saved_current = _current_index;
  size_t index = (size_t)saved_current;

  while(index < num_regions) {
    if (is_in(index)) {
      jint cur = Atomic::cmpxchg((jint)(index + 1), &_current_index, saved_current);
      assert(cur >= (jint)saved_current, "Must move forward");
      if (cur == saved_current) {
        assert(is_in(index), "Invariant");
        return _heap->get_region(index);
      } else {
        index = (size_t)cur;
        saved_current = cur;
      }
    } else {
      index ++;
    }
  }
  return NULL;
}

ShenandoahHeapRegion* ShenandoahCollectionSet::next() {
  assert(ShenandoahSafepoint::is_at_shenandoah_safepoint(), "Must be at a safepoint");
  assert(Thread::current()->is_VM_thread(), "Must be VMThread");
  size_t num_regions = _heap->num_regions();
  for (size_t index = (size_t)_current_index; index < num_regions; index ++) {
    if (is_in(index)) {
      _current_index = (jint)(index + 1);
      return _heap->get_region(index);
    }
  }

  return NULL;
}

void ShenandoahCollectionSet::print_on(outputStream* out) const {
  out->print_cr("Collection Set : " SIZE_FORMAT "", count());

  debug_only(size_t regions = 0;)
  for (size_t index = 0; index < _heap->num_regions(); index ++) {
    if (is_in(index)) {
      _heap->get_region(index)->print_on(out);
      debug_only(regions ++;)
    }
  }
  assert(regions == count(), "Must match");
}
