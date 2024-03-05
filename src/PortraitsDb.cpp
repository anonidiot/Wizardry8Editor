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

#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QVector>
#include <QVectorIterator>
#include <QDebug>

#include "PortraitsDb.h"
#include "SLFFile.h"
#include "common.h"

bool s_initFailed         = false;
bool s_portraitsAvailable = false;

QVector<int>          s_portraitIds[ PORTRAIT_GRP_SIZE ];
QVector<QString>      s_largePortraitPaths;
QVector<QString>      s_mediumPortraitPaths;
QVector<QString>      s_smallPortraitPaths;

static void loadPortraitsDb();
static bool parsePortraitDb( QString filename );

void loadPortraitsDb()
{
    // For now _only_ supporting %WIZDIR%/USER/PORTRAITS and _not_
    // %MOD%/Data/DATABASES/PortraitsSTI.dat
    // TODO: Haven't even determined which gets precedence, or is
    // it both?

    QDir         userDir( SLFFile::getWizardryPath() );
    QStringList  filter;

    filter << "User";
    QStringList entries = userDir.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

    if (entries.size() == 1)
    {
        userDir.cd( entries.at(0) );

        QDirIterator it( userDir, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString file = it.next();

            if (file.compare( userDir.absoluteFilePath( "PortraitsSTI.dat" ), Qt::CaseInsensitive ) == 0)
            {
//                qDebug() << "Found" << file;
                if (parsePortraitDb( file ))
                {
                    s_portraitsAvailable = true;
                    return;
                }
            }
        }
    }
    s_initFailed = true;
}

