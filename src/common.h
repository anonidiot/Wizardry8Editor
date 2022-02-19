/*
 * Copyright (C) 2022 Anonymous Idiot
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef COMMON_H__
#define COMMON_H__

#include <QtGlobal>

#include <assert.h>

#if Q_ASSERT_IS_BROKEN
 #include <assert.h>
 #undef Q_ASSERT
 #define Q_ASSERT assert
#endif

union itof
{
    float  a;
    qint32 b;
};

#define FORMAT_8(X)      ((*(X+0)<<0))
#define FORMAT_LE16(X)   ((*(X+0)<<0) | (*(X+1)<<8))
#define FORMAT_LE32(X)   ((*(X+0)<<0) | (*(X+1)<<8) | (*(X+2)<<16) | (*(X+3)<<24))
// This is a GCC specific macro
#define FORMAT_FLOAT(X)  ({ union itof z; z.b = FORMAT_LE32(X); z.a; })

#define ASSIGN_LE8(X,V)  { *(X+0) = ((V & 0xff) >>  0); }
#define ASSIGN_LE16(X,V) { *(X+0) = ((V & 0x00ff) >>  0); \
                           *(X+1) = ((V & 0xff00) >>  8); }
#define ASSIGN_LE32(X,V) { *(X+0) = ((V & 0x000000ff) >>  0); \
                           *(X+1) = ((V & 0x0000ff00) >>  8); \
                           *(X+2) = ((V & 0x00ff0000) >> 16); \
                           *(X+3) = ((V & 0xff000000) >> 24); }
#define ASSIGN_FLOAT(X,V)  do { union itof z; z.a = (V); ASSIGN_LE32(X,z.b); } while (0)

// In QImage
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
 #define sizeInBytes byteCount
#endif

// In QList
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
 #define swapItemsAt swap
#endif

#endif /* COMMON_H__ */
