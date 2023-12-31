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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHWORKGROUP_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHWORKGROUP_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/workgroup.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shenandoah/shenandoahTaskqueue.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"

class ShenandoahObjToScanQueueSet;

class ShenandoahWorkerScope : public StackObj {
private:
  uint      _n_workers;
  WorkGang* _workers;
public:
  ShenandoahWorkerScope(WorkGang* workers, uint nworkers, const char* msg, bool do_check = true);
  ~ShenandoahWorkerScope();
};

class ShenandoahPushWorkerScope : StackObj {
protected:
  uint      _n_workers;
  uint      _old_workers;
  WorkGang* _workers;

public:
  ShenandoahPushWorkerScope(WorkGang* workers, uint nworkers, bool do_check = true);
  ~ShenandoahPushWorkerScope();
};

class ShenandoahPushWorkerQueuesScope : public ShenandoahPushWorkerScope {
private:
  ShenandoahObjToScanQueueSet* _queues;

public:
  ShenandoahPushWorkerQueuesScope(WorkGang* workers, ShenandoahObjToScanQueueSet* queues, uint nworkers, bool do_check = true);
  ~ShenandoahPushWorkerQueuesScope();
};

class ShenandoahWorkGang : public WorkGang {
private:
  bool     _initialize_gclab;
public:
  ShenandoahWorkGang(const char* name,
           uint workers,
           bool are_GC_task_threads,
           bool are_ConcurrentGC_threads) :
    WorkGang(name, workers, are_GC_task_threads, are_ConcurrentGC_threads), _initialize_gclab(false) {
    }

  // Create a GC worker and install it into the work gang.
  // We need to initialize gclab for dynamic allocated workers
  AbstractGangWorker* install_worker(uint which);

  // We allow _active_workers < _total_workers when UseDynamicNumberOfGCThreads is off.
  // We use the same WorkGang for concurrent and parallel processing, and honor
  // ConcGCThreads and ParallelGCThreads settings
  virtual uint active_workers() const {
    assert(_active_workers > 0, "no active worker");
    assert(_active_workers <= _total_workers,
           "_active_workers: %u > _total_workers: %u", _active_workers, _total_workers);
    return _active_workers;
  }

  void set_initialize_gclab() { assert(!_initialize_gclab, "Can only enable once"); _initialize_gclab = true; }
};

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHWORKGROUP_HPP
