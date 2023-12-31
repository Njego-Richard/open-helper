/*
 * Copyright (c) 1999, 2003, Oracle and/or its affiliates. All rights reserved.
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


/*
 * The real verifier now lives in libverifier.so/verifier.dll.
 *
 * This dummy exists so that HotSpot will run with the new
 * libjava.so/java.dll which is where is it accustomed to finding the
 * verifier.
 */

#include "/workspace/spring-petclinic/openjdk/src/java.base/share/native/include/jni.h"

struct struct_class_size_info;
typedef struct struct_class_size_info class_size_info;


JNIIMPORT jboolean
VerifyClass(JNIEnv *env, jclass cb, char *buffer, jint len);

JNIIMPORT jboolean
VerifyClassForMajorVersion(JNIEnv *env, jclass cb, char *buffer, jint len,
                           jint major_version);

JNIEXPORT jboolean
VerifyClassCodes(JNIEnv *env, jclass cb, char *buffer, jint len)
{
    return VerifyClass(env, cb, buffer, len);
}

JNIEXPORT jboolean
VerifyClassCodesForMajorVersion(JNIEnv *env, jclass cb, char *buffer,
                                jint len, jint major_version)
{
    return VerifyClassForMajorVersion(env, cb, buffer, len, major_version);
}
