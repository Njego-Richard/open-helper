/*
 * Copyright (c) 2002, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/aot/aotLoader.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/codeCache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/gcTaskManager.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psCardTable.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psPromotionManager.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psPromotionManager.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psScavenge.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/parallel/psTasks.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/taskqueue.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/iterator.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/universe.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vmThread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/services/management.hpp"

//
// ScavengeRootsTask
//

void ScavengeRootsTask::do_it(GCTaskManager* manager, uint which) {
  assert(ParallelScavengeHeap::heap()->is_gc_active(), "called outside gc");

  PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
  PSScavengeRootsClosure roots_closure(pm);
  PSPromoteRootsClosure  roots_to_old_closure(pm);

  switch (_root_type) {
    case universe:
      Universe::oops_do(&roots_closure);
      break;

    case jni_handles:
      JNIHandles::oops_do(&roots_closure);
      break;

    case threads:
    {
      ResourceMark rm;
      Threads::oops_do(&roots_closure, NULL);
    }
    break;

    case object_synchronizer:
      ObjectSynchronizer::oops_do(&roots_closure);
      break;

    case system_dictionary:
      SystemDictionary::oops_do(&roots_closure);
      break;

    case class_loader_data:
    {
      PSScavengeCLDClosure cld_closure(pm);
      ClassLoaderDataGraph::cld_do(&cld_closure);
    }
    break;

    case management:
      Management::oops_do(&roots_closure);
      break;

    case jvmti:
      JvmtiExport::oops_do(&roots_closure);
      break;


    case code_cache:
      {
        MarkingCodeBlobClosure each_scavengable_code_blob(&roots_to_old_closure, CodeBlobToOopClosure::FixRelocations);
        CodeCache::scavenge_root_nmethods_do(&each_scavengable_code_blob);
        AOTLoader::oops_do(&roots_closure);
      }
      break;

    default:
      fatal("Unknown root type");
  }

  // Do the real work
  pm->drain_stacks(false);
}

//
// ThreadRootsTask
//

void ThreadRootsTask::do_it(GCTaskManager* manager, uint which) {
  assert(ParallelScavengeHeap::heap()->is_gc_active(), "called outside gc");

  PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
  PSScavengeRootsClosure roots_closure(pm);
  MarkingCodeBlobClosure roots_in_blobs(&roots_closure, CodeBlobToOopClosure::FixRelocations);

  _thread->oops_do(&roots_closure, &roots_in_blobs);

  // Do the real work
  pm->drain_stacks(false);
}

//
// StealTask
//

StealTask::StealTask(ParallelTaskTerminator* t) :
  _terminator(t) {}

void StealTask::do_it(GCTaskManager* manager, uint which) {
  assert(ParallelScavengeHeap::heap()->is_gc_active(), "called outside gc");

  PSPromotionManager* pm =
    PSPromotionManager::gc_thread_promotion_manager(which);
  pm->drain_stacks(true);
  guarantee(pm->stacks_empty(),
            "stacks should be empty at this point");

  int random_seed = 17;
  while(true) {
    StarTask p;
    if (PSPromotionManager::steal_depth(which, &random_seed, p)) {
      TASKQUEUE_STATS_ONLY(pm->record_steal(p));
      pm->process_popped_location_depth(p);
      pm->drain_stacks_depth(true);
    } else {
      if (terminator()->offer_termination()) {
        break;
      }
    }
  }
  guarantee(pm->stacks_empty(), "stacks should be empty at this point");
}

//
// OldToYoungRootsTask
//

void OldToYoungRootsTask::do_it(GCTaskManager* manager, uint which) {
  // There are not old-to-young pointers if the old gen is empty.
  assert(!_old_gen->object_space()->is_empty(),
    "Should not be called is there is no work");
  assert(_old_gen != NULL, "Sanity");
  assert(_old_gen->object_space()->contains(_gen_top) || _gen_top == _old_gen->object_space()->top(), "Sanity");
  assert(_stripe_number < ParallelGCThreads, "Sanity");

  {
    PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
    PSCardTable* card_table = ParallelScavengeHeap::heap()->card_table();

    card_table->scavenge_contents_parallel(_old_gen->start_array(),
                                           _old_gen->object_space(),
                                           _gen_top,
                                           pm,
                                           _stripe_number,
                                           _stripe_total);

    // Do the real work
    pm->drain_stacks(false);
  }
}
