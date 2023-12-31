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

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/decoder.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/nativeCallStack.hpp"

NativeCallStack NativeCallStack::EMPTY_STACK(0, false);

NativeCallStack::NativeCallStack(int toSkip, bool fillStack) :
  _hash_value(0) {

  if (fillStack) {
    // We need to skip the NativeCallStack::NativeCallStack frame if a tail call is NOT used
    // to call os::get_native_stack. A tail call is used if _NMT_NOINLINE_ is not defined
    // (which means this is not a slowdebug build), and we are on 64-bit (except Windows).
    // This is not necessarily a rule, but what has been obvserved to date.
#if (defined(_NMT_NOINLINE_) || defined(_WINDOWS) || !defined(_LP64) || (defined(BSD) && defined (__aarch64__)))
    // Not a tail call.
    toSkip++;
#if (defined(_NMT_NOINLINE_) && defined(BSD) && defined(_LP64))
    // Mac OS X slowdebug builds have this odd behavior where NativeCallStack::NativeCallStack
    // appears as two frames, so we need to skip an extra frame.
    toSkip++;
#endif // Special-case for BSD.
#endif // Not a tail call.
    os::get_native_stack(_stack, NMT_TrackingStackDepth, toSkip);
  } else {
    for (int index = 0; index < NMT_TrackingStackDepth; index ++) {
      _stack[index] = NULL;
    }
  }
}

NativeCallStack::NativeCallStack(address* pc, int frameCount) {
  int frameToCopy = (frameCount < NMT_TrackingStackDepth) ?
    frameCount : NMT_TrackingStackDepth;
  int index;
  for (index = 0; index < frameToCopy; index ++) {
    _stack[index] = pc[index];
  }
  for (; index < NMT_TrackingStackDepth; index ++) {
    _stack[index] = NULL;
  }
  _hash_value = 0;
}

// number of stack frames captured
int NativeCallStack::frames() const {
  int index;
  for (index = 0; index < NMT_TrackingStackDepth; index ++) {
    if (_stack[index] == NULL) {
      break;
    }
  }
  return index;
}

// Hash code. Any better algorithm?
unsigned int NativeCallStack::hash() const {
  uintptr_t hash_val = _hash_value;
  if (hash_val == 0) {
    for (int index = 0; index < NMT_TrackingStackDepth; index++) {
      if (_stack[index] == NULL) break;
      hash_val += (uintptr_t)_stack[index];
    }

    NativeCallStack* p = const_cast<NativeCallStack*>(this);
    p->_hash_value = (unsigned int)(hash_val & 0xFFFFFFFF);
  }
  return _hash_value;
}

void NativeCallStack::print_on(outputStream* out) const {
  print_on(out, 0);
}

// Decode and print this call path
void NativeCallStack::print_on(outputStream* out, int indent) const {
  address pc;
  char    buf[1024];
  int     offset;
  int     line_no;
  if (is_empty()) {
    for (int index = 0; index < indent; index ++) out->print(" ");
    out->print("[BOOTSTRAP]");
  } else {
    for (int frame = 0; frame < NMT_TrackingStackDepth; frame ++) {
      pc = get_frame(frame);
      if (pc == NULL) break;
      // Print indent
      for (int index = 0; index < indent; index ++) out->print(" ");
      if (os::dll_address_to_function_name(pc, buf, sizeof(buf), &offset)) {
        out->print("[" PTR_FORMAT "] %s+0x%x", p2i(pc), buf, offset);
      } else {
        out->print("[" PTR_FORMAT "]", p2i(pc));
      }

      if (Decoder::get_source_info(pc, buf, sizeof(buf), &line_no)) {
        out->print("  (%s:%d)", buf, line_no);
      }
      out->cr();
    }
  }
}
