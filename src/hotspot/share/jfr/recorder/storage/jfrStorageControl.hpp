/*
 * Copyright (c) 2012, 2018, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_JFR_RECORDER_STORAGE_JFRSTORAGECONTROL_HPP
#define SHARE_VM_JFR_RECORDER_STORAGE_JFRSTORAGECONTROL_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/jfr/utilities/jfrAllocation.hpp"

class JfrStorageControl : public JfrCHeapObj {
 private:
  size_t _global_count_total;
  size_t _full_count;
  volatile size_t _global_lease_count;
  volatile size_t _dead_count;
  size_t _to_disk_threshold;
  size_t _in_memory_discard_threshold;
  size_t _global_lease_threshold;
  size_t _scavenge_threshold;
  bool _to_disk;

 public:
  JfrStorageControl(size_t global_count_total, size_t in_memory_discard_threshold);

  void set_to_disk(bool enable);
  bool to_disk() const;

  size_t full_count() const;
  size_t increment_full();
  size_t decrement_full();
  void   reset_full();
  bool should_post_buffer_full_message() const;
  bool should_discard() const;

  size_t global_lease_count() const;
  size_t increment_leased();
  size_t decrement_leased();
  bool is_global_lease_allowed() const;

  size_t dead_count() const;
  size_t increment_dead();
  size_t decrement_dead();

  void set_scavenge_threshold(size_t number_of_dead_buffers);
  bool should_scavenge() const;
};

#endif // SHARE_VM_JFR_RECORDER_STORAGE_JFRSTORAGECONTROL_HPP
