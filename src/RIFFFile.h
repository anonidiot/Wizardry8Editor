/*
 * Copyright (C) 2022-2024 Anonymous Idiot
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

#ifndef RIFFFILE_H__
#define RIFFFILE_H__

#include <QDir>
#include <QFile>


struct riff_entry
{
    QString   code;
    qint64    offset;
    qint32    size;
};

class RIFFFile : public QFile
{
    Q_OBJECT

public:
    static const int PARTY_SIZE           = 0x49C2;
    static const int CHARACTER_SIZE       = 0x1862;
    static const int CHARACTER_EXTRA_SIZE = 0x0106;

    RIFFFile(const QString &name);
    ~RIFFFile();

    bool       isGood() { if (m_numSegs != 0) return true; else return false; }
    QString    getError();

    int        getNumSegments();
    QString    getSegmentCode(int segment);
    bool       seekSegment(int segment);
    bool       seekSegment(QString segment_code);
    bool       seekPartySegment();

    QVector<qint32> getVisitedMapsList();

    QByteArray readParty();
    bool       writeParty(const QByteArray &party);
    QByteArray readCharacter(int charIdx);
    bool       writeCharacter(int charIdx, const QByteArray &character);
    QByteArray readCharacterExtra(int charIdx);
    bool       writeCharacterExtra(int charIdx, const QByteArray &pa);
    QByteArray readFacts();
    bool       writeFacts(const QByteArray &fa);

    qint8      readByte();
    quint8     readUByte();
    qint16     readLEShort();
    quint16    readLEUShort();
    qint32     readLELong();
    quint32    readLEULong();
    bool       writeByte(qint8 value);
    bool       writeUByte(quint8 value);
    bool       writeLEShort(qint16 value);
    bool       writeLEUShort(quint16 value);
    bool       writeLELong(qint32 value);
    bool       writeLEULong(quint32 value);


protected:
    bool isRIFF(QFile &file);

private:
    void                readDirectory();
    QList<riff_entry>   readLVLS();

    QString             m_error;
    qint64              m_filesize;
    int                 m_numSegs;
    QList<riff_entry>   m_segments;
    bool                m_WIZ8variant; // file uses alternative WIZ8 header used by Wizardry 1.2.8 insted of RIFF
};

#endif /* RIFFFILE_H__ */
