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

#include <QDirIterator>
#include "RIFFFile.h"
#include "common.h"

#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
 #define skip read
#endif

RIFFFile::RIFFFile(const QString &name) :
    QFile(name),
    m_error(),
    m_filesize(-1),
    m_numSegs(0)
{
    if (open(QFile::ReadOnly))
    {
        m_filesize = size();

        readDirectory();

        close(); // wait for caller to open us
    }
    else
    {
        m_error = QString( tr("Couldn't open file: %1") ).arg( name );
    }
}

RIFFFile::~RIFFFile()
{
}

QString RIFFFile::getError()
{
    return m_error;
}

// expects that desired LVLS has already been seeked to prior to call
QList<riff_entry> RIFFFile::readLVLS()
{
    QList<riff_entry>  segments;

    int num_segs = readLELong();
#if 0
    qint32 val2 = readLELong(); // val2 is some kind of tag id
#else
    skip(4);
#endif
    for (int k=0; k<num_segs; k++)
    {
        struct riff_entry r;
        quint8  subcode[5];

        read( (char *)subcode, 4 );
        subcode[4] = 0;
        r.code = QString((char *)subcode);

        // rubbish
        skip(2);

        // segment size
        r.size   = readLELong();
        r.offset = pos();

        segments.append(r);

        skip(r.size);
    }
    return segments;
}

void RIFFFile::readDirectory()
{
    quint8  buf[10];

    // RIFF files have a 4 byte header saying 'RIFF'
    read( (char *)buf, 4 );

    // Offset 0
    if ((buf[0] == 'R') &&
        (buf[1] == 'I') &&
        (buf[2] == 'F') &&
        (buf[3] == 'F'))
    {
        // Offset 4
        skip(2);
        // Offset 6
        qint32 filedata = readLELong();

        // 10 is from the current offset
        if (filedata + 10 <= m_filesize)
        {
            m_numSegs = readLELong();

            for (int k=0; k<m_numSegs; k++)
            {
                struct riff_entry r;

                // Segment code
                read( (char *)buf, 4 );
                buf[4] = 0;
                r.code = QString((char *)buf);

                // rubbish
                skip(2);

                // segment size
                r.size   = readLELong();
                r.offset = pos();

                m_segments.append(r);

                skip(r.size);
            }
            if (m_numSegs == 0)
            {
                m_error = tr( "RIFF file doesn't have any segments." );
            }
        }
    }
    else
    {
        m_error = tr( "Save game file doesn't start with the expected RIFF header" );
    }
}

QString RIFFFile::getSegmentCode(int segment)
{
    if (segment < m_segments.size())
    {
        return m_segments.at(segment).code;
    }
    return "";
}

void RIFFFile::seekSegment(int segment)
{
    if (segment < m_segments.size())
    {
        seek(m_segments.at(segment).offset);
    }
}

QByteArray RIFFFile::readParty()
{
    if (seekPartySegment())
    {
        qint32 charOffset = readLEULong();

        return read(charOffset);
    }
    return QByteArray();
}

bool RIFFFile::writeParty(const QByteArray &party)
{
    if (seekPartySegment())
    {
        writeLEULong(party.size());

        if (party.size() == write(party))
            return true;
    }
    return false;
}

QByteArray RIFFFile::readCharacter(int charIdx)
{
    if (seekPartySegment())
    {
        qint32 charOffset = readLEULong();

        skip( charOffset + charIdx*(CHARACTER_SIZE+sizeof(quint32)));

        qint32 charSize = readLEULong();

        if (charSize != CHARACTER_SIZE)
            return QByteArray();

        return read(CHARACTER_SIZE);
    }
    return QByteArray();
}

bool RIFFFile::writeCharacter(int charIdx, const QByteArray &character)
{
    if (seekPartySegment())
    {
        qint32 charOffset = readLEULong();

        skip( charOffset + charIdx*(CHARACTER_SIZE+sizeof(quint32)));

        Q_ASSERT(character.size() == CHARACTER_SIZE);
        writeLELong(character.size());

        if (CHARACTER_SIZE == write(character))
            return true;
    }
    return false;
}

