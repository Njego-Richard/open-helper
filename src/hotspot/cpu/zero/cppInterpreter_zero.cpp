/*
 * Copyright (c) 2003, 2018, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.
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
#include "asm/assembler.hpp"
#include "interpreter/bytecodeHistogram.hpp"
#include "interpreter/cppInterpreter.hpp"
#include "interpreter/cppInterpreterGenerator.hpp"
#include "interpreter/interpreter.hpp"
#include "interpreter/interpreterRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/arrayOop.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/cpCache.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/methodData.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/method.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/jvmtiExport.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/prims/jvmtiThreadState.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/arguments.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/atomic.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/deoptimization.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/frame.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/interfaceSupport.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/jniHandles.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/orderAccess.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/sharedRuntime.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/stubRoutines.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/synchronizer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/timer.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/vframeArray.hpp"
#include "stack_zero.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"

#ifdef CC_INTERP

#define fixup_after_potential_safepoint()       \
  method = istate->method()

#define CALL_VM_NOCHECK_NOFIX(func)             \
  thread->set_last_Java_frame();                \
  func;                                         \
  thread->reset_last_Java_frame();

#define CALL_VM_NOCHECK(func)                   \
  CALL_VM_NOCHECK_NOFIX(func)                   \
  fixup_after_potential_safepoint()

int CppInterpreter::normal_entry(Method* method, intptr_t UNUSED, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;

  // Allocate and initialize our frame.
  InterpreterFrame *frame = InterpreterFrame::build(method, CHECK_0);
  thread->push_zero_frame(frame);

  // Execute those bytecodes!
  main_loop(0, THREAD);

  // No deoptimized frames on the stack
  return 0;
}

intptr_t narrow(BasicType type, intptr_t result) {
  // mask integer result to narrower return type.
  switch (type) {
    case T_BOOLEAN:
      return result&1;
    case T_BYTE:
      return (intptr_t)(jbyte)result;
    case T_CHAR:
      return (intptr_t)(uintptr_t)(jchar)result;
    case T_SHORT:
      return (intptr_t)(jshort)result;
    case T_OBJECT:  // nothing to do fall through
    case T_ARRAY:
    case T_LONG:
    case T_INT:
    case T_FLOAT:
    case T_DOUBLE:
    case T_VOID:
      return result;
    default:
      ShouldNotReachHere();
      return result; // silence compiler warnings
  }
}


void CppInterpreter::main_loop(int recurse, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();

  // If we are entering from a deopt we may need to call
  // ourself a few times in order to get to our frame.
  if (recurse)
    main_loop(recurse - 1, THREAD);

  InterpreterFrame *frame = thread->top_zero_frame()->as_interpreter_frame();
  interpreterState istate = frame->interpreter_state();
  Method* method = istate->method();

  intptr_t *result = NULL;
  int result_slots = 0;

  while (true) {
    // We can set up the frame anchor with everything we want at
    // this point as we are thread_in_Java and no safepoints can
    // occur until we go to vm mode.  We do have to clear flags
    // on return from vm but that is it.
    thread->set_last_Java_frame();

    // Call the interpreter
    if (JvmtiExport::can_post_interpreter_events())
      BytecodeInterpreter::runWithChecks(istate);
    else
      BytecodeInterpreter::run(istate);
    fixup_after_potential_safepoint();

    // Clear the frame anchor
    thread->reset_last_Java_frame();

    // Examine the message from the interpreter to decide what to do
    if (istate->msg() == BytecodeInterpreter::call_method) {
      Method* callee = istate->callee();

      // Trim back the stack to put the parameters at the top
      stack->set_sp(istate->stack() + 1);

      // Make the call
      Interpreter::invoke_method(callee, istate->callee_entry_point(), THREAD);
      fixup_after_potential_safepoint();

      // Convert the result
      istate->set_stack(stack->sp() - 1);

      // Restore the stack
      stack->set_sp(istate->stack_limit() + 1);

      // Resume the interpreter
      istate->set_msg(BytecodeInterpreter::method_resume);
    }
    else if (istate->msg() == BytecodeInterpreter::more_monitors) {
      int monitor_words = frame::interpreter_frame_monitor_size();

      // Allocate the space
      stack->overflow_check(monitor_words, THREAD);
      if (HAS_PENDING_EXCEPTION)
        break;
      stack->alloc(monitor_words * wordSize);

      // Move the expression stack contents
      for (intptr_t *p = istate->stack() + 1; p < istate->stack_base(); p++)
        *(p - monitor_words) = *p;

      // Move the expression stack pointers
      istate->set_stack_limit(istate->stack_limit() - monitor_words);
      istate->set_stack(istate->stack() - monitor_words);
      istate->set_stack_base(istate->stack_base() - monitor_words);

      // Zero the new monitor so the interpreter can find it.
      ((BasicObjectLock *) istate->stack_base())->set_obj(NULL);

      // Resume the interpreter
      istate->set_msg(BytecodeInterpreter::got_monitors);
    }
    else if (istate->msg() == BytecodeInterpreter::return_from_method) {
      // Copy the result into the caller's frame
      result_slots = type2size[method->result_type()];
      assert(result_slots >= 0 && result_slots <= 2, "what?");
      result = istate->stack() + result_slots;
      break;
    }
    else if (istate->msg() == BytecodeInterpreter::throwing_exception) {
      assert(HAS_PENDING_EXCEPTION, "should do");
      break;
    }
    else if (istate->msg() == BytecodeInterpreter::do_osr) {
      // Unwind the current frame
      thread->pop_zero_frame();

      // Remove any extension of the previous frame
      int extra_locals = method->max_locals() - method->size_of_parameters();
      stack->set_sp(stack->sp() + extra_locals);

      // Jump into the OSR method
      Interpreter::invoke_osr(
        method, istate->osr_entry(), istate->osr_buf(), THREAD);
      return;
    }
    else {
      ShouldNotReachHere();
    }
  }

  // Unwind the current frame
  thread->pop_zero_frame();

  // Pop our local variables
  stack->set_sp(stack->sp() + method->max_locals());

  // Push our result
  for (int i = 0; i < result_slots; i++) {
    // Adjust result to smaller
    union {
      intptr_t res;
      jint res_jint;
    };
    res = result[-i];
    if (result_slots == 1) {
      BasicType t = method->result_type();
      if (is_subword_type(t)) {
        res_jint = (jint)narrow(t, res_jint);
      }
    }
    stack->push(res);
  }
}

int CppInterpreter::native_entry(Method* method, intptr_t UNUSED, TRAPS) {
  // Make sure method is native and not abstract
  assert(method->is_native() && !method->is_abstract(), "should be");

  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();

  // Allocate and initialize our frame
  InterpreterFrame *frame = InterpreterFrame::build(method, CHECK_0);
  thread->push_zero_frame(frame);
  interpreterState istate = frame->interpreter_state();
  intptr_t *locals = istate->locals();

  // Update the invocation counter
  if ((UseCompiler || CountCompiledCalls) && !method->is_synchronized()) {
    MethodCounters* mcs = method->method_counters();
    if (mcs == NULL) {
      CALL_VM_NOCHECK(mcs = InterpreterRuntime::build_method_counters(thread, method));
      if (HAS_PENDING_EXCEPTION)
        goto unwind_and_return;
    }
    InvocationCounter *counter = mcs->invocation_counter();
    counter->increment();
    if (counter->reached_InvocationLimit(mcs->backedge_counter())) {
      CALL_VM_NOCHECK(
        InterpreterRuntime::frequency_counter_overflow(thread, NULL));
      if (HAS_PENDING_EXCEPTION)
        goto unwind_and_return;
    }
  }

  // Lock if necessary
  BasicObjectLock *monitor;
  monitor = NULL;
  if (method->is_synchronized()) {
    monitor = (BasicObjectLock*) istate->stack_base();
    oop lockee = monitor->obj();
    markOop disp = lockee->mark()->set_unlocked();

    monitor->lock()->set_displaced_header(disp);
    if (lockee->cas_set_mark((markOop)monitor, disp) != disp) {
      if (thread->is_lock_owned((address) disp->clear_lock_bits())) {
        monitor->lock()->set_displaced_header(NULL);
      }
      else {
        CALL_VM_NOCHECK(InterpreterRuntime::monitorenter(thread, monitor));
        if (HAS_PENDING_EXCEPTION)
          goto unwind_and_return;
      }
    }
  }

  // Get the signature handler
  InterpreterRuntime::SignatureHandler *handler; {
    address handlerAddr = method->signature_handler();
    if (handlerAddr == NULL) {
      CALL_VM_NOCHECK(InterpreterRuntime::prepare_native_call(thread, method));
      if (HAS_PENDING_EXCEPTION)
        goto unlock_unwind_and_return;

      handlerAddr = method->signature_handler();
      assert(handlerAddr != NULL, "eh?");
    }
    if (handlerAddr == (address) InterpreterRuntime::slow_signature_handler) {
      CALL_VM_NOCHECK(handlerAddr =
        InterpreterRuntime::slow_signature_handler(thread, method, NULL,NULL));
      if (HAS_PENDING_EXCEPTION)
        goto unlock_unwind_and_return;
    }
    handler = \
      InterpreterRuntime::SignatureHandler::from_handlerAddr(handlerAddr);
  }

  // Get the native function entry point
  address function;
  function = method->native_function();
  assert(function != NULL, "should be set if signature handler is");

  // Build the argument list
  stack->overflow_check(handler->argument_count() * 2, THREAD);
  if (HAS_PENDING_EXCEPTION)
    goto unlock_unwind_and_return;

  void **arguments;
  void *mirror; {
    arguments =
      (void **) stack->alloc(handler->argument_count() * sizeof(void **));
    void **dst = arguments;

    void *env = thread->jni_environment();
    *(dst++) = &env;

    if (method->is_static()) {
      istate->set_oop_temp(
        method->constants()->pool_holder()->java_mirror());
      mirror = istate->oop_temp_addr();
      *(dst++) = &mirror;
    }

    intptr_t *src = locals;
    for (int i = dst - arguments; i < handler->argument_count(); i++) {
      ffi_type *type = handler->argument_type(i);
      if (type == &ffi_type_pointer) {
        if (*src) {
          stack->push((intptr_t) src);
          *(dst++) = stack->sp();
        }
        else {
          *(dst++) = src;
        }
        src--;
      }
      else if (type->size == 4) {
        *(dst++) = src--;
      }
      else if (type->size == 8) {
        src--;
        *(dst++) = src--;
      }
      else {
        ShouldNotReachHere();
      }
    }
  }

  // Set up the Java frame anchor
  thread->set_last_Java_frame();

  // Change the thread state to _thread_in_native
  ThreadStateTransition::transition_from_java(thread, _thread_in_native);

  // Make the call
  intptr_t result[4 - LogBytesPerWord];
  ffi_call(handler->cif(), (void (*)()) function, result, arguments);

  // Change the thread state back to _thread_in_Java.
  // ThreadStateTransition::transition_from_native() cannot be used
  // here because it does not check for asynchronous exceptions.
  // We have to manage the transition ourself.
  thread->set_thread_state(_thread_in_native_trans);

  // Make sure new state is visible in the GC thread
  InterfaceSupport::serialize_thread_state(thread);

  // Handle safepoint operations, pending suspend requests,
  // and pending asynchronous exceptions.
  if (SafepointMechanism::poll(thread) ||
      thread->has_special_condition_for_native_trans()) {
    JavaThread::check_special_condition_for_native_trans(thread);
    CHECK_UNHANDLED_OOPS_ONLY(thread->clear_unhandled_oops());
  }

  // Finally we can change the thread state to _thread_in_Java.
  thread->set_thread_state(_thread_in_Java);
  fixup_after_potential_safepoint();

  // Clear the frame anchor
  thread->reset_last_Java_frame();

  // If the result was an oop then unbox it and store it in
  // oop_temp where the garbage collector can see it before
  // we release the handle it might be protected by.
  if (handler->result_type() == &ffi_type_pointer) {
    if (result[0] == 0) {
      istate->set_oop_temp(NULL);
    } else {
      jobject handle = reinterpret_cast<jobject>(result[0]);
      istate->set_oop_temp(JNIHandles::resolve(handle));
    }
  }

  // Reset handle block
  thread->active_handles()->clear();

 unlock_unwind_and_return:

  // Unlock if necessary
  if (monitor) {
    BasicLock *lock = monitor->lock();
    markOop header = lock->displaced_header();
    oop rcvr = monitor->obj();
    monitor->set_obj(NULL);

    if (header != NULL) {
      markOop old_header = markOopDesc::encode(lock);
      if (rcvr->cas_set_mark(header, old_header) != old_header) {
        monitor->set_obj(rcvr); {
          HandleMark hm(thread);
          CALL_VM_NOCHECK(InterpreterRuntime::monitorexit(thread, monitor));
        }
      }
    }
  }

 unwind_and_return:

  // Unwind the current activation
  thread->pop_zero_frame();

  // Pop our parameters
  stack->set_sp(stack->sp() + method->size_of_parameters());

  // Push our result
  if (!HAS_PENDING_EXCEPTION) {
    BasicType type = method->result_type();
    stack->set_sp(stack->sp() - type2size[type]);

    switch (type) {
    case T_VOID:
      break;

    case T_BOOLEAN:
#ifndef VM_LITTLE_ENDIAN
      result[0] <<= (BitsPerWord - BitsPerByte);
#endif
      SET_LOCALS_INT(*(jboolean *) result != 0, 0);
      break;

    case T_CHAR:
#ifndef VM_LITTLE_ENDIAN
      result[0] <<= (BitsPerWord - BitsPerShort);
#endif
      SET_LOCALS_INT(*(jchar *) result, 0);
      break;

    case T_BYTE:
#ifndef VM_LITTLE_ENDIAN
      result[0] <<= (BitsPerWord - BitsPerByte);
#endif
      SET_LOCALS_INT(*(jbyte *) result, 0);
      break;

    case T_SHORT:
#ifndef VM_LITTLE_ENDIAN
      result[0] <<= (BitsPerWord - BitsPerShort);
#endif
      SET_LOCALS_INT(*(jshort *) result, 0);
      break;

    case T_INT:
#ifndef VM_LITTLE_ENDIAN
      result[0] <<= (BitsPerWord - BitsPerInt);
#endif
      SET_LOCALS_INT(*(jint *) result, 0);
      break;

    case T_LONG:
      SET_LOCALS_LONG(*(jlong *) result, 0);
      break;

    case T_FLOAT:
      SET_LOCALS_FLOAT(*(jfloat *) result, 0);
      break;

    case T_DOUBLE:
      SET_LOCALS_DOUBLE(*(jdouble *) result, 0);
      break;

    case T_OBJECT:
    case T_ARRAY:
      SET_LOCALS_OBJECT(istate->oop_temp(), 0);
      break;

    default:
      ShouldNotReachHere();
    }
  }

  // No deoptimized frames on the stack
  return 0;
}

int CppInterpreter::accessor_entry(Method* method, intptr_t UNUSED, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();
  intptr_t *locals = stack->sp();

  // Drop into the slow path if we need a safepoint check
  if (SafepointMechanism::poll(THREAD)) {
    return normal_entry(method, 0, THREAD);
  }

  // Load the object pointer and drop into the slow path
  // if we have a NullPointerException
  oop object = LOCALS_OBJECT(0);
  if (object == NULL) {
    return normal_entry(method, 0, THREAD);
  }

  // Read the field index from the bytecode, which looks like this:
  //  0:  aload_0
  //  1:  getfield
  //  2:    index
  //  3:    index
  //  4:  ireturn/areturn/freturn/lreturn/dreturn
  // NB this is not raw bytecode: index is in machine order
  u1 *code = method->code_base();
  assert(code[0] == Bytecodes::_aload_0 &&
         code[1] == Bytecodes::_getfield &&
         (code[4] == Bytecodes::_ireturn ||
          code[4] == Bytecodes::_freturn ||
          code[4] == Bytecodes::_lreturn ||
          code[4] == Bytecodes::_dreturn ||
          code[4] == Bytecodes::_areturn), "should do");
  u2 index = Bytes::get_native_u2(&code[2]);

  // Get the entry from the constant pool cache, and drop into
  // the slow path if it has not been resolved
  ConstantPoolCache* cache = method->constants()->cache();
  ConstantPoolCacheEntry* entry = cache->entry_at(index);
  if (!entry->is_resolved(Bytecodes::_getfield)) {
    return normal_entry(method, 0, THREAD);
  }

  // Get the result and push it onto the stack
  switch (entry->flag_state()) {
  case ltos:
  case dtos:
    stack->overflow_check(1, CHECK_0);
    stack->alloc(wordSize);
    break;
  }
  if (entry->is_volatile()) {
    if (support_IRIW_for_not_multiple_copy_atomic_cpu) {
      OrderAccess::fence();
    }
    switch (entry->flag_state()) {
    case ctos:
      SET_LOCALS_INT(object->char_field_acquire(entry->f2_as_index()), 0);
      break;

    case btos:
    case ztos:
      SET_LOCALS_INT(object->byte_field_acquire(entry->f2_as_index()), 0);
      break;

    case stos:
      SET_LOCALS_INT(object->short_field_acquire(entry->f2_as_index()), 0);
      break;

    case itos:
      SET_LOCALS_INT(object->int_field_acquire(entry->f2_as_index()), 0);
      break;

    case ltos:
      SET_LOCALS_LONG(object->long_field_acquire(entry->f2_as_index()), 0);
      break;

    case ftos:
      SET_LOCALS_FLOAT(object->float_field_acquire(entry->f2_as_index()), 0);
      break;

    case dtos:
      SET_LOCALS_DOUBLE(object->double_field_acquire(entry->f2_as_index()), 0);
      break;

    case atos:
      SET_LOCALS_OBJECT(object->obj_field_acquire(entry->f2_as_index()), 0);
      break;

    default:
      ShouldNotReachHere();
    }
  }
  else {
    switch (entry->flag_state()) {
    case ctos:
      SET_LOCALS_INT(object->char_field(entry->f2_as_index()), 0);
      break;

    case btos:
    case ztos:
      SET_LOCALS_INT(object->byte_field(entry->f2_as_index()), 0);
      break;

    case stos:
      SET_LOCALS_INT(object->short_field(entry->f2_as_index()), 0);
      break;

    case itos:
      SET_LOCALS_INT(object->int_field(entry->f2_as_index()), 0);
      break;

    case ltos:
      SET_LOCALS_LONG(object->long_field(entry->f2_as_index()), 0);
      break;

    case ftos:
      SET_LOCALS_FLOAT(object->float_field(entry->f2_as_index()), 0);
      break;

    case dtos:
      SET_LOCALS_DOUBLE(object->double_field(entry->f2_as_index()), 0);
      break;

    case atos:
      SET_LOCALS_OBJECT(object->obj_field(entry->f2_as_index()), 0);
      break;

    default:
      ShouldNotReachHere();
    }
  }

  // No deoptimized frames on the stack
  return 0;
}

int CppInterpreter::empty_entry(Method* method, intptr_t UNUSED, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();

  // Drop into the slow path if we need a safepoint check
  if (SafepointMechanism::poll(THREAD)) {
    return normal_entry(method, 0, THREAD);
  }

  // Pop our parameters
  stack->set_sp(stack->sp() + method->size_of_parameters());

  // No deoptimized frames on the stack
  return 0;
}

// The new slots will be inserted before slot insert_before.
// Slots < insert_before will have the same slot number after the insert.
// Slots >= insert_before will become old_slot + num_slots.
void CppInterpreter::insert_vmslots(int insert_before, int num_slots, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();

  // Allocate the space
  stack->overflow_check(num_slots, CHECK);
  stack->alloc(num_slots * wordSize);
  intptr_t *vmslots = stack->sp();

  // Shuffle everything up
  for (int i = 0; i < insert_before; i++)
    SET_VMSLOTS_SLOT(VMSLOTS_SLOT(i + num_slots), i);
}

void CppInterpreter::remove_vmslots(int first_slot, int num_slots, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();
  intptr_t *vmslots = stack->sp();

  // Move everything down
  for (int i = first_slot - 1; i >= 0; i--)
    SET_VMSLOTS_SLOT(VMSLOTS_SLOT(i), i + num_slots);

  // Deallocate the space
  stack->set_sp(stack->sp() + num_slots);
}

BasicType CppInterpreter::result_type_of_handle(oop method_handle) {
  oop method_type = java_lang_invoke_MethodHandle::type(method_handle);
  oop return_type = java_lang_invoke_MethodType::rtype(method_type);
  return java_lang_Class::as_BasicType(return_type, (Klass* *) NULL);
}

intptr_t* CppInterpreter::calculate_unwind_sp(ZeroStack* stack,
                                              oop method_handle) {
  oop method_type = java_lang_invoke_MethodHandle::type(method_handle);
  int argument_slots = java_lang_invoke_MethodType::ptype_slot_count(method_type);

  return stack->sp() + argument_slots;
}

IRT_ENTRY(void, CppInterpreter::throw_exception(JavaThread* thread,
                                                Symbol*     name,
                                                char*       message))
  THROW_MSG(name, message);
IRT_END

InterpreterFrame *InterpreterFrame::build(Method* const method, TRAPS) {
  JavaThread *thread = (JavaThread *) THREAD;
  ZeroStack *stack = thread->zero_stack();

  // Calculate the size of the frame we'll build, including
  // any adjustments to the caller's frame that we'll make.
  int extra_locals  = 0;
  int monitor_words = 0;
  int stack_words   = 0;

  if (!method->is_native()) {
    extra_locals = method->max_locals() - method->size_of_parameters();
    stack_words  = method->max_stack();
  }
  if (method->is_synchronized()) {
    monitor_words = frame::interpreter_frame_monitor_size();
  }
  stack->overflow_check(
    extra_locals + header_words + monitor_words + stack_words, CHECK_NULL);

  // Adjust the caller's stack frame to accomodate any additional
  // local variables we have contiguously with our parameters.
  for (int i = 0; i < extra_locals; i++)
    stack->push(0);

  intptr_t *locals;
  if (method->is_native())
    locals = stack->sp() + (method->size_of_parameters() - 1);
  else
    locals = stack->sp() + (method->max_locals() - 1);

  stack->push(0); // next_frame, filled in later
  intptr_t *fp = stack->sp();
  assert(fp - stack->sp() == next_frame_off, "should be");

  stack->push(INTERPRETER_FRAME);
  assert(fp - stack->sp() == frame_type_off, "should be");

  interpreterState istate =
    (interpreterState) stack->alloc(sizeof(BytecodeInterpreter));
  assert(fp - stack->sp() == istate_off, "should be");

  istate->set_locals(locals);
  istate->set_method(method);
  istate->set_mirror(method->method_holder()->java_mirror());
  istate->set_self_link(istate);
  istate->set_prev_link(NULL);
  istate->set_thread(thread);
  istate->set_bcp(method->is_native() ? NULL : method->code_base());
  istate->set_constants(method->constants()->cache());
  istate->set_msg(BytecodeInterpreter::method_entry);
  istate->set_oop_temp(NULL);
  istate->set_mdx(NULL);
  istate->set_callee(NULL);

  istate->set_monitor_base((BasicObjectLock *) stack->sp());
  if (method->is_synchronized()) {
    BasicObjectLock *monitor =
      (BasicObjectLock *) stack->alloc(monitor_words * wordSize);
    oop object;
    if (method->is_static())
      object = method->constants()->pool_holder()->java_mirror();
    else
      object = (oop) (void*)locals[0];
    monitor->set_obj(object);
  }

  istate->set_stack_base(stack->sp());
  istate->set_stack(stack->sp() - 1);
  if (stack_words)
    stack->alloc(stack_words * wordSize);
  istate->set_stack_limit(stack->sp() - 1);

  return (InterpreterFrame *) fp;
}

InterpreterFrame *InterpreterFrame::build(int size, TRAPS) {
  ZeroStack *stack = ((JavaThread *) THREAD)->zero_stack();

  int size_in_words = size >> LogBytesPerWord;
  assert(size_in_words * wordSize == size, "unaligned");
  assert(size_in_words >= header_words, "too small");
  stack->overflow_check(size_in_words, CHECK_NULL);

  stack->push(0); // next_frame, filled in later
  intptr_t *fp = stack->sp();
  assert(fp - stack->sp() == next_frame_off, "should be");

  stack->push(INTERPRETER_FRAME);
  assert(fp - stack->sp() == frame_type_off, "should be");

  interpreterState istate =
    (interpreterState) stack->alloc(sizeof(BytecodeInterpreter));
  assert(fp - stack->sp() == istate_off, "should be");
  istate->set_self_link(NULL); // mark invalid

  stack->alloc((size_in_words - header_words) * wordSize);

  return (InterpreterFrame *) fp;
}

address CppInterpreter::return_entry(TosState state, int length, Bytecodes::Code code) {
  ShouldNotCallThis();
  return NULL;
}

address CppInterpreter::deopt_entry(TosState state, int length) {
  return NULL;
}

// Helper for figuring out if frames are interpreter frames

bool CppInterpreter::contains(address pc) {
  return false; // make frame::print_value_on work
}
#endif // CC_INTERP
