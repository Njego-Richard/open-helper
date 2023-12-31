/*
 * Copyright (c) 1997, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/include/jvm.h"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/allocation.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/decoder.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/vmError.hpp"

#ifndef _WINDOWS
#if defined(__APPLE__)
  #include "decoder_machO.hpp"
#elif defined(AIX)
  #include "decoder_aix.hpp"
#else
  #include "decoder_elf.hpp"
#endif

AbstractDecoder*  Decoder::_shared_decoder = NULL;
AbstractDecoder*  Decoder::_error_handler_decoder = NULL;
NullDecoder       Decoder::_do_nothing_decoder;

AbstractDecoder* Decoder::get_shared_instance() {
  assert(shared_decoder_lock()->owned_by_self(), "Require DecoderLock to enter");

  if (_shared_decoder == NULL) {
    _shared_decoder = create_decoder();
  }
  return _shared_decoder;
}

AbstractDecoder* Decoder::get_error_handler_instance() {
  if (_error_handler_decoder == NULL) {
    _error_handler_decoder = create_decoder();
  }
  return _error_handler_decoder;
}


AbstractDecoder* Decoder::create_decoder() {
  AbstractDecoder* decoder;
#if defined (__APPLE__)
  decoder = new (std::nothrow)MachODecoder();
#elif defined(AIX)
  decoder = new (std::nothrow)AIXDecoder();
#else
  decoder = new (std::nothrow)ElfDecoder();
#endif

  if (decoder == NULL || decoder->has_error()) {
    if (decoder != NULL) {
      delete decoder;
    }
    decoder = &_do_nothing_decoder;
  }
  return decoder;
}

inline bool DecoderLocker::is_first_error_thread() {
  return (os::current_thread_id() == VMError::get_first_error_tid());
}

DecoderLocker::DecoderLocker() :
  MutexLockerEx(DecoderLocker::is_first_error_thread() ?
                NULL : Decoder::shared_decoder_lock(),
                Mutex::_no_safepoint_check_flag) {
  _decoder = is_first_error_thread() ?
    Decoder::get_error_handler_instance() : Decoder::get_shared_instance();
  assert(_decoder != NULL, "null decoder");
}

Mutex* Decoder::shared_decoder_lock() {
  assert(SharedDecoder_lock != NULL, "Just check");
  return SharedDecoder_lock;
}

bool Decoder::decode(address addr, char* buf, int buflen, int* offset, const char* modulepath, bool demangle) {
  bool error_handling_thread = os::current_thread_id() == VMError::first_error_tid;
  MutexLockerEx locker(error_handling_thread ? NULL : shared_decoder_lock(),
                       Mutex::_no_safepoint_check_flag);
  AbstractDecoder* decoder = error_handling_thread ?
    get_error_handler_instance(): get_shared_instance();
  assert(decoder != NULL, "null decoder");

  return decoder->decode(addr, buf, buflen, offset, modulepath, demangle);
}

bool Decoder::decode(address addr, char* buf, int buflen, int* offset, const void* base) {
  bool error_handling_thread = os::current_thread_id() == VMError::first_error_tid;
  MutexLockerEx locker(error_handling_thread ? NULL : shared_decoder_lock(),
                       Mutex::_no_safepoint_check_flag);
  AbstractDecoder* decoder = error_handling_thread ?
    get_error_handler_instance(): get_shared_instance();
  assert(decoder != NULL, "null decoder");

  return decoder->decode(addr, buf, buflen, offset, base);
}


bool Decoder::demangle(const char* symbol, char* buf, int buflen) {
  bool error_handling_thread = os::current_thread_id() == VMError::first_error_tid;
  MutexLockerEx locker(error_handling_thread ? NULL : shared_decoder_lock(),
                       Mutex::_no_safepoint_check_flag);
  AbstractDecoder* decoder = error_handling_thread ?
    get_error_handler_instance(): get_shared_instance();
  assert(decoder != NULL, "null decoder");
  return decoder->demangle(symbol, buf, buflen);
}

void Decoder::print_state_on(outputStream* st) {
}

bool Decoder::get_source_info(address pc, char* buf, size_t buflen, int* line) {
  return false;
}

#endif // !_WINDOWS

