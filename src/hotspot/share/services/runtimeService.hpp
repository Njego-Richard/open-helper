/*
 * Copyright (c) 2003, 2014, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_SERVICES_RUNTIMESERVICE_HPP
#define SHARE_VM_SERVICES_RUNTIMESERVICE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/perfData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/timer.hpp"

class RuntimeService : public AllStatic {
private:
  static PerfCounter* _sync_time_ticks;        // Accumulated time spent getting to safepoints
  static PerfCounter* _total_safepoints;
  static PerfCounter* _safepoint_time_ticks;   // Accumulated time at safepoints
  static PerfCounter* _application_time_ticks; // Accumulated time not at safepoints

  static TimeStamp _safepoint_timer;
  static TimeStamp _app_timer;
  static double _last_safepoint_sync_time_sec;

public:
  static void init();

  static jlong safepoint_sync_time_ms();
  static jlong safepoint_count();
  static jlong safepoint_time_ms();
  static jlong application_time_ms();

  static double last_safepoint_time_sec()      { return _safepoint_timer.seconds(); }
  static double last_application_time_sec()    { return _app_timer.seconds(); }

  // callbacks
  static void record_safepoint_begin() NOT_MANAGEMENT_RETURN;
  static void record_safepoint_synchronized() NOT_MANAGEMENT_RETURN;
  static void record_safepoint_end() NOT_MANAGEMENT_RETURN;
  static void record_application_start() NOT_MANAGEMENT_RETURN;

};

#endif // SHARE_VM_SERVICES_RUNTIMESERVICE_HPP
