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

#ifndef SHARE_VM_JFR_PERIODIC_JFRTHREADCPULOAD_HPP
#define SHARE_VM_JFR_PERIODIC_JFRTHREADCPULOAD_HPP

#include "/workspace/spring-petclinic/openjdk/src/java.base/share/native/include/jni.h"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"

class JavaThread;
class EventThreadCPULoad;

class JfrThreadCPULoadEvent : public AllStatic {
  static int _last_active_processor_count;
 public:
  static jlong get_wallclock_time();
  static int get_processor_count();
  static bool update_event(EventThreadCPULoad& event, JavaThread* thread, jlong cur_wallclock_time, int processor_count);
  static void send_events();
  static void send_event_for_thread(JavaThread* jt);
};

#endif // SHARE_VM_JFR_PERIODIC_JFRTHREADCPULOAD_HPP

