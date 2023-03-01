/*
 * Copyright (C) 2022-2023 Anonymous Idiot
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

#ifndef SLFFILE_H__
#define SLFFILE_H__

#include <QDir>
#include <QException>
#include <QFile>

// This class is intended to behave in the same fashion as QFile()
// It doesn't inherit from QFile() though due to a bug in the QIODevice
// parent class - despite allowing both the size() and pos() virtual methods
// to be overridden by children, QIODevice() persists in direct referencing
// the internal d->pos variable instead of calling the pos() function, which
// prevents us from subclassing it usefully for our purpose.

class SLFFile
{
public:
    SLFFile(const QString &folder, const QString &slfFile, const QString &name);
    SLFFile(const QString &slfFile, const QString &name);
    SLFFile(const QString &name) : SLFFile(QString("DATA.SLF"), name) {};
    ~SLFFile();

    static QPixmap    getPixmapFromSlf( QString slfFile, int idx );

    bool       isGood();

    void       setFileName(const QString &name);
    QString    fileName();
    bool       open(QFile::OpenMode flags);
    void       close();

    bool       seek(qint64 offset);

    qint64     size();

    qint64     skip(qint64 bytes);
    qint64     read(char *buf, qint64 bytes);
    QByteArray read(qint64 bytes);
    QByteArray readAll();

    qint8      readByte();
    quint8     readUByte();
    qint16     readLEShort();
    quint16    readLEUShort();
    qint32     readLELong();
    quint32    readLEULong();
    float      readFloat();

    qint64     pos();

    static void setWizardryPath(QString path);
    static QString &getWizardryPath();

protected:
    void init(const QString &name);
    bool isSlf(QFile &file);
    bool containsFile(QFile &file, const QString &filename);

private:
    void seekToFile();

    QDir       m_wizardryPath;
    QString    m_subfolder;
    QString    m_slf;

    bool       m_in_slf;
    QString    m_filename;
    QFile     *m_storage;
    quint32    m_dataOffset;     /** offset in SLF file that actual data for file starts at */
    qint64     m_dataLen;        /** size of file being accessed INSDE the archive - actual data size */
};

class SLFFileException : public QException
{
public:
    void raise() const override { throw *this; }
    SLFFileException *clone() const override { return new SLFFileException(*this); }
};
#endif /* SLFFILE_H__ */
