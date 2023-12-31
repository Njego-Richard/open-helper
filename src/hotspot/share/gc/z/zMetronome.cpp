/*
 * Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.
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
 */

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zMetronome.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/mutexLocker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/timer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/ticks.hpp"

ZMetronome::ZMetronome(uint64_t hz) :
    _monitor(Monitor::leaf, "ZMetronome", false, Monitor::_safepoint_check_never),
    _interval_ms(MILLIUNITS / hz),
    _start_ms(0),
    _nticks(0),
    _stopped(false) {}

uint64_t ZMetronome::nticks() const {
  return _nticks;
}

bool ZMetronome::wait_for_tick() {
  if (_nticks++ == 0) {
    // First tick, set start time
    const Ticks now = Ticks::now();
    _start_ms = TimeHelper::counter_to_millis(now.value());
  }

  for (;;) {
    // We might wake up spuriously from wait, so always recalculate
    // the timeout after a wakeup to see if we need to wait again.
    const Ticks now = Ticks::now();
    const uint64_t now_ms = TimeHelper::counter_to_millis(now.value());
    const uint64_t next_ms = _start_ms + (_interval_ms * _nticks);
    const int64_t timeout_ms = next_ms - now_ms;

    MonitorLockerEx ml(&_monitor, Monitor::_no_safepoint_check_flag);
    if (!_stopped && timeout_ms > 0) {
      // Wait
      ml.wait(Monitor::_no_safepoint_check_flag, timeout_ms);
    } else {
      // Tick
      return !_stopped;
    }
  }
}

void ZMetronome::stop() {
  MonitorLockerEx ml(&_monitor, Monitor::_no_safepoint_check_flag);
  _stopped = true;
  ml.notify();
}
