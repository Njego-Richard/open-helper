/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcTimer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/referenceProcessorPhaseTimes.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/referenceProcessor.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/workerDataArray.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/log.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/logStream.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"

#define ASSERT_REF_TYPE(ref_type) assert((ref_type) >= REF_SOFT && (ref_type) <= REF_PHANTOM, \
                                         "Invariant (%d)", (int)ref_type)

#define ASSERT_PHASE(phase) assert((phase) >= ReferenceProcessor::RefPhase1 && \
                                   (phase) < ReferenceProcessor::RefPhaseMax,  \
                                   "Invariant (%d)", (int)phase);

#define ASSERT_SUB_PHASE(phase) assert((phase) >= ReferenceProcessor::SoftRefSubPhase1 && \
                                       (phase) < ReferenceProcessor::RefSubPhaseMax, \
                                       "Invariant (%d)", (int)phase);

static const char* SubPhasesParWorkTitle[ReferenceProcessor::RefSubPhaseMax] = {
       "SoftRef (ms):",
       "SoftRef (ms):",
       "WeakRef (ms):",
       "FinalRef (ms):",
       "FinalRef (ms):",
       "PhantomRef (ms):"
       };

static const char* Phase2ParWorkTitle = "Total (ms):";

static const char* SubPhasesSerWorkTitle[ReferenceProcessor::RefSubPhaseMax] = {
       "SoftRef:",
       "SoftRef:",
       "WeakRef:",
       "FinalRef:",
       "FinalRef:",
       "PhantomRef:"
       };

static const char* Phase2SerWorkTitle = "Total:";

static const char* Indents[6] = {"", "  ", "    ", "      ", "        ", "          "};

static const char* PhaseNames[ReferenceProcessor::RefPhaseMax] = {
       "Reconsider SoftReferences",
       "Notify Soft/WeakReferences",
       "Notify and keep alive finalizable",
       "Notify PhantomReferences"
       };

static const char* ReferenceTypeNames[REF_PHANTOM + 1] = {
       "None", "Other", "SoftReference", "WeakReference", "FinalReference", "PhantomReference"
       };

STATIC_ASSERT((REF_PHANTOM + 1) == ARRAY_SIZE(ReferenceTypeNames));

static const char* phase_enum_2_phase_string(ReferenceProcessor::RefProcPhases phase) {
  ASSERT_PHASE(phase);
  return PhaseNames[phase];
}

static const char* ref_type_2_string(ReferenceType ref_type) {
  ASSERT_REF_TYPE(ref_type);
  return ReferenceTypeNames[ref_type];
}

RefProcWorkerTimeTracker::RefProcWorkerTimeTracker(WorkerDataArray<double>* worker_time, uint worker_id) :
  _worker_time(worker_time), _start_time(os::elapsedTime()), _worker_id(worker_id) {
  assert(worker_time != NULL, "Invariant");
}

RefProcWorkerTimeTracker::~RefProcWorkerTimeTracker() {
  double result = os::elapsedTime() - _start_time;
  _worker_time->set(_worker_id, result);
}

RefProcSubPhasesWorkerTimeTracker::RefProcSubPhasesWorkerTimeTracker(ReferenceProcessor::RefProcSubPhases phase,
                                                                     ReferenceProcessorPhaseTimes* phase_times,
                                                                     uint worker_id) :
  RefProcWorkerTimeTracker(phase_times->sub_phase_worker_time_sec(phase), worker_id) {
}

RefProcSubPhasesWorkerTimeTracker::~RefProcSubPhasesWorkerTimeTracker() {
}

RefProcPhaseTimeBaseTracker::RefProcPhaseTimeBaseTracker(const char* title,
                                                         ReferenceProcessor::RefProcPhases phase_number,
                                                         ReferenceProcessorPhaseTimes* phase_times) :
  _phase_times(phase_times), _start_ticks(), _end_ticks(), _phase_number(phase_number) {
  assert(_phase_times != NULL, "Invariant");

  _start_ticks.stamp();
  if (_phase_times->gc_timer() != NULL) {
    _phase_times->gc_timer()->register_gc_phase_start(title, _start_ticks);
  }
}

