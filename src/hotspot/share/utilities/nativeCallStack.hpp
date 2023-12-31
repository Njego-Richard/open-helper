/*
 * Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_UTILITIES_NATIVE_CALL_STACK_HPP
#define SHARE_VM_UTILITIES_NATIVE_CALL_STACK_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/services/nmtCommon.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/ostream.hpp"

/*
 * This class represents a native call path (does not include Java frame)
 *
 * This class is developed in the context of native memory tracking, it can
 * be an useful tool for debugging purpose.
 *
 * For example, following code should print out native call path:
 *
 *   ....
 *   NativeCallStack here;
 *   here.print_on(tty);
 *   ....
 *
 * However, there are a couple of restrictions on this class. If the restrictions are
 * not strictly followed, it may break native memory tracking badly.
 *
 * 1. Number of stack frames to capture, is defined by native memory tracking.
 *    This number has impacts on how much memory to be used by native
 *    memory tracking.
 * 2. The class is strict stack object, no heap or virtual memory can be allocated
 *    from it.
 */
class MemTracker;

class NativeCallStack : public StackObj {
  friend class MemTracker;

private:
  address       _stack[NMT_TrackingStackDepth];
  unsigned int  _hash_value;

  static NativeCallStack EMPTY_STACK;
public:
  NativeCallStack(int toSkip = 0, bool fillStack = false);
  NativeCallStack(address* pc, int frameCount);

  static inline const NativeCallStack& empty_stack() {
    return EMPTY_STACK;
  }

  // if it is an empty stack
  inline bool is_empty() const {
    return _stack[0] == NULL;
  }

  // number of stack frames captured
  int frames() const;

  inline int compare(const NativeCallStack& other) const {
    return memcmp(_stack, other._stack, sizeof(_stack));
  }

  inline bool equals(const NativeCallStack& other) const {
    // compare hash values
    if (hash() != other.hash()) return false;
    // compare each frame
    return compare(other) == 0;
  }

  inline address get_frame(int index) const {
    assert(index >= 0 && index < NMT_TrackingStackDepth, "Index out of bound");
    return _stack[index];
  }

  // Hash code. Any better algorithm?
  unsigned int hash() const;

  void print_on(outputStream* out) const;
  void print_on(outputStream* out, int indent) const;
};

#endif
