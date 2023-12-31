/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
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
#include "logTestUtils.inline.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/logConfiguration.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/logTagSet.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/logging/logTagSetDescriptions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/memory/resourceArea.hpp"
#include "unittest.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/utilities/ostream.hpp"

TEST_VM(LogTagSetDescriptions, describe) {
  for (LogTagSetDescription* d = tagset_descriptions; d->tagset != NULL; d++) {
    char expected[1 * K];
    d->tagset->label(expected, sizeof(expected), "+");
    jio_snprintf(expected + strlen(expected),
                 sizeof(expected) - strlen(expected),
                 ": %s", d->descr);

    ResourceMark rm;
    stringStream stream;
    LogConfiguration::describe(&stream);
    EXPECT_PRED2(string_contains_substring, stream.as_string(), expected)
      << "missing log tag set descriptions in LogConfiguration::describe";
  }
}

TEST_VM(LogTagSetDescriptions, command_line_help) {
  const char* filename = "logtagset_descriptions";
  FILE* fp = fopen(filename, "w+");
  ASSERT_NE((void*)NULL, fp);
  fileStream stream(fp);
  LogConfiguration::print_command_line_help(&stream);
  fclose(fp);

  for (LogTagSetDescription* d = tagset_descriptions; d->tagset != NULL; d++) {
    char expected[1 * K];
    d->tagset->label(expected, sizeof(expected), "+");
    jio_snprintf(expected + strlen(expected),
                 sizeof(expected) - strlen(expected),
                 ": %s", d->descr);

    EXPECT_TRUE(file_contains_substring(filename, expected)) << "missing log tag set descriptions in -Xlog:help output";
  }
  delete_file(filename);
}
