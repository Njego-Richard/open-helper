/*
 * Copyright (c) 1997, 2017, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcLocker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/atomic.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/safepoint.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/threadSMR.hpp"

volatile jint GCLocker::_jni_lock_count = 0;
volatile bool GCLocker::_needs_gc       = false;
volatile bool GCLocker::_doing_gc       = false;
unsigned int GCLocker::_total_collections = 0;

#ifdef ASSERT
volatile jint GCLocker::_debug_jni_lock_count = 0;
#endif


#ifdef ASSERT
void GCLocker::verify_critical_count() {
  if (SafepointSynchronize::is_at_safepoint()) {
    assert(!needs_gc() || _debug_jni_lock_count == _jni_lock_count, "must agree");
    int count = 0;
    // Count the number of threads with critical operations in progress
    JavaThreadIteratorWithHandle jtiwh;
    for (; JavaThread *thr = jtiwh.next(); ) {
      if (thr->in_critical()) {
        count++;
      }
    }
    if (_jni_lock_count != count) {
      log_error(gc, verify)("critical counts don't match: %d != %d", _jni_lock_count, count);
      jtiwh.rewind();
      for (; JavaThread *thr = jtiwh.next(); ) {
        if (thr->in_critical()) {
          log_error(gc, verify)(INTPTR_FORMAT " in_critical %d", p2i(thr), thr->in_critical());
        }
      }
    }
    assert(_jni_lock_count == count, "must be equal");
  }
}

// In debug mode track the locking state at all times
void GCLocker::increment_debug_jni_lock_count() {
  assert(_debug_jni_lock_count >= 0, "bad value");
  Atomic::inc(&_debug_jni_lock_count);
}

void GCLocker::decrement_debug_jni_lock_count() {
  assert(_debug_jni_lock_count > 0, "bad value");
  Atomic::dec(&_debug_jni_lock_count);
}
#endif

void GCLocker::log_debug_jni(const char* msg) {
  Log(gc, jni) log;
  if (log.is_debug()) {
    ResourceMark rm; // JavaThread::name() allocates to convert to UTF8
    log.debug("%s Thread \"%s\" %d locked.", msg, Thread::current()->name(), _jni_lock_count);
  }
}

bool GCLocker::is_at_safepoint() {
  return SafepointSynchronize::is_at_safepoint();
}

bool GCLocker::check_active_before_gc() {
  assert(SafepointSynchronize::is_at_safepoint(), "only read at safepoint");
  if (is_active() && !_needs_gc) {
    verify_critical_count();
    _needs_gc = true;
    log_debug_jni("Setting _needs_gc.");
  }
  return is_active();
}

void GCLocker::stall_until_clear() {
  assert(!JavaThread::current()->in_critical(), "Would deadlock");
  MutexLocker   ml(JNICritical_lock);

  if (needs_gc()) {
    log_debug_jni("Allocation failed. Thread stalled by JNI critical section.");
  }

  // Wait for _needs_gc  to be cleared
  while (needs_gc()) {
    JNICritical_lock->wait();
  }
}

bool GCLocker::should_discard(GCCause::Cause cause, uint total_collections) {
  return (cause == GCCause::_gc_locker) &&
         (_total_collections != total_collections);
}

void GCLocker::jni_lock(JavaThread* thread) {
  assert(!thread->in_critical(), "shouldn't currently be in a critical region");
  MutexLocker mu(JNICritical_lock);
  // Block entering threads if we know at least one thread is in a
  // JNI critical region and we need a GC.
  // We check that at least one thread is in a critical region before
  // blocking because blocked threads are woken up by a thread exiting
  // a JNI critical region.
  while (is_active_and_needs_gc() || _doing_gc) {
    JNICritical_lock->wait();
  }
  thread->enter_critical();
  _jni_lock_count++;
  increment_debug_jni_lock_count();
}

void GCLocker::jni_unlock(JavaThread* thread) {
  assert(thread->in_last_critical(), "should be exiting critical region");
  MutexLocker mu(JNICritical_lock);
  _jni_lock_count--;
  decrement_debug_jni_lock_count();
  thread->exit_critical();
  if (needs_gc() && !is_active_internal()) {
    // We're the last thread out. Request a GC.
    // Capture the current total collections, to allow detection of
    // other collections that make this one unnecessary.  The value of
    // total_collections() is only changed at a safepoint, so there
    // must not be a safepoint between the lock becoming inactive and
    // getting the count, else there may be unnecessary GCLocker GCs.
    _total_collections = Universe::heap()->total_collections();
    _doing_gc = true;
    {
      // Must give up the lock while at a safepoint
      MutexUnlocker munlock(JNICritical_lock);
      log_debug_jni("Performing GC after exiting critical section.");
      Universe::heap()->collect(GCCause::_gc_locker);
    }
    _doing_gc = false;
    _needs_gc = false;
    JNICritical_lock->notify_all();
  }
}