Ticks RefProcPhaseTimeBaseTracker::end_ticks() {
  // If ASSERT is defined, the default value of Ticks will be -2.
  if (_end_ticks.value() <= 0) {
    _end_ticks.stamp();
  }

  return _end_ticks;
}

double RefProcPhaseTimeBaseTracker::elapsed_time() {
  jlong end_value = end_ticks().value();

  return TimeHelper::counter_to_millis(end_value - _start_ticks.value());
}

RefProcPhaseTimeBaseTracker::~RefProcPhaseTimeBaseTracker() {
  if (_phase_times->gc_timer() != NULL) {
    Ticks ticks = end_ticks();
    _phase_times->gc_timer()->register_gc_phase_end(ticks);
  }
}

RefProcBalanceQueuesTimeTracker::RefProcBalanceQueuesTimeTracker(ReferenceProcessor::RefProcPhases phase_number,
                                                                 ReferenceProcessorPhaseTimes* phase_times) :
  RefProcPhaseTimeBaseTracker("Balance queues", phase_number, phase_times) {}

RefProcBalanceQueuesTimeTracker::~RefProcBalanceQueuesTimeTracker() {
  double elapsed = elapsed_time();
  phase_times()->set_balance_queues_time_ms(_phase_number, elapsed);
}

RefProcPhaseTimeTracker::RefProcPhaseTimeTracker(ReferenceProcessor::RefProcPhases phase_number,
                                                       ReferenceProcessorPhaseTimes* phase_times) :
  RefProcPhaseTimeBaseTracker(phase_enum_2_phase_string(phase_number), phase_number, phase_times) {
}

RefProcPhaseTimeTracker::~RefProcPhaseTimeTracker() {
  double elapsed = elapsed_time();
  phase_times()->set_phase_time_ms(_phase_number, elapsed);
}

RefProcTotalPhaseTimesTracker::RefProcTotalPhaseTimesTracker(ReferenceProcessor::RefProcPhases phase_number,
                                                             ReferenceProcessorPhaseTimes* phase_times,
                                                             ReferenceProcessor* rp) :
  _rp(rp), RefProcPhaseTimeBaseTracker(phase_enum_2_phase_string(phase_number), phase_number, phase_times) {
}

RefProcTotalPhaseTimesTracker::~RefProcTotalPhaseTimesTracker() {
  double elapsed = elapsed_time();
  phase_times()->set_phase_time_ms(_phase_number, elapsed);
}

ReferenceProcessorPhaseTimes::ReferenceProcessorPhaseTimes(GCTimer* gc_timer, uint max_gc_threads) :
  _gc_timer(gc_timer), _processing_is_mt(false) {

  for (uint i = 0; i < ReferenceProcessor::RefSubPhaseMax; i++) {
    _sub_phases_worker_time_sec[i] = new WorkerDataArray<double>(max_gc_threads, SubPhasesParWorkTitle[i]);
  }
  _phase2_worker_time_sec = new WorkerDataArray<double>(max_gc_threads, Phase2ParWorkTitle);

  reset();
}

inline int ref_type_2_index(ReferenceType ref_type) {
  return ref_type - REF_SOFT;
}

WorkerDataArray<double>* ReferenceProcessorPhaseTimes::sub_phase_worker_time_sec(ReferenceProcessor::RefProcSubPhases sub_phase) const {
  ASSERT_SUB_PHASE(sub_phase);
  return _sub_phases_worker_time_sec[sub_phase];
}

double ReferenceProcessorPhaseTimes::phase_time_ms(ReferenceProcessor::RefProcPhases phase) const {
  ASSERT_PHASE(phase);
  return _phases_time_ms[phase];
}

void ReferenceProcessorPhaseTimes::set_phase_time_ms(ReferenceProcessor::RefProcPhases phase,
                                                     double phase_time_ms) {
  ASSERT_PHASE(phase);
  _phases_time_ms[phase] = phase_time_ms;
}

