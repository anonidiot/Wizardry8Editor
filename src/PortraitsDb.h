/*
 * Copyright (C) 2024 Anonymous Idiot
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
#ifndef _PORTRAITS_DB_H__
#define _PORTRAITS_DB_H__

#include <QString>
#include <QVector>

typedef enum
{
    PORTRAIT_GRP_HUMAN,
    PORTRAIT_GRP_ELF,
    PORTRAIT_GRP_DWARF,
    PORTRAIT_GRP_GNOME,
    PORTRAIT_GRP_HOBBIT,
    PORTRAIT_GRP_FAIRY,
    PORTRAIT_GRP_LIZARDMAN,
    PORTRAIT_GRP_DRACON,
    PORTRAIT_GRP_FELPURR,
    PORTRAIT_GRP_RAWULF,
    PORTRAIT_GRP_MOOK,
    PORTRAIT_GRP_NINJA,
    PORTRAIT_GRP_TRYNNIE,
    PORTRAIT_GRP_TRANG,
    PORTRAIT_GRP_UMPANI,
    PORTRAIT_GRP_RAPAX,
    PORTRAIT_GRP_ANDROID,

    PORTRAIT_GRP_SIZE
} portrait_category;

QString getLargePortraitFromPortraitDB( int portraitIdx );
QString getMediumPortraitFromPortraitDB( int portraitIdx );
QString getSmallPortraitFromPortraitDB( int portraitIdx );
QVector<int> getIdsForPortraitCategory( portrait_category category);

#endif /* _PORTRAITS_DB_H__ */
