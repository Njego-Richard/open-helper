/*
 * Copyright (c) 2017, 2018, Red Hat, Inc. All rights reserved.
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

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahThreadLocalData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahWorkGroup.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahTaskqueue.hpp"

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"

ShenandoahWorkerScope::ShenandoahWorkerScope(WorkGang* workers, uint nworkers, const char* msg, bool check) :
  _workers(workers) {
  assert(msg != NULL, "Missing message");

  _n_workers = _workers->update_active_workers(nworkers);
  assert(_n_workers <= nworkers, "Must be");

  log_info(gc, task)("Using %u of %u workers for %s",
    _n_workers, ShenandoahHeap::heap()->max_workers(), msg);

  if (check) {
    ShenandoahHeap::heap()->assert_gc_workers(_n_workers);
  }
}

ShenandoahWorkerScope::~ShenandoahWorkerScope() {
  assert(_workers->active_workers() == _n_workers,
    "Active workers can not be changed within this scope");
}

ShenandoahPushWorkerScope::ShenandoahPushWorkerScope(WorkGang* workers, uint nworkers, bool check) :
  _old_workers(workers->active_workers()),
  _workers(workers) {
  _n_workers = _workers->update_active_workers(nworkers);
  assert(_n_workers <= nworkers, "Must be");

  // bypass concurrent/parallel protocol check for non-regular paths, e.g. verifier, etc.
  if (check) {
    ShenandoahHeap::heap()->assert_gc_workers(_n_workers);
  }
}

ShenandoahPushWorkerScope::~ShenandoahPushWorkerScope() {
  assert(_workers->active_workers() == _n_workers,
    "Active workers can not be changed within this scope");
  // Restore old worker value
  uint nworkers = _workers->update_active_workers(_old_workers);
  assert(nworkers == _old_workers, "Must be able to restore");
}

ShenandoahPushWorkerQueuesScope::ShenandoahPushWorkerQueuesScope(WorkGang* workers, ShenandoahObjToScanQueueSet* queues, uint nworkers, bool check) :
  ShenandoahPushWorkerScope(workers, nworkers, check), _queues(queues) {
  _queues->reserve(_n_workers);
}

ShenandoahPushWorkerQueuesScope::~ShenandoahPushWorkerQueuesScope() {
  // Restore old worker value
  _queues->reserve(_old_workers);
}

AbstractGangWorker* ShenandoahWorkGang::install_worker(uint which) {
  AbstractGangWorker* worker = WorkGang::install_worker(which);
  ShenandoahThreadLocalData::create(worker);
  if (_initialize_gclab) {
    ShenandoahThreadLocalData::initialize_gclab(worker);
  }
  return worker;
}
