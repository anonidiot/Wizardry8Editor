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
#include <QPixmap>

#include "SLFFile.h"
#include "STItoQImage.h"
#include "common.h"

#include <QDebug>

static QString                   s_wizardryPath;
static QMap<QString, QString>    s_cache;

void SLFFile::setWizardryPath(QString path)
{
    s_wizardryPath = path;
}

QString &SLFFile::getWizardryPath()
{
    return s_wizardryPath;
}

SLFFile::SLFFile(const QString &slfFile, const QString &name) :
    m_wizardryPath(s_wizardryPath),
    m_slf(slfFile),
    m_storage(NULL),
    m_dataOffset(0xffffffff),
    m_dataLen(-1)
{
    // The cache only serves to help us avoid the directory parsing multiple times
    // if we encounter the same SLF file we've already found before - helps speed
    // up the load of Screens that can sometimes reuse the same slf file multiple
    // times for repeating widgets.

    m_filename = QString(name).replace("\\", "/");

    if (s_cache.contains( m_filename ))
    {
        QString memory = s_cache.value( m_filename );

        // empty strings in cache mean it wasn't found when search was conducted
        if (!memory.isEmpty())
        {
            m_in_slf   = (memory.at(0).digitValue() == 1);
            m_storage  = new QFile( memory.mid(1) ); // fortunately we use absolute paths when we open our files so CWD irrelevent
        }
    }
    else
    {
        setFileName( name );
        if (isGood())
        {
            s_cache.insert( m_filename, QString("%1%2").arg( m_in_slf ? "1" : "0" ).arg( m_storage->fileName() ) );
        }
        else
        {
            s_cache.insert( m_filename, QString() );
        }
    }
}

QPixmap SLFFile::getPixmapFromSlf( QString slfFile, int idx )
{
    QPixmap img;

    SLFFile slf( slfFile );
    if (slf.isGood())
    {
        if (slf.open(QFile::ReadOnly))
        {
            QByteArray array = slf.readAll();
            STItoQImage c( array );

            img = QPixmap::fromImage( c.getImage( idx ) );

            slf.close();
        }
    }
    return img;
}

void SLFFile::setFileName(const QString &name)
{
    QStringList filter;

    if (! m_wizardryPath.exists())
    {
        qCritical() << "Wizardry path" << m_wizardryPath << "doesn't exist!";
        return;
    }

    m_filename = QString(name).replace("\\", "/");
    // Order of precedence: 1) file in the filesystem, 2) inside a Patch file, 3) inside main file

    // 1) File in the filesystem -- complicating factor is that Wizardry
    //    uses case insensitive filenames, but this could be on a case-sensitive OS (eg linux)

    // Find the DATA subfolder - which we don't know the casing of yet

    filter.clear();
    filter << "DATA";

    QStringList entries = m_wizardryPath.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

    if (entries.size() == 1)
    {
        QDir    data_subfolder = m_wizardryPath;

        data_subfolder.cd( entries.at(0) );

        QDirIterator it( data_subfolder, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString file = it.next();

            if (file.compare( data_subfolder.absoluteFilePath( m_filename ), Qt::CaseInsensitive ) == 0)
            {
//                qDebug() << "Found" << m_filename << "in filesystem file" << file;
                m_storage = new QFile(file);
                m_in_slf = false;
                return;
            }
        }
    }

    // 2) inside patch file

    // Find the PATCHES subfolder - which we don't know the casing of yet

    filter.clear();
    filter << "PATCHES";

    entries = m_wizardryPath.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    if (entries.size() == 1)
    {
        QDir    patches_subfolder = m_wizardryPath;

        patches_subfolder.cd( entries.at(0) );

        filter.clear();
        filter << "PATCH.*";

        // We sort in reversed order so that later numbered patches can override earlier ones
        entries = patches_subfolder.entryList(filter, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase | QDir::Reversed );

        for (int k=0; k<entries.size(); k++)
        {
            // Patches have to end with 3 digits - the filter above isn't flexible enough
            // to let us enforce that (don't want to be processing patches that have been
            // disabled in place, eg. PATCH.010.BAK

            QString suffix = entries.at(k).mid(6);

            if ((suffix.length() == 3) &&
                (suffix.at(0).isDigit()) &&
                (suffix.at(1).isDigit()) &&
                (suffix.at(2).isDigit()))
            {
                QFile *probe = new QFile( patches_subfolder.absoluteFilePath( entries.at(k) ) );

                if (isSlf(*probe) && containsFile(*probe, m_filename))
                {
//                    qDebug() << "Found" << m_filename << "in SLF file" << probe->fileName();
                    m_storage = probe;
                    m_in_slf = true;
                    return;
                }
                delete probe;
            }
        }
    }

    // 3) inside main DATA.SLF file

    filter.clear();
    filter << "DATA";

    entries = m_wizardryPath.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

    if (entries.size() == 1)
    {
        QDir    data_subfolder = m_wizardryPath;

        data_subfolder.cd( entries.at(0) );

        QDirIterator it( data_subfolder, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString file = it.next();

            if (m_slf.compare( it.fileName(), Qt::CaseInsensitive ) == 0)
            {
                QFile *probe = new QFile( file );

                if (isSlf(*probe) && containsFile(*probe, m_filename))
                {
//                    qDebug() << "Found" << m_filename << "in SLF file" << probe->fileName();
                    m_storage = probe;
                    m_in_slf = true;
                    return;
                }
                delete probe;
            }
        }
    }
    // This isn't an error - the medium portaits use 2 different naming schemes, so
    // whenever we want to reference them we have to probe first one then the other,
    // for example, so one always fails.
    qDebug() << "Failed to find the file you wanted:" << m_filename;
}