void ReferenceProcessorPhaseTimes::reset() {
  for (int i = 0; i < ReferenceProcessor::RefSubPhaseMax; i++) {
    _sub_phases_worker_time_sec[i]->reset();
    _sub_phases_total_time_ms[i] = uninitialized();
  }

  for (int i = 0; i < ReferenceProcessor::RefPhaseMax; i++) {
    _phases_time_ms[i] = uninitialized();
    _balance_queues_time_ms[i] = uninitialized();
  }

  _phase2_worker_time_sec->reset();

  for (int i = 0; i < number_of_subclasses_of_ref; i++) {
    _ref_cleared[i] = 0;
    _ref_discovered[i] = 0;
  }

  _total_time_ms = uninitialized();

  _processing_is_mt = false;
}

ReferenceProcessorPhaseTimes::~ReferenceProcessorPhaseTimes() {
  for (int i = 0; i < ReferenceProcessor::RefSubPhaseMax; i++) {
    delete _sub_phases_worker_time_sec[i];
  }
  delete _phase2_worker_time_sec;
}

double ReferenceProcessorPhaseTimes::sub_phase_total_time_ms(ReferenceProcessor::RefProcSubPhases sub_phase) const {
  ASSERT_SUB_PHASE(sub_phase);
  return _sub_phases_total_time_ms[sub_phase];
}

void ReferenceProcessorPhaseTimes::set_sub_phase_total_phase_time_ms(ReferenceProcessor::RefProcSubPhases sub_phase,
                                                                     double time_ms) {
  ASSERT_SUB_PHASE(sub_phase);
  _sub_phases_total_time_ms[sub_phase] = time_ms;
}

void ReferenceProcessorPhaseTimes::add_ref_cleared(ReferenceType ref_type, size_t count) {
  ASSERT_REF_TYPE(ref_type);
  Atomic::add(count, &_ref_cleared[ref_type_2_index(ref_type)]);
}

void ReferenceProcessorPhaseTimes::set_ref_discovered(ReferenceType ref_type, size_t count) {
  ASSERT_REF_TYPE(ref_type);
  _ref_discovered[ref_type_2_index(ref_type)] = count;
}

double ReferenceProcessorPhaseTimes::balance_queues_time_ms(ReferenceProcessor::RefProcPhases phase) const {
  ASSERT_PHASE(phase);
  return _balance_queues_time_ms[phase];
}

void ReferenceProcessorPhaseTimes::set_balance_queues_time_ms(ReferenceProcessor::RefProcPhases phase, double time_ms) {
  ASSERT_PHASE(phase);
  _balance_queues_time_ms[phase] = time_ms;
}

#define TIME_FORMAT "%.1lfms"

void ReferenceProcessorPhaseTimes::print_all_references(uint base_indent, bool print_total) const {
  if (print_total) {
    LogTarget(Debug, gc, phases, ref) lt;

    if (lt.is_enabled()) {
      LogStream ls(lt);
      ls.print_cr("%s%s: " TIME_FORMAT,
                  Indents[base_indent], "Reference Processing", total_time_ms());
    }
  }

  uint next_indent = base_indent + 1;
  print_phase(ReferenceProcessor::RefPhase1, next_indent);
  print_phase(ReferenceProcessor::RefPhase2, next_indent);
  print_phase(ReferenceProcessor::RefPhase3, next_indent);
  print_phase(ReferenceProcessor::RefPhase4, next_indent);

  print_reference(REF_SOFT, next_indent);
  print_reference(REF_WEAK, next_indent);
  print_reference(REF_FINAL, next_indent);
  print_reference(REF_PHANTOM, next_indent);

}