bool parsePortraitDb( QString filename )
{
    QByteArray all;

    if (!filename.isEmpty())
    {
        QFile      portraitDb(filename);

        if (portraitDb.open(QFile::ReadOnly))
        {
            all = portraitDb.readAll();
            portraitDb.close();
        }
    }
    // I've only figured out some of the structure of this file.
    // But hopefully enough for our limited usage here. We only
    // need to read it currently, not update it.

    const quint8 *data = (const quint8 *) all.constData();
    const quint8 *d_ptr = data;

    // Unfortunately it doesn't appear to be a length prefixed
    // size of file that we can ensure we have the right number
    // of bytes for right from the start, so this makes for some
    // annoying constant checking to make sure we aren't going
    // to access beyond the end of the buffer.

    // I don't understand the first section at all
    // So skip it entirely
    if (all.size() < 8)
        return false;

    quint32 numUnknowns = FORMAT_LE32(d_ptr+4);

    d_ptr += 8 + numUnknowns * 2;

    // Next bit is a categorisation of the portraits relative to
    // the lists they appear in. The order of indexes is also
    // specified here, and may differ to the order of the filepath
    // strings in the file

    quint32 total = 0;
    for (int k=0; k< PORTRAIT_GRP_SIZE; k++)
    {
        if (d_ptr - data + 4 > all.size())
            return false;

        // How many new portraits being added per race.
        // These are the generic races and the separate category
        // for Ninja - as used in ScreenPersonality. ie.
        //  Humans, Elves, Dwarves, Gnomes, Hobbits, Fairies,
        //  Lizardmen, Dracons, Felpurrs, Rawulfs, Mook, Ninja,
        //  Trynnie, T'Rang, Umpani, Rapax, Android

        // Some of these are going to be 0

        quint32 raceCnt = FORMAT_LE32(d_ptr);

        total += raceCnt;

        d_ptr += 4;

        if (d_ptr - data + raceCnt*4 < all.size())
        {
            for (int j=0; j<(int)raceCnt; j++)
            {
                quint32 val = FORMAT_LE32(d_ptr);

                d_ptr += 4;
                s_portraitIds[k] << val;
            }
        }
        else
        {
            return false;
        }
    }

    // Skip to next section. Sections are broken up by varying numbers of
    // 0xffs. Unsure why it seems to be different in different places, doesn't
    // seem to be for alignment

    // Shouldn't be any left over bytes in our section if we've parsed it
    // right, but check to be safe
    while (1)
    {
        if (d_ptr - data + 1 > all.size())
            return false;

        if (*d_ptr != 0xff)
            d_ptr++;
        else
            break;
    }

    // Should be 8 of them
    while (1)
    {
        if (d_ptr - data + 1 > all.size())
            return false;

        if (*d_ptr == 0xff)
            d_ptr++;
        else
            break;
    }

    quint32 num_pics = 0;
    if (d_ptr - data + 4 < all.size())
    {
        num_pics = FORMAT_LE32(d_ptr);

        d_ptr += 4;

        if (total != num_pics)
        {
            qWarning() << "Mismatch in number of portraits" << total << "!=" << num_pics;
        }

        // There are 4 bytes per portrait here.
        // Byte 0 - always 00
        // Byte 1 - portrait group (so some redundancy here)
        // Byte 2 - sex (0=Male, 1=Female)
        // Byte 3 - facing (0=FrontOn, 1=Looking to Screen Left, 2=Looking to Screen Right)

        d_ptr += 4 * num_pics;
        if (d_ptr - data > all.size())
            return false;
    }
    // Skip to next section.

    // Shouldn't be any left over bytes in our section if we've parsed it
    // right, but check to be safe
    while (1)
    {
        if (d_ptr - data + 1 > all.size())
            return false;

        if (*d_ptr != 0xff)
            d_ptr++;
        else
            break;
    }

    // Should be 12 of them
    while (1)
    {
        if (d_ptr - data + 1 > all.size())
            return false;

        if (*d_ptr == 0xff)
            d_ptr++;
        else
            break;
    }

    // completely skip this section which relates to animation
    d_ptr += num_pics;

    // Shouldn't be any left over bytes in our section if we've parsed it
    // right, but check to be safe
    while (1)
    {
        if (d_ptr - data + 1 > all.size())
            return false;

        if (*d_ptr != 0xff)
            d_ptr++;
        else
            break;
    }

    // Should be 12 of them
    while (1)
    {
        if (d_ptr - data + 1 > all.size())
            return false;

        if (*d_ptr == 0xff)
            d_ptr++;
        else
            break;
    }

    // No idea why the number here is 1 more than the number of pictures.
    // Ignore it.
    d_ptr += 4;

    if (d_ptr - data + 3*60*num_pics > all.size())
        return false;

    for (int k=0; k<(int)num_pics; k++)
    {
        s_largePortraitPaths << QString((const char *)d_ptr);
        d_ptr += 60;
    }
    for (int k=0; k<(int)num_pics; k++)
    {
        s_mediumPortraitPaths << QString((const char *)d_ptr);
        d_ptr += 60;
    }
    for (int k=0; k<(int)num_pics; k++)
    {
        s_smallPortraitPaths << QString((const char *)d_ptr);
        d_ptr += 60;
    }
    return true;
}

QString getLargePortraitFromPortraitDB( int portraitIdx )
{
    if (!s_portraitsAvailable && !s_initFailed)
        loadPortraitsDb();

    if (s_portraitsAvailable && (portraitIdx < s_largePortraitPaths.size()))
        return s_largePortraitPaths[ portraitIdx ];

    return "";
}

QString getMediumPortraitFromPortraitDB( int portraitIdx )
{
    if (!s_portraitsAvailable && !s_initFailed)
        loadPortraitsDb();

    if (s_portraitsAvailable && (portraitIdx < s_mediumPortraitPaths.size()))
        return s_mediumPortraitPaths[ portraitIdx ];

    return "";
}

QString getSmallPortraitFromPortraitDB( int portraitIdx )
{
    if (!s_portraitsAvailable && !s_initFailed)
        loadPortraitsDb();

    if (s_portraitsAvailable && (portraitIdx < s_smallPortraitPaths.size()))
        return s_smallPortraitPaths[ portraitIdx ];

    return "";
}

QVector<int> getIdsForPortraitCategory(portrait_category category)
{
    if (category < PORTRAIT_GRP_SIZE)
    {
        if (!s_portraitsAvailable && !s_initFailed)
            loadPortraitsDb();

        return s_portraitIds[ category ];
    }
    return QVector<int>();
}
