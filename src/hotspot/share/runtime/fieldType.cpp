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
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/classfile/systemDictionary.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/oopFactory.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/oop.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/typeArrayKlass.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/fieldType.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/signature.hpp"

BasicType FieldType::basic_type(Symbol* signature) {
  return char2type(signature->byte_at(0));
}

// Check if it is a valid array signature
bool FieldType::is_valid_array_signature(Symbol* sig) {
  assert(sig->utf8_length() > 1, "this should already have been checked");
  assert(sig->byte_at(0) == '[', "this should already have been checked");
  // The first character is already checked
  int i = 1;
  int len = sig->utf8_length();
  // First skip all '['s
  while(i < len - 1 && sig->byte_at(i) == '[') i++;

  // Check type
  switch(sig->byte_at(i)) {
    case 'B': // T_BYTE
    case 'C': // T_CHAR
    case 'D': // T_DOUBLE
    case 'F': // T_FLOAT
    case 'I': // T_INT
    case 'J': // T_LONG
    case 'S': // T_SHORT
    case 'Z': // T_BOOLEAN
      // If it is an array, the type is the last character
      return (i + 1 == len);
    case 'L':
      // If it is an object, the last character must be a ';'
      return sig->byte_at(len - 1) == ';';
  }

  return false;
}


BasicType FieldType::get_array_info(Symbol* signature, FieldArrayInfo& fd, TRAPS) {
  assert(basic_type(signature) == T_ARRAY, "must be array");
  int index = 1;
  int dim   = 1;
  while (signature->byte_at(index) == '[') {
    index++;
    dim++;
  }
  ResourceMark rm;
  char *element = signature->as_C_string() + index;
  BasicType element_type = char2type(element[0]);
  if (element_type == T_OBJECT) {
    int len = (int)strlen(element);
    assert(element[len-1] == ';', "last char should be a semicolon");
    element[len-1] = '\0';        // chop off semicolon
    fd._object_key = SymbolTable::new_symbol(element + 1, CHECK_(T_BYTE));
  }
  // Pass dimension back to caller
  fd._dimension = dim;
  return element_type;
}
