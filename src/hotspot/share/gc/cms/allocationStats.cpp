/*
 * Copyright (c) 2005, 2015, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/cms/allocationStats.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/ostream.hpp"

// Technically this should be derived from machine speed, and
// ideally it would be dynamically adjusted.
float AllocationStats::_threshold = ((float)CMS_SweepTimerThresholdMillis)/1000;

void AllocationStats::initialize(bool split_birth)   {
  AdaptivePaddedAverage* dummy =
    new (&_demand_rate_estimate) AdaptivePaddedAverage(CMS_FLSWeight,
                                                       CMS_FLSPadding);
  _desired = 0;
  _coal_desired = 0;
  _surplus = 0;
  _bfr_surp = 0;
  _prev_sweep = 0;
  _before_sweep = 0;
  _coal_births = 0;
  _coal_deaths = 0;
  _split_births = (split_birth ? 1 : 0);
  _split_deaths = 0;
  _returned_bytes = 0;
}
