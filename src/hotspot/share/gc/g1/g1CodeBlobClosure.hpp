/*
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_G1_G1CODEBLOBCLOSURE_HPP
#define SHARE_VM_GC_G1_G1CODEBLOBCLOSURE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/gc/g1/g1CollectedHeap.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/iterator.hpp"

class nmethod;

class G1CodeBlobClosure : public CodeBlobClosure {
  class HeapRegionGatheringOopClosure : public OopClosure {
    G1CollectedHeap* _g1h;
    OopClosure* _work;
    nmethod* _nm;

    template <typename T>
    void do_oop_work(T* p);

  public:
    HeapRegionGatheringOopClosure(OopClosure* oc) : _g1h(G1CollectedHeap::heap()), _work(oc), _nm(NULL) {}

    void do_oop(oop* o);
    void do_oop(narrowOop* o);

    void set_nm(nmethod* nm) {
      _nm = nm;
    }
  };

  HeapRegionGatheringOopClosure _oc;
public:
  G1CodeBlobClosure(OopClosure* oc) : _oc(oc) {}

  void do_code_blob(CodeBlob* cb);
};

#endif // SHARE_VM_GC_G1_G1CODEBLOBCLOSURE_HPP

