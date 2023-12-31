/*
 * Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_OOPS_METHODCOUNTERS_HPP
#define SHARE_VM_OOPS_METHODCOUNTERS_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/metadata.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/compiler/compilerDefinitions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/compiler/compilerOracle.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/invocationCounter.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/arguments.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/align.hpp"

class MethodCounters : public Metadata {
 friend class VMStructs;
 friend class JVMCIVMStructs;
 private:
  // If you add a new field that points to any metaspace object, you
  // must add this field to MethodCounters::metaspace_pointers_do().
#if INCLUDE_AOT
  Method*           _method;                     // Back link to Method
#endif
#if COMPILER2_OR_JVMCI
  int               _interpreter_invocation_count; // Count of times invoked (reused as prev_event_count in tiered)
  u2                _interpreter_throwout_count; // Count of times method was exited via exception while interpreting
#endif
#if INCLUDE_JVMTI
  u2                _number_of_breakpoints;      // fullspeed debugging support
#endif
  InvocationCounter _invocation_counter;         // Incremented before each activation of the method - used to trigger frequency-based optimizations
  InvocationCounter _backedge_counter;           // Incremented before each backedge taken - used to trigger frequencey-based optimizations
  // NMethod age is a counter for warm methods detection in the code cache sweeper.
  // The counter is reset by the sweeper and is decremented by some of the compiled
  // code. The counter values are interpreted as follows:
  // 1. (HotMethodDetection..INT_MAX] - initial value, no counters inserted
  // 2. [1..HotMethodDetectionLimit)  - the method is warm, the counter is used
  //                                    to figure out which methods can be flushed.
  // 3. (INT_MIN..0]                  - method is hot and will deopt and get
  //                                    recompiled without the counters
  int               _nmethod_age;
  int               _interpreter_invocation_limit;        // per-method InterpreterInvocationLimit
  int               _interpreter_backward_branch_limit;   // per-method InterpreterBackwardBranchLimit
  int               _interpreter_profile_limit;           // per-method InterpreterProfileLimit
  int               _invoke_mask;                         // per-method Tier0InvokeNotifyFreqLog
  int               _backedge_mask;                       // per-method Tier0BackedgeNotifyFreqLog
#ifdef TIERED
  float             _rate;                        // Events (invocation and backedge counter increments) per millisecond
  jlong             _prev_time;                   // Previous time the rate was acquired
  u1                _highest_comp_level;          // Highest compile level this method has ever seen.
  u1                _highest_osr_comp_level;      // Same for OSR level
#endif

  MethodCounters(const methodHandle& mh) :
#if INCLUDE_AOT
                                    _method(mh()),
#endif
                                    _nmethod_age(INT_MAX)
#ifdef TIERED
                                 , _rate(0),
                                   _prev_time(0),
                                   _highest_comp_level(0),
                                   _highest_osr_comp_level(0)
#endif
  {
    set_interpreter_invocation_count(0);
    set_interpreter_throwout_count(0);
    JVMTI_ONLY(clear_number_of_breakpoints());
    invocation_counter()->init();
    backedge_counter()->init();

    if (StressCodeAging) {
      set_nmethod_age(HotMethodDetectionLimit);
    }

    // Set per-method thresholds.
    double scale = 1.0;
    CompilerOracle::has_option_value(mh, "CompileThresholdScaling", scale);

    int compile_threshold = CompilerConfig::scaled_compile_threshold(CompileThreshold, scale);
    _interpreter_invocation_limit = compile_threshold << InvocationCounter::count_shift;
    if (ProfileInterpreter) {
      // If interpreter profiling is enabled, the backward branch limit
      // is compared against the method data counter rather than an invocation
      // counter, therefore no shifting of bits is required.
      _interpreter_backward_branch_limit = (compile_threshold * (OnStackReplacePercentage - InterpreterProfilePercentage)) / 100;
    } else {
      _interpreter_backward_branch_limit = ((compile_threshold * OnStackReplacePercentage) / 100) << InvocationCounter::count_shift;
    }
    _interpreter_profile_limit = ((compile_threshold * InterpreterProfilePercentage) / 100) << InvocationCounter::count_shift;
    _invoke_mask = right_n_bits(CompilerConfig::scaled_freq_log(Tier0InvokeNotifyFreqLog, scale)) << InvocationCounter::count_shift;
    _backedge_mask = right_n_bits(CompilerConfig::scaled_freq_log(Tier0BackedgeNotifyFreqLog, scale)) << InvocationCounter::count_shift;
  }

 public:
  virtual bool is_methodCounters() const volatile { return true; }

  static MethodCounters* allocate(const methodHandle& mh, TRAPS);

  void deallocate_contents(ClassLoaderData* loader_data) {}

  AOT_ONLY(Method* method() const { return _method; })

  static int method_counters_size() {
    return align_up((int)sizeof(MethodCounters), wordSize) / wordSize;
  }
  virtual int size() const {
    return method_counters_size();
  }
  void metaspace_pointers_do(MetaspaceClosure* it);
  MetaspaceObj::Type type() const { return MethodCountersType; }
  void clear_counters();

#if COMPILER2_OR_JVMCI

  int interpreter_invocation_count() {
    return _interpreter_invocation_count;
  }
  void set_interpreter_invocation_count(int count) {
    _interpreter_invocation_count = count;
  }
  int increment_interpreter_invocation_count() {
    return ++_interpreter_invocation_count;
  }

  void interpreter_throwout_increment() {
    if (_interpreter_throwout_count < 65534) {
      _interpreter_throwout_count++;
    }
  }
  int  interpreter_throwout_count() const {
    return _interpreter_throwout_count;
  }
  void set_interpreter_throwout_count(int count) {
    _interpreter_throwout_count = count;
  }

#else // COMPILER2_OR_JVMCI

  int interpreter_invocation_count() {
    return 0;
  }
  void set_interpreter_invocation_count(int count) {
    assert(count == 0, "count must be 0");
  }

  int  interpreter_throwout_count() const {
    return 0;
  }
  void set_interpreter_throwout_count(int count) {
    assert(count == 0, "count must be 0");
  }

#endif // COMPILER2_OR_JVMCI

#if INCLUDE_JVMTI
  u2   number_of_breakpoints() const   { return _number_of_breakpoints; }
  void incr_number_of_breakpoints()    { ++_number_of_breakpoints; }
  void decr_number_of_breakpoints()    { --_number_of_breakpoints; }
  void clear_number_of_breakpoints()   { _number_of_breakpoints = 0; }
#endif

#ifdef TIERED
  jlong prev_time() const                        { return _prev_time; }
  void set_prev_time(jlong time)                 { _prev_time = time; }
  float rate() const                             { return _rate; }
  void set_rate(float rate)                      { _rate = rate; }
#endif

  int highest_comp_level() const;
  void set_highest_comp_level(int level);
  int highest_osr_comp_level() const;
  void set_highest_osr_comp_level(int level);

  // invocation counter
  InvocationCounter* invocation_counter() { return &_invocation_counter; }
  InvocationCounter* backedge_counter()   { return &_backedge_counter; }

  int nmethod_age() {
    return _nmethod_age;
  }
  void set_nmethod_age(int age) {
    _nmethod_age = age;
  }
  void reset_nmethod_age() {
    set_nmethod_age(HotMethodDetectionLimit);
  }

  static bool is_nmethod_hot(int age)       { return age <= 0; }
  static bool is_nmethod_warm(int age)      { return age < HotMethodDetectionLimit; }
  static bool is_nmethod_age_unset(int age) { return age > HotMethodDetectionLimit; }

  static ByteSize nmethod_age_offset() {
    return byte_offset_of(MethodCounters, _nmethod_age);
  }

#if COMPILER2_OR_JVMCI

  static ByteSize interpreter_invocation_counter_offset() {
    return byte_offset_of(MethodCounters, _interpreter_invocation_count);
  }

  static int interpreter_invocation_counter_offset_in_bytes() {
    return offset_of(MethodCounters, _interpreter_invocation_count);
  }

#else // COMPILER2_OR_JVMCI

  static ByteSize interpreter_invocation_counter_offset() {
    ShouldNotReachHere();
    return in_ByteSize(0);
  }

#endif // COMPILER2_OR_JVMCI

  static ByteSize invocation_counter_offset()    {
    return byte_offset_of(MethodCounters, _invocation_counter);
  }

  static ByteSize backedge_counter_offset()      {
    return byte_offset_of(MethodCounters, _backedge_counter);
  }

  static ByteSize interpreter_invocation_limit_offset() {
    return byte_offset_of(MethodCounters, _interpreter_invocation_limit);
  }

  static ByteSize interpreter_backward_branch_limit_offset() {
    return byte_offset_of(MethodCounters, _interpreter_backward_branch_limit);
  }

  static ByteSize interpreter_profile_limit_offset() {
    return byte_offset_of(MethodCounters, _interpreter_profile_limit);
  }

  static ByteSize invoke_mask_offset() {
    return byte_offset_of(MethodCounters, _invoke_mask);
  }

  static ByteSize backedge_mask_offset() {
    return byte_offset_of(MethodCounters, _backedge_mask);
  }

  virtual const char* internal_name() const { return "{method counters}"; }
  virtual void print_value_on(outputStream* st) const;

};
#endif //SHARE_VM_OOPS_METHODCOUNTERS_HPP