QByteArray RIFFFile::readCharacterExtra(int charIdx)
{
    if (seekPartySegment())
    {
        qint32 charOffset = readLEULong();

        skip( charOffset + 8*(CHARACTER_SIZE+sizeof(quint32)));

        skip( charIdx*(CHARACTER_EXTRA_SIZE+sizeof(quint32)));

        qint32 charxSize = readLELong();

        if (charxSize != CHARACTER_EXTRA_SIZE)
            return QByteArray();

        return read(CHARACTER_EXTRA_SIZE);
    }
    return QByteArray();
}

bool RIFFFile::writeCharacterExtra(int charIdx, const QByteArray &pa)
{
    if (seekPartySegment())
    {
        qint32 charOffset = readLEULong();

        skip( charOffset + 8*(CHARACTER_SIZE+sizeof(quint32)));

        skip( charIdx*(CHARACTER_EXTRA_SIZE+sizeof(quint32)));

        Q_ASSERT(pa.size() == CHARACTER_EXTRA_SIZE);
        writeLELong(pa.size());

        if (CHARACTER_EXTRA_SIZE == write(pa))
            return true;
    }
    return false;
}


bool RIFFFile::seekPartySegment()
{
    for (int k=0; k<m_segments.size(); k++)
    {
        if (m_segments.at(k).code.compare("GSTA") == 0)
        {
            seekSegment( k );

            return true;
        }
    }
    return false;
}

qint8 RIFFFile::readByte()
{
    qint8 b;

    read( (char *)&b, 1 );

    return b;
}

bool RIFFFile::writeByte(qint8 value)
{
    quint8 buf[1];

    ASSIGN_LE8(buf, ((quint8)value));
    if (sizeof(qint8) == write( (char *)buf, 1 ))
        return true;

    return false;
}

quint8 RIFFFile::readUByte()
{
    quint8 b;

    read( (char *)&b, 1 );

    return b;
}

bool RIFFFile::writeUByte(quint8 value)
{
    quint8 buf[1];

    ASSIGN_LE8(buf, value);
    if (sizeof(quint8) == write( (char *)buf, 1 ))
        return true;

    return false;
}

qint16 RIFFFile::readLEShort()
{
    quint8 buf[2];

    read( (char *)buf, 2 );

    return FORMAT_LE16(buf);
}

bool RIFFFile::writeLEShort(qint16 value)
{
    quint8 buf[2];

    ASSIGN_LE16(buf, ((quint16)value));
    if (sizeof(qint16) == write( (char *)buf, 2 ))
        return true;

    return false;
}

quint16 RIFFFile::readLEUShort()
{
    quint8 buf[2];

    read( (char *)buf, 2 );

    return FORMAT_LE16(buf);
}

bool RIFFFile::writeLEUShort(quint16 value)
{
    quint8 buf[2];

    ASSIGN_LE16(buf, value);
    if (sizeof(quint16) == write( (char *)buf, 2 ))
        return true;

    return false;
}

qint32 RIFFFile::readLELong()
{
    quint8 buf[4];

    read( (char *)buf, 4 );

    return FORMAT_LE32(buf);
}

bool RIFFFile::writeLELong(qint32 value)
{
    quint8 buf[4];

    ASSIGN_LE32(buf, ((quint32)value));
    if (sizeof(qint32) == write( (char *)buf, 4 ))
        return true;

    return false;
}

quint32 RIFFFile::readLEULong()
{
    quint8 buf[4];

    read( (char *)buf, 4 );

    return FORMAT_LE32(buf);
}

bool RIFFFile::writeLEULong(quint32 value)
{
    quint8 buf[4];

    ASSIGN_LE32(buf, value);
    if (sizeof(quint32) == write( (char *)buf, 4 ))
        return true;

    return false;
}

bool RIFFFile::isRIFF(QFile &file)
{
    if (file.open(QFile::ReadOnly))
    {
        quint8  buf[4];

        // RIFF files have a 4 byte header saying 'RIFF'
        read( (char *)buf, 4 );

        if ((buf[0] == 'R') &&
            (buf[1] == 'I') &&
            (buf[2] == 'F') &&
            (buf[3] == 'F'))
        {
            return true;
        }
    }
    return false;
}
