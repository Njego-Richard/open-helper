/*
 * Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
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
 */

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include "/workspace/spring-petclinic/openjdk/src/java.base/share/native/include/jni.h"
#include "jni_util.h"
#include <windows.h>

JNIEXPORT jlong JNICALL Java_jdk_test_lib_util_FileUtils_getWinProcessHandleCount(JNIEnv *env)
{
    DWORD handleCount;
    HANDLE handle = GetCurrentProcess();
    if (GetProcessHandleCount(handle, &handleCount)) {
        return (jlong)handleCount;
    } else {
        return -1L;
    }
}

#endif  /*  _WIN32 */
