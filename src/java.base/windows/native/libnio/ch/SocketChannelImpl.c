/*
 * Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.
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

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include "/workspace/spring-petclinic/openjdk/src/java.base/share/native/include/jni.h"
#include "jni_util.h"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/include/jvm.h"
#include "jlong.h"
#include "sun_nio_ch_SocketChannelImpl.h"

#include "nio.h"
#include "nio_util.h"
#include "net_util.h"


static jfieldID ia_addrID;      /* java.net.InetAddress.address */

JNIEXPORT void JNICALL
Java_sun_nio_ch_SocketChannelImpl_initIDs(JNIEnv *env, jclass cls)
{
    CHECK_NULL(cls = (*env)->FindClass(env, "java/net/InetAddress"));
    CHECK_NULL(ia_addrID = (*env)->GetFieldID(env, cls, "address", "I"));
}

jint
handleSocketError(JNIEnv *env, int errorValue)
{
    NET_ThrowNew(env, errorValue, NULL);
    return IOS_THROWN;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_SocketChannelImpl_checkConnect(JNIEnv *env, jobject this,
                                               jobject fdo, jboolean block)
{
    int optError = 0;
    int result;
    int n = sizeof(int);
    jint fd = fdval(env, fdo);
    fd_set wr, ex;
    struct timeval t = { 0, 0 };

    FD_ZERO(&wr);
    FD_ZERO(&ex);
    FD_SET((u_int)fd, &wr);
    FD_SET((u_int)fd, &ex);

    result = select(fd+1, 0, &wr, &ex, block ? NULL : &t);

    if (result == 0) {  /* timeout */
        return block ? 0 : IOS_UNAVAILABLE;
    } else {
        if (result == SOCKET_ERROR) { /* select failed */
            handleSocketError(env, WSAGetLastError());
            return IOS_THROWN;
        }
    }

    // connection established if writable and no error to check
    if (FD_ISSET(fd, &wr) && !FD_ISSET(fd, &ex)) {
        return 1;
    }

    result = getsockopt((SOCKET)fd,
                        SOL_SOCKET,
                        SO_ERROR,
                        (char *)&optError,
                        &n);
    if (result == SOCKET_ERROR) {
        int lastError = WSAGetLastError();
        if (lastError == WSAEINPROGRESS) {
            return IOS_UNAVAILABLE;
        }
        NET_ThrowNew(env, lastError, "getsockopt");
        return IOS_THROWN;
    }
    if (optError != NO_ERROR) {
        handleSocketError(env, optError);
        return IOS_THROWN;
    }

    return 0;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_SocketChannelImpl_sendOutOfBandData(JNIEnv* env, jclass this,
                                                    jobject fdo, jbyte b)
{
    int n = send(fdval(env, fdo), (const char*)&b, 1, MSG_OOB);
    if (n == SOCKET_ERROR) {
        handleSocketError(env, WSAGetLastError());
        return IOS_THROWN;
    } else {
        return n;
    }
}
