/*
 * Copyright (c) 1997, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/constantPool.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/atomic.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"

#ifdef ASSERT
oop* HandleArea::allocate_handle(oop obj) {
  assert(_handle_mark_nesting > 1, "memory leak: allocating handle outside HandleMark");
  assert(_no_handle_mark_nesting == 0, "allocating handle inside NoHandleMark");
  assert(oopDesc::is_oop(obj), "not an oop: " INTPTR_FORMAT, p2i(obj));
  return real_allocate_handle(obj);
}
#endif

// Copy constructors and destructors for metadata handles
// These do too much to inline.
#define DEF_METADATA_HANDLE_FN_NOINLINE(name, type) \
name##Handle::name##Handle(const name##Handle &h) {                    \
  _value = h._value;                                                   \
  if (_value != NULL) {                                                \
    assert(_value->is_valid(), "obj is valid");                        \
    if (h._thread != NULL) {                                           \
      assert(h._thread == Thread::current(), "thread must be current");\
      _thread = h._thread;                                             \
    } else {                                                           \
      _thread = Thread::current();                                     \
    }                                                                  \
    assert (_thread->is_in_stack((address)this), "not on stack?");     \
    _thread->metadata_handles()->push((Metadata*)_value);              \
  } else {                                                             \
    _thread = NULL;                                                    \
  }                                                                    \
}                                                                      \
name##Handle& name##Handle::operator=(const name##Handle &s) {         \
  remove();                                                            \
  _value = s._value;                                                   \
  if (_value != NULL) {                                                \
    assert(_value->is_valid(), "obj is valid");                        \
    if (s._thread != NULL) {                                           \
      assert(s._thread == Thread::current(), "thread must be current");\
      _thread = s._thread;                                             \
    } else {                                                           \
      _thread = Thread::current();                                     \
    }                                                                  \
    assert (_thread->is_in_stack((address)this), "not on stack?");     \
    _thread->metadata_handles()->push((Metadata*)_value);              \
  } else {                                                             \
    _thread = NULL;                                                    \
  }                                                                    \
  return *this;                                                        \
}                                                                      \
inline void name##Handle::remove() {                                   \
  if (_value != NULL) {                                                \
    int i = _thread->metadata_handles()->find_from_end((Metadata*)_value); \
    assert(i!=-1, "not in metadata_handles list");                     \
    _thread->metadata_handles()->remove_at(i);                         \
  }                                                                    \
}                                                                      \
name##Handle::~name##Handle () { remove(); }                           \

DEF_METADATA_HANDLE_FN_NOINLINE(method, Method)
DEF_METADATA_HANDLE_FN_NOINLINE(constantPool, ConstantPool)


static uintx chunk_oops_do(OopClosure* f, Chunk* chunk, char* chunk_top) {
  oop* bottom = (oop*) chunk->bottom();
  oop* top    = (oop*) chunk_top;
  uintx handles_visited = top - bottom;
  assert(top >= bottom && top <= (oop*) chunk->top(), "just checking");
  // during GC phase 3, a handle may be a forward pointer that
  // is not yet valid, so loosen the assertion
  while (bottom < top) {
    f->do_oop(bottom++);
  }
  return handles_visited;
}

// Used for debugging handle allocation.
NOT_PRODUCT(jint _nof_handlemarks  = 0;)

void HandleArea::oops_do(OopClosure* f) {
  uintx handles_visited = 0;
  // First handle the current chunk. It is filled to the high water mark.
  handles_visited += chunk_oops_do(f, _chunk, _hwm);
  // Then handle all previous chunks. They are completely filled.
  Chunk* k = _first;
  while(k != _chunk) {
    handles_visited += chunk_oops_do(f, k, k->top());
    k = k->next();
  }

  if (_prev != NULL) _prev->oops_do(f);
}

void HandleMark::initialize(Thread* thread) {
  _thread = thread;
  // Save area
  _area  = thread->handle_area();
  // Save current top
  _chunk = _area->_chunk;
  _hwm   = _area->_hwm;
  _max   = _area->_max;
  _size_in_bytes = _area->_size_in_bytes;
  debug_only(_area->_handle_mark_nesting++);
  assert(_area->_handle_mark_nesting > 0, "must stack allocate HandleMarks");
  debug_only(Atomic::inc(&_nof_handlemarks);)

  // Link this in the thread
  set_previous_handle_mark(thread->last_handle_mark());
  thread->set_last_handle_mark(this);
}


HandleMark::~HandleMark() {
  HandleArea* area = _area;   // help compilers with poor alias analysis
  assert(area == _thread->handle_area(), "sanity check");
  assert(area->_handle_mark_nesting > 0, "must stack allocate HandleMarks" );
  debug_only(area->_handle_mark_nesting--);

  // Delete later chunks
  if( _chunk->next() ) {
    // reset arena size before delete chunks. Otherwise, the total
    // arena size could exceed total chunk size
    assert(area->size_in_bytes() > size_in_bytes(), "Sanity check");
    area->set_size_in_bytes(size_in_bytes());
    _chunk->next_chop();
  } else {
    assert(area->size_in_bytes() == size_in_bytes(), "Sanity check");
  }
  // Roll back arena to saved top markers
  area->_chunk = _chunk;
  area->_hwm = _hwm;
  area->_max = _max;
#ifdef ASSERT
  // clear out first chunk (to detect allocation bugs)
  if (ZapVMHandleArea) {
    memset(_hwm, badHandleValue, _max - _hwm);
  }
  Atomic::dec(&_nof_handlemarks);
#endif

  // Unlink this from the thread
  _thread->set_last_handle_mark(previous_handle_mark());
}

void* HandleMark::operator new(size_t size) throw() {
  return AllocateHeap(size, mtThread);
}

void* HandleMark::operator new [] (size_t size) throw() {
  return AllocateHeap(size, mtThread);
}

void HandleMark::operator delete(void* p) {
  FreeHeap(p);
}

void HandleMark::operator delete[](void* p) {
  FreeHeap(p);
}

#ifdef ASSERT

NoHandleMark::NoHandleMark() {
  HandleArea* area = Thread::current()->handle_area();
  area->_no_handle_mark_nesting++;
  assert(area->_no_handle_mark_nesting > 0, "must stack allocate NoHandleMark" );
}


NoHandleMark::~NoHandleMark() {
  HandleArea* area = Thread::current()->handle_area();
  assert(area->_no_handle_mark_nesting > 0, "must stack allocate NoHandleMark" );
  area->_no_handle_mark_nesting--;
}


ResetNoHandleMark::ResetNoHandleMark() {
  HandleArea* area = Thread::current()->handle_area();
  _no_handle_mark_nesting = area->_no_handle_mark_nesting;
  area->_no_handle_mark_nesting = 0;
}


ResetNoHandleMark::~ResetNoHandleMark() {
  HandleArea* area = Thread::current()->handle_area();
  area->_no_handle_mark_nesting = _no_handle_mark_nesting;
}

#endif // ASSERT
