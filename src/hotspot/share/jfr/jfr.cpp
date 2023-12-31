/*
 * Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
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
#include "jfr/jfr.hpp"
#include "jfr/leakprofiler/leakProfiler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/periodic/sampling/jfrThreadSampler.hpp"
#include "jfr/recorder/jfrRecorder.hpp"
#include "jfr/recorder/checkpoint/jfrCheckpointManager.hpp"
#include "jfr/recorder/repository/jfrEmergencyDump.hpp"
#include "jfr/recorder/service/jfrOptionSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/support/jfrThreadLocal.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/java.hpp"

bool Jfr::is_enabled() {
  return JfrRecorder::is_enabled();
}

bool Jfr::is_disabled() {
  return JfrRecorder::is_disabled();
}

bool Jfr::is_recording() {
  return JfrRecorder::is_recording();
}

void Jfr::on_create_vm_1() {
  if (!JfrRecorder::on_create_vm_1()) {
    vm_exit_during_initialization("Failure when starting JFR on_create_vm_1");
  }
}

void Jfr::on_create_vm_2() {
  if (!JfrRecorder::on_create_vm_2()) {
    vm_exit_during_initialization("Failure when starting JFR on_create_vm_2");
  }
}

void Jfr::on_create_vm_3() {
  if (!JfrRecorder::on_create_vm_3()) {
    vm_exit_during_initialization("Failure when starting JFR on_create_vm_3");
  }
}

void Jfr::on_unloading_classes() {
  if (JfrRecorder::is_created()) {
    JfrCheckpointManager::write_type_set_for_unloaded_classes();
  }
}

void Jfr::on_thread_start(Thread* t) {
  JfrThreadLocal::on_start(t);
}

void Jfr::on_thread_exit(Thread* t) {
  JfrThreadLocal::on_exit(t);
}

void Jfr::on_java_thread_dismantle(JavaThread* jt) {
  if (JfrRecorder::is_recording()) {
    JfrCheckpointManager::write_thread_checkpoint(jt);
  }
}

void Jfr::on_vm_shutdown(bool exception_handler) {
  if (JfrRecorder::is_recording()) {
    JfrEmergencyDump::on_vm_shutdown(exception_handler);
  }
}

void Jfr::weak_oops_do(BoolObjectClosure* is_alive, OopClosure* f) {
  if (LeakProfiler::is_running()) {
    LeakProfiler::oops_do(is_alive, f);
  }
}

bool Jfr::on_flight_recorder_option(const JavaVMOption** option, char* delimiter) {
  return JfrOptionSet::parse_flight_recorder_option(option, delimiter);
}

bool Jfr::on_start_flight_recording_option(const JavaVMOption** option, char* delimiter) {
  return JfrOptionSet::parse_start_flight_recording_option(option, delimiter);
}
