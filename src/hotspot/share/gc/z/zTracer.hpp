/*
 * Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_GC_Z_ZTRACER_HPP
#define SHARE_GC_Z_ZTRACER_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/gcTrace.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/z/zAllocationFlags.hpp"

class ZStatCounter;
class ZStatPhase;
class ZStatSampler;

class ZTracer : public GCTracer {
private:
  static ZTracer* _tracer;

  ZTracer();

  void send_stat_counter(uint32_t counter_id, uint64_t increment, uint64_t value);
  void send_stat_sampler(uint32_t sampler_id, uint64_t value);
  void send_thread_phase(const char* name, const Ticks& start, const Ticks& end);
  void send_page_alloc(size_t size, size_t used, size_t free, size_t cache, bool nonblocking, bool noreserve);

public:
  static ZTracer* tracer();
  static void initialize();

  void report_stat_counter(const ZStatCounter& counter, uint64_t increment, uint64_t value);
  void report_stat_sampler(const ZStatSampler& sampler, uint64_t value);
  void report_thread_phase(const ZStatPhase& phase, const Ticks& start, const Ticks& end);
  void report_thread_phase(const char* name, const Ticks& start, const Ticks& end);
  void report_page_alloc(size_t size, size_t used, size_t free, size_t cache, ZAllocationFlags flags);
};

class ZTraceThreadPhase : public StackObj {
private:
  const Ticks       _start;
  const char* const _name;

public:
  ZTraceThreadPhase(const char* name);
  ~ZTraceThreadPhase();
};

#endif // SHARE_GC_Z_ZTRACER_HPP
