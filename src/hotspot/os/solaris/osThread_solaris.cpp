/*
 * Copyright (c) 1998, 2014, Oracle and/or its affiliates. All rights reserved.
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

// no precompiled headers
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/mutexLocker.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/osThread.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/safepoint.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vmThread.hpp"

#include <signal.h>

 // ***************************************************************
 // Platform dependent initialization and cleanup
 // ***************************************************************

void OSThread::pd_initialize() {
  _thread_id                         = 0;
  sigemptyset(&_caller_sigmask);

  _vm_created_thread                 = false;
}

void OSThread::pd_destroy() {
}

// copied from synchronizer.cpp

void OSThread::SR_handler(Thread* thread, ucontext_t* uc) {
  os::Solaris::SR_handler(thread, uc);
}