void ReferenceProcessorPhaseTimes::print_reference(ReferenceType ref_type, uint base_indent) const {
  LogTarget(Debug, gc, phases, ref) lt;

  if (lt.is_enabled()) {
    LogStream ls(lt);
    ResourceMark rm;

    ls.print_cr("%s%s:", Indents[base_indent], ref_type_2_string(ref_type));

    uint const next_indent = base_indent + 1;
    int const ref_type_index = ref_type_2_index(ref_type);

    ls.print_cr("%sDiscovered: " SIZE_FORMAT, Indents[next_indent], _ref_discovered[ref_type_index]);
    ls.print_cr("%sCleared: " SIZE_FORMAT, Indents[next_indent], _ref_cleared[ref_type_index]);
  }
}

void ReferenceProcessorPhaseTimes::print_phase(ReferenceProcessor::RefProcPhases phase, uint indent) const {
  double phase_time = phase_time_ms(phase);

  if (phase_time == uninitialized()) {
    return;
  }

  LogTarget(Debug, gc, phases, ref) lt;
  LogStream ls(lt);

  ls.print_cr("%s%s%s " TIME_FORMAT,
              Indents[indent],
              phase_enum_2_phase_string(phase),
              indent == 0 ? "" : ":", /* 0 indent logs don't need colon. */
              phase_time);

  LogTarget(Debug, gc, phases, ref) lt2;
  if (lt2.is_enabled()) {
    LogStream ls(lt2);

    if (_processing_is_mt) {
      print_balance_time(&ls, phase, indent + 1);
    }

    switch (phase) {
      case ReferenceProcessor::RefPhase1:
        print_sub_phase(&ls, ReferenceProcessor::SoftRefSubPhase1, indent + 1);
        break;
      case ReferenceProcessor::RefPhase2:
        print_sub_phase(&ls, ReferenceProcessor::SoftRefSubPhase2, indent + 1);
        print_sub_phase(&ls, ReferenceProcessor::WeakRefSubPhase2, indent + 1);
        print_sub_phase(&ls, ReferenceProcessor::FinalRefSubPhase2, indent + 1);
        break;
      case ReferenceProcessor::RefPhase3:
        print_sub_phase(&ls, ReferenceProcessor::FinalRefSubPhase3, indent + 1);
        break;
      case ReferenceProcessor::RefPhase4:
        print_sub_phase(&ls, ReferenceProcessor::PhantomRefSubPhase4, indent + 1);
        break;
      default:
        ShouldNotReachHere();
    }
    if (phase == ReferenceProcessor::RefPhase2) {
      print_worker_time(&ls, _phase2_worker_time_sec, Phase2SerWorkTitle, indent + 1);
    }
  }
}

void ReferenceProcessorPhaseTimes::print_balance_time(LogStream* ls, ReferenceProcessor::RefProcPhases phase, uint indent) const {
  double balance_time = balance_queues_time_ms(phase);
  if (balance_time != uninitialized()) {
    ls->print_cr("%s%s " TIME_FORMAT, Indents[indent], "Balance queues:", balance_time);
  }
}

void ReferenceProcessorPhaseTimes::print_sub_phase(LogStream* ls, ReferenceProcessor::RefProcSubPhases sub_phase, uint indent) const {
  print_worker_time(ls, _sub_phases_worker_time_sec[sub_phase], SubPhasesSerWorkTitle[sub_phase], indent);
}

void ReferenceProcessorPhaseTimes::print_worker_time(LogStream* ls, WorkerDataArray<double>* worker_time, const char* ser_title, uint indent) const {
  ls->print("%s", Indents[indent]);
  if (_processing_is_mt) {
    worker_time->print_summary_on(ls, true);
    LogTarget(Trace, gc, phases, task) lt;
    if (lt.is_enabled()) {
      LogStream ls2(lt);
      ls2.print("%s", Indents[indent]);
      worker_time->print_details_on(&ls2);
    }
  } else {
    if (worker_time->get(0) != uninitialized()) {
      ls->print_cr("%s " TIME_FORMAT,
                   ser_title,
                   worker_time->get(0) * MILLIUNITS);
    } else {
      ls->print_cr("%s skipped", ser_title);
    }
  }
}

#undef ASSERT_REF_TYPE
#undef ASSERT_SUB_PHASE
#undef ASSERT_PHASE
#undef TIME_FORMAT
