/*
 * Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/recorder/checkpoint/types/traceid/jfrTraceIdEpoch.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/safepoint.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/orderAccess.hpp"

// Alternating epochs on each rotation allow for concurrent tagging.
// The regular epoch shift happens only during a safepoint.
// The fence is there only for the emergency dump case which happens outside of safepoint.
bool JfrTraceIdEpoch::_epoch_state = false;
void JfrTraceIdEpoch::shift_epoch() {
  _epoch_state = !_epoch_state;
  if (!SafepointSynchronize::is_at_safepoint()) {
    OrderAccess::fence();
  }
}

