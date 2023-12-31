/*
 * Copyright (c) 1998, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/classLoaderData.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/codeCache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/oopMapCache.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/jniHandles.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/memprofiler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/mutexLocker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/task.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/threadSMR.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vmThread.hpp"

#ifndef PRODUCT

// --------------------------------------------------------
// MemProfilerTask

class MemProfilerTask : public PeriodicTask {
 public:
  MemProfilerTask(int interval_time) : PeriodicTask(interval_time) {}
  void task();
};


void MemProfilerTask::task() {
  MemProfiler::do_trace();
}


//----------------------------------------------------------
// Implementation of MemProfiler

MemProfilerTask* MemProfiler::_task   = NULL;
FILE*            MemProfiler::_log_fp = NULL;


bool MemProfiler::is_active() {
  return _task != NULL;
}


void MemProfiler::engage() {
  const char *log_name = "mprofile.log";
  if (!is_active()) {
    // Create log file
    _log_fp = fopen(log_name , "w+");
    if (_log_fp == NULL) {
      fatal("MemProfiler: Cannot create log file: %s", log_name);
    }
    fprintf(_log_fp, "MemProfiler: sizes are in Kb, time is in seconds since startup\n\n");
    fprintf(_log_fp, "  time, #thr, #cls,  heap,  heap,  perm,  perm,  code, hndls, rescs, oopmp\n");
    fprintf(_log_fp, "                     used, total,  used, total, total, total, total, total\n");
    fprintf(_log_fp, "--------------------------------------------------------------------------\n");

    _task = new MemProfilerTask(MemProfilingInterval);
    _task->enroll();
  }
}


void MemProfiler::disengage() {
  if (!is_active()) return;
  // Do one last trace at disengage time
  do_trace();

  // Close logfile
  fprintf(_log_fp, "MemProfiler detached\n");
  fclose(_log_fp);

  // remove MemProfilerTask
  assert(_task != NULL, "sanity check");
  _task->disenroll();
  delete _task;
  _task = NULL;
}


void MemProfiler::do_trace() {
  // Calculate thread local sizes
  size_t handles_memory_usage    = VMThread::vm_thread()->handle_area()->size_in_bytes();
  size_t resource_memory_usage   = VMThread::vm_thread()->resource_area()->size_in_bytes();
  {
    JavaThreadIteratorWithHandle jtiwh;
    for (; JavaThread *cur = jtiwh.next(); ) {
      handles_memory_usage  += cur->handle_area()->size_in_bytes();
      resource_memory_usage += cur->resource_area()->size_in_bytes();
    }

    // Print trace line in log
    fprintf(_log_fp, "%6.1f,%5d," SIZE_FORMAT_W(5) "," UINTX_FORMAT_W(6) "," UINTX_FORMAT_W(6) ",",
            os::elapsedTime(),
            jtiwh.length(),
            ClassLoaderDataGraph::num_instance_classes(),
            Universe::heap()->used() / K,
            Universe::heap()->capacity() / K);
  }

  fprintf(_log_fp, UINTX_FORMAT_W(6) ",", CodeCache::capacity() / K);

  fprintf(_log_fp, UINTX_FORMAT_W(6) "," UINTX_FORMAT_W(6) ",%6ld\n",
          handles_memory_usage / K,
          resource_memory_usage / K,
          0L);
  fflush(_log_fp);
}

#endif
