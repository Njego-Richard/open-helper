/*
 * Copyright (c) 1998, 2010, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_UTILITIES_PRESERVEEXCEPTION_HPP
#define SHARE_VM_UTILITIES_PRESERVEEXCEPTION_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/handles.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/thread.hpp"

// This file provides more support for exception handling; see also exceptions.hpp
class PreserveExceptionMark {
 private:
  Thread*     _thread;
  Handle      _preserved_exception_oop;
  int         _preserved_exception_line;
  const char* _preserved_exception_file;

 public:
  PreserveExceptionMark(Thread*& thread);
  ~PreserveExceptionMark();
};


// This is a clone of PreserveExceptionMark which asserts instead
// of failing when what it wraps generates a pending exception.
// It also addresses bug 6431341.
class CautiouslyPreserveExceptionMark {
 private:
  Thread*     _thread;
  Handle      _preserved_exception_oop;
  int         _preserved_exception_line;
  const char* _preserved_exception_file;

 public:
  CautiouslyPreserveExceptionMark(Thread* thread);
  ~CautiouslyPreserveExceptionMark();
};


// Like PreserveExceptionMark but allows new exceptions to be generated in
// the body of the mark. If a new exception is generated then the original one
// is discarded.
class WeakPreserveExceptionMark {
private:
  Thread*     _thread;
  Handle      _preserved_exception_oop;
  int         _preserved_exception_line;
  const char* _preserved_exception_file;

  void        preserve();
  void        restore();

  public:
    WeakPreserveExceptionMark(Thread* pThread) :  _thread(pThread), _preserved_exception_oop()  {
      if (pThread->has_pending_exception()) {
        preserve();
      }
    }
    ~WeakPreserveExceptionMark() {
      if (_preserved_exception_oop.not_null()) {
        restore();
      }
    }
};



// use global exception mark when allowing pending exception to be set and
// saving and restoring them
#define PRESERVE_EXCEPTION_MARK                    Thread* THREAD; PreserveExceptionMark __em(THREAD);

#endif // SHARE_VM_UTILITIES_PRESERVEEXCEPTION_HPP
