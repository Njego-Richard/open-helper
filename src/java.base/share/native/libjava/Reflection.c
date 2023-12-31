/*
 * Copyright (c) 2001, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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

#include "/workspace/spring-petclinic/openjdk/src/java.base/share/native/include/jni.h"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/include/jvm.h"
#include "jdk_internal_reflect_Reflection.h"

JNIEXPORT jclass JNICALL
Java_jdk_internal_reflect_Reflection_getCallerClass__(JNIEnv *env, jclass unused)
{
    return JVM_GetCallerClass(env);
}

JNIEXPORT jint JNICALL
Java_jdk_internal_reflect_Reflection_getClassAccessFlags(JNIEnv *env, jclass unused, jclass cls)
{
    return JVM_GetClassAccessFlags(env, cls);
}

JNIEXPORT jboolean JNICALL
Java_jdk_internal_reflect_Reflection_areNestMates(JNIEnv *env, jclass unused, jclass current, jclass member)
{
  return JVM_AreNestMates(env, current, member);
}
