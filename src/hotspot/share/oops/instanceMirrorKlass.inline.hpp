/* Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_OOPS_INSTANCEMIRRORKLASS_INLINE_HPP
#define SHARE_VM_OOPS_INSTANCEMIRRORKLASS_INLINE_HPP

#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/javaClasses.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceKlass.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/instanceMirrorKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/klass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/debug.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/globalDefinitions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/macros.hpp"

template <typename T, class OopClosureType>
void InstanceMirrorKlass::oop_oop_iterate_statics(oop obj, OopClosureType* closure) {
  T* p         = (T*)start_of_static_fields(obj);
  T* const end = p + java_lang_Class::static_oop_field_count(obj);

  for (; p < end; ++p) {
    Devirtualizer::do_oop(closure, p);
  }
}

template <typename T, class OopClosureType>
void InstanceMirrorKlass::oop_oop_iterate(oop obj, OopClosureType* closure) {
  InstanceKlass::oop_oop_iterate<T>(obj, closure);

  if (Devirtualizer::do_metadata(closure)) {
    Klass* klass = java_lang_Class::as_Klass(obj);
    // We'll get NULL for primitive mirrors.
    if (klass != NULL) {
      if (klass->is_instance_klass() && InstanceKlass::cast(klass)->is_anonymous()) {
        // An anonymous class doesn't have its own class loader, so when handling
        // the java mirror for an anonymous class we need to make sure its class
        // loader data is claimed, this is done by calling do_cld explicitly.
        // For non-anonymous classes the call to do_cld is made when the class
        // loader itself is handled.
        Devirtualizer::do_cld(closure, klass->class_loader_data());
      } else {
        Devirtualizer::do_klass(closure, klass);
      }
    } else {
      // We would like to assert here (as below) that if klass has been NULL, then
      // this has been a mirror for a primitive type that we do not need to follow
      // as they are always strong roots.
      // However, we might get across a klass that just changed during CMS concurrent
      // marking if allocation occurred in the old generation.
      // This is benign here, as we keep alive all CLDs that were loaded during the
      // CMS concurrent phase in the class loading, i.e. they will be iterated over
      // and kept alive during remark.
      // assert(java_lang_Class::is_primitive(obj), "Sanity check");
    }
  }

  oop_oop_iterate_statics<T>(obj, closure);
}

template <typename T, class OopClosureType>
void InstanceMirrorKlass::oop_oop_iterate_reverse(oop obj, OopClosureType* closure) {
  InstanceKlass::oop_oop_iterate_reverse<T>(obj, closure);

  InstanceMirrorKlass::oop_oop_iterate_statics<T>(obj, closure);
}

template <typename T, class OopClosureType>
void InstanceMirrorKlass::oop_oop_iterate_statics_bounded(oop obj,
                                                          OopClosureType* closure,
                                                          MemRegion mr) {
  T* p   = (T*)start_of_static_fields(obj);
  T* end = p + java_lang_Class::static_oop_field_count(obj);

  T* const l   = (T*)mr.start();
  T* const h   = (T*)mr.end();
  assert(mask_bits((intptr_t)l, sizeof(T)-1) == 0 &&
         mask_bits((intptr_t)h, sizeof(T)-1) == 0,
         "bounded region must be properly aligned");

  if (p < l) {
    p = l;
  }
  if (end > h) {
    end = h;
  }

  for (;p < end; ++p) {
    Devirtualizer::do_oop(closure, p);
  }
}

template <typename T, class OopClosureType>
void InstanceMirrorKlass::oop_oop_iterate_bounded(oop obj, OopClosureType* closure, MemRegion mr) {
  InstanceKlass::oop_oop_iterate_bounded<T>(obj, closure, mr);

  if (Devirtualizer::do_metadata(closure)) {
    if (mr.contains(obj)) {
      Klass* klass = java_lang_Class::as_Klass(obj);
      // We'll get NULL for primitive mirrors.
      if (klass != NULL) {
        Devirtualizer::do_klass(closure, klass);
      }
    }
  }

  oop_oop_iterate_statics_bounded<T>(obj, closure, mr);
}

#endif // SHARE_VM_OOPS_INSTANCEMIRRORKLASS_INLINE_HPP
