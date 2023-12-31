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

#ifndef SHARE_VM_GC_G1_G1CONCURRENTREFINE_HPP
#define SHARE_VM_GC_G1_G1CONCURRENTREFINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"

// Forward decl
class CardTableEntryClosure;
class G1ConcurrentRefine;
class G1ConcurrentRefineThread;
class outputStream;
class ThreadClosure;

// Helper class for refinement thread management. Used to start, stop and
// iterate over them.
class G1ConcurrentRefineThreadControl {
  G1ConcurrentRefine* _cr;

  G1ConcurrentRefineThread** _threads;
  uint _num_max_threads;

  // Create the refinement thread for the given worker id.
  // If initializing is true, ignore InjectGCWorkerCreationFailure.
  G1ConcurrentRefineThread* create_refinement_thread(uint worker_id, bool initializing);
public:
  G1ConcurrentRefineThreadControl();
  ~G1ConcurrentRefineThreadControl();

  jint initialize(G1ConcurrentRefine* cr, uint num_max_threads);

  // If there is a "successor" thread that can be activated given the current id,
  // activate it.
  void maybe_activate_next(uint cur_worker_id);

  void print_on(outputStream* st) const;
  void worker_threads_do(ThreadClosure* tc);
  void stop();
};

// Controls refinement threads and their activation based on the number of completed
// buffers currently available in the global dirty card queue.
// Refinement threads pick work from the queue based on these thresholds. They are activated
// gradually based on the amount of work to do.
// Refinement thread n activates thread n+1 if the instance of this class determines there
// is enough work available. Threads deactivate themselves if the current amount of
// completed buffers falls below their individual threshold.
class G1ConcurrentRefine : public CHeapObj<mtGC> {
  G1ConcurrentRefineThreadControl _thread_control;
  /*
   * The value of the completed dirty card queue length falls into one of 3 zones:
   * green, yellow, red. If the value is in [0, green) nothing is
   * done, the buffers are left unprocessed to enable the caching effect of the
   * dirtied cards. In the yellow zone [green, yellow) the concurrent refinement
   * threads are gradually activated. In [yellow, red) all threads are
   * running. If the length becomes red (max queue length) the mutators start
   * processing the buffers.
   *
   * There are some interesting cases (when G1UseAdaptiveConcRefinement
   * is turned off):
   * 1) green = yellow = red = 0. In this case the mutator will process all
   *    buffers. Except for those that are created by the deferred updates
   *    machinery during a collection.
   * 2) green = 0. Means no caching. Can be a good way to minimize the
   *    amount of time spent updating remembered sets during a collection.
   */
  size_t _green_zone;
  size_t _yellow_zone;
  size_t _red_zone;
  size_t _min_yellow_zone_size;

  G1ConcurrentRefine(size_t green_zone,
                     size_t yellow_zone,
                     size_t red_zone,
                     size_t min_yellow_zone_size);

  // Update green/yellow/red zone values based on how well goals are being met.
  void update_zones(double update_rs_time,
                    size_t update_rs_processed_buffers,
                    double goal_ms);

  static uint worker_id_offset();
  void maybe_activate_more_threads(uint worker_id, size_t num_cur_buffers);

  jint initialize();
public:
  ~G1ConcurrentRefine();

  // Returns a G1ConcurrentRefine instance if succeeded to create/initialize the
  // G1ConcurrentRefine instance. Otherwise, returns NULL with error code.
  static G1ConcurrentRefine* create(jint* ecode);

  void stop();

  // Adjust refinement thresholds based on work done during the pause and the goal time.
  void adjust(double update_rs_time, size_t update_rs_processed_buffers, double goal_ms);

  size_t activation_threshold(uint worker_id) const;
  size_t deactivation_threshold(uint worker_id) const;
  // Perform a single refinement step. Called by the refinement threads when woken up.
  bool do_refinement_step(uint worker_id);

  // Iterate over all concurrent refinement threads applying the given closure.
  void threads_do(ThreadClosure *tc);

  // Maximum number of refinement threads.
  static uint max_num_threads();

  void print_threads_on(outputStream* st) const;

  size_t green_zone() const      { return _green_zone;  }
  size_t yellow_zone() const     { return _yellow_zone; }
  size_t red_zone() const        { return _red_zone;    }
};

#endif // SHARE_VM_GC_G1_G1CONCURRENTREFINE_HPP
