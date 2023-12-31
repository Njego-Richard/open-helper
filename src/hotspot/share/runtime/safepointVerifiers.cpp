/*
 * Copyright (c) 1997, 2017, Oracle and/or its affiliates. All rights reserved.
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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/safepointVerifiers.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/shared/collectedHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/universe.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"

// Implementation of NoGCVerifier

#ifdef ASSERT

NoGCVerifier::NoGCVerifier(bool verifygc) {
  _verifygc = verifygc;
  if (_verifygc) {
    CollectedHeap* h = Universe::heap();
    assert(!h->is_gc_active(), "GC active during NoGCVerifier");
    _old_invocations = h->total_collections();
  }
}


NoGCVerifier::~NoGCVerifier() {
  if (_verifygc) {
    CollectedHeap* h = Universe::heap();
    assert(!h->is_gc_active(), "GC active during NoGCVerifier");
    if (_old_invocations != h->total_collections()) {
      fatal("collection in a NoGCVerifier secured function");
    }
  }
}

PauseNoGCVerifier::PauseNoGCVerifier(NoGCVerifier * ngcv) {
  _ngcv = ngcv;
  if (_ngcv->_verifygc) {
    // if we were verifying, then make sure that nothing is
    // wrong before we "pause" verification
    CollectedHeap* h = Universe::heap();
    assert(!h->is_gc_active(), "GC active during NoGCVerifier");
    if (_ngcv->_old_invocations != h->total_collections()) {
      fatal("collection in a NoGCVerifier secured function");
    }
  }
}


PauseNoGCVerifier::~PauseNoGCVerifier() {
  if (_ngcv->_verifygc) {
    // if we were verifying before, then reenable verification
    CollectedHeap* h = Universe::heap();
    assert(!h->is_gc_active(), "GC active during NoGCVerifier");
    _ngcv->_old_invocations = h->total_collections();
  }
}

#endif