SLFFile::~SLFFile()
{
    if (m_storage)
        delete m_storage;
}

QString SLFFile::fileName()
{
    return m_filename;
}

bool SLFFile::isGood()
{
    if (m_storage)
        return true;
    return false;
}

bool SLFFile::open(QFile::OpenMode flags)
{
    if (m_storage && m_storage->open(flags))
    {
        seekToFile();
        return true;
    }
    return false;
}

void SLFFile::close()
{
    if (m_storage && m_storage->isOpen())
        m_storage->close();

    m_dataOffset = 0xffffffff;
    m_dataLen = -1;
}

// Slight deviation to QFile -- if file isn't opened, size() will return -1
qint64 SLFFile::size()
{
    return m_dataLen;
}

bool SLFFile::seek(qint64 offset)
{
    if (offset < m_dataLen)
        return m_storage->seek( m_dataOffset + offset );

    return false;
}

qint64 SLFFile::skip(qint64 bytes)
{
    if (bytes > m_dataLen - (m_storage->pos() - m_dataOffset) )
        bytes = m_dataLen - (m_storage->pos() - m_dataOffset);

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    m_storage->read( bytes );
    return bytes;
#else
    return m_storage->skip( bytes );
#endif
}

QByteArray SLFFile::readAll()
{
    return m_storage->read( m_dataLen - (m_storage->pos() - m_dataOffset) );
}

QByteArray SLFFile::read(qint64 bytes)
{
    if (bytes > m_dataLen - (m_storage->pos() - m_dataOffset) )
        bytes = m_dataLen - (m_storage->pos() - m_dataOffset);

    return m_storage->read( bytes );
}

qint64 SLFFile::read(char *buf, qint64 bytes)
{
    if (bytes > m_dataLen - (m_storage->pos() - m_dataOffset) )
        bytes = m_dataLen - (m_storage->pos() - m_dataOffset);

    return m_storage->read( buf, bytes );
}

qint8 SLFFile::readByte()
{
    qint8 b;

    m_storage->read( (char *)&b, 1 );

    return b;
}

quint8 SLFFile::readUByte()
{
    quint8 b;

    m_storage->read( (char *)&b, 1 );

    return b;
}

qint16 SLFFile::readLEShort()
{
    quint8 buf[2];

    m_storage->read( (char *)buf, 2 );

    return FORMAT_LE16(buf);
}

quint16 SLFFile::readLEUShort()
{
    quint8 buf[2];

    m_storage->read( (char *)buf, 2 );

    return FORMAT_LE16(buf);
}

qint32 SLFFile::readLELong()
{
    quint8 buf[4];

    m_storage->read( (char *)buf, 4 );

    return FORMAT_LE32(buf);
}

quint32 SLFFile::readLEULong()
{
    quint8 buf[4];

    m_storage->read( (char *)buf, 4 );

    return FORMAT_LE32(buf);
}

bool SLFFile::isSlf(QFile &file)
{
    if (file.open(QFile::ReadOnly))
    {
        quint8  buf[256];

        // If it is an SLF file the first 256 bytes should start with the name of
        // the file (or maybe something else?) then followed by all zeros
        // And then a second 256 bytes in similar style gives the containing folder
        for (int i=0; i<2; i++)
        {
            file.read((char *)buf, 256);

            bool zeros = false;
            for (unsigned int k=0; k<sizeof(buf); k++)
            {
                if (buf[k] == 0)
                {
                    zeros = true;
                    continue;
                }
                if (! zeros && ((buf[k] >= 0x20) && (buf[k] <= 0x7f))) // printable ASCII character range
                    continue;

                file.close();
                return false;
            }
        }

        file.close();
        return true;
    }
    return false;
}

bool SLFFile::containsFile(QFile &file, const QString &filename)
{
    if (file.open(QFile::ReadOnly))
    {
        quint32   num_files;
        quint8    buf[257];

        // jump over archive name and the base folder name
        file.seek(512);

        // get the number of files in the archive
        file.read((char*)buf, 4);
        num_files = FORMAT_LE32(buf);
        for (int k=(int)num_files; k >= 0; k--)
        {
            file.seek( file.size() - 280 * k);

            file.read((char*)buf, 256);
            buf[256] = 0;

            QString archiveFile = QString::fromLatin1((char*)buf).replace("\\", "/");

            if (archiveFile.compare( filename, Qt::CaseInsensitive ) == 0)
            {
                file.close();
                return true;
            }
        }

        file.close();
    }
    return false;
}

void SLFFile::seekToFile()
{
    // File already expected to be opened
    quint32   num_files;
    quint8    buf[257];

    if (! m_in_slf)
    {
        // If we're using a real file, seek to beginning and set
        // offset to 0 and size to full file
        m_dataOffset = 0;
        m_dataLen    = m_storage->size();

        m_storage->seek(0);
        return;
    }

    // jump over archive name and the base folder name
    m_storage->seek(512);

    // get the number of files in the archive
    m_storage->read((char *)buf, 4);
    num_files = FORMAT_LE32(buf);

    for (int k=(int)num_files; k >= 0; k--)
    {
        m_storage->seek( m_storage->size() - 280 * k);

        m_storage->read((char *)buf, 256);
        buf[256] = 0;

        QString archiveFile = QString::fromLatin1((char *)buf).replace("\\", "/");

        if (archiveFile.compare( m_filename, Qt::CaseInsensitive ) == 0)
        {
            m_storage->read((char *)buf, 8);
            m_dataOffset = FORMAT_LE32(buf);
            m_dataLen    = (qint64) FORMAT_LE32(buf+4);

            m_storage->seek( m_dataOffset );
            break;
        }
    }
}

