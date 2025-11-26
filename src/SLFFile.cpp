/*
 * Copyright (C) 2022-2025 Anonymous Idiot
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
#include "STI.h"
#include "common.h"

#include <QDebug>

static QString                   s_wizardryPath;
static QString                   s_worldPath;
static QMap<QString, QString>    s_cache;
static bool                      s_parallelWorlds = false;
static QString                   s_world;

void SLFFile::setWizardryPath(QString path)
{
    s_wizardryPath = path;
}

// Current design only expects the parallel world to be set at app init.
// Initially we expect an empty string, which provides for minimal support
// to draw a "Select Parallel World to use" dialog. And after that we expect
// to be intialised with the actual world.
// The main thing this simplifies is the Urho3D engine usage of the SLF file.
// But it also affects all graphics etc. shown in all windows open etc. If we
// do want to reset the world, we're pretty much doing an app reset.
void SLFFile::setParallelWorld(QString world)
{
    // radically changes method for locating files
    s_parallelWorlds = true;
    s_world = world;

    if (!s_world.isEmpty())
    {
        QDir        wizPath( s_wizardryPath );
        QStringList filter;

        filter << "ParallelWorld";
        QStringList entries = wizPath.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

        if (entries.size() == 1)
        {
            QDir    parworlds = wizPath;

            parworlds.cd( entries.at(0) );

            filter.clear();
            filter << s_world;
            entries = parworlds.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

            if (entries.size() == 1)
            {
                parworlds.cd( entries.at(0) );

                s_worldPath = parworlds.absolutePath();
            }
        }
    }

    // flush the entire path cache because everything is different now
    s_cache.clear();
}

QString &SLFFile::getWizardryPath()
{
    return s_wizardryPath;
}

QString &SLFFile::getParallelWorldPath()
{
    return s_worldPath;
}

SLFFile::SLFFile(const QString &folder, const QString &slfFile, const QString &name, bool force_base) :
    m_subfolder(folder),
    m_slf(slfFile),
    m_storage(NULL),
    m_dataOffset(0xffffffff),
    m_dataLen(-1)
{
    init( name, force_base );
}

SLFFile::SLFFile(const QString &slfFile, const QString &name, bool force_base) :
    m_subfolder("DATA"),
    m_slf(slfFile),
    m_storage(NULL),
    m_dataOffset(0xffffffff),
    m_dataLen(-1)
{
    init( name, force_base );
}

// Special constructor for a SLF file we've explicitly had to locate manually because
// it doesn't obey conventional placement rules (eg. FanPatch.dat).
// The file handle should not be used by the caller afterwards. It will be deleted
// when the SLFFile is deleted, as per usual.
SLFFile::SLFFile(QFile *slfFile) :
    m_subfolder(""),
    m_slf(""),
    m_in_slf(true),
    m_in_patch(false),
    m_filename(""),
    m_storage(slfFile),
    m_dataOffset(0xffffffff),
    m_dataLen(-1)
{
}

void SLFFile::flushFromCache(const QString &name)
{
    QString filename = QString(name).replace("\\", "/").toUpper();

    s_cache.remove( "B" + filename );
    s_cache.remove( "G" + filename );
}

void SLFFile::init(const QString &name, bool force_base )
{
    // The cache only serves to help us avoid the directory parsing multiple times
    // if we encounter the same SLF file we've already found before - helps speed
    // up the load of Screens that can sometimes reuse the same slf file multiple
    // times for repeating widgets.

    m_filename = QString(name).replace("\\", "/").toUpper();

    QString key = (force_base ? "B" : "G") + m_filename; // Base-restricted or Global (everywhere)

    if (s_cache.contains( key ))
    {
        QString memory = s_cache.value( key );

        // empty strings in cache mean it wasn't found when search was conducted
        if (!memory.isEmpty())
        {
            m_in_slf   = (memory.at(0).digitValue() == 1);
            m_in_patch = (memory.at(1).digitValue() == 1);
            m_storage  = new QFile( memory.mid(2) ); // fortunately we use absolute paths when we open our files so CWD irrelevent
        }
    }
    else
    {
        setFileName( name, force_base );
        if (isGood())
        {
            s_cache.insert( key, QString("%1%2%3").arg( m_in_slf ? "1" : "0" ).arg( m_in_patch ? "1" : "0" ).arg( m_storage->fileName() ) );
        }
        else
        {
            s_cache.insert( key, QString() );
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
            if (slf.size() != -1)
            {
                QByteArray array;

                slf.readAll( array );
                STI c( array );

                img = QPixmap::fromImage( c.getImage( idx ) );

            slf.close();
            }
        }
    }
    return img;
}

void SLFFile::setFileName(const QString &name, bool force_base)
{
    QStringList filter;

    m_filename = QString(name).replace("\\", "/").toUpper();
    // Non-parallel worlds Order of precedence:
    // 1. file in the filesystem
    // 2. inside a Patch file
    // 3. inside main file

    // Parallel worlds Order of precedence:
    // 1. file in the specific world filesystem
    // 2. inside main file - WHICH HAS BEEN RELOCATED TO s_wizardryPath
    // Patches and files in base path are ignored.
    // Portraits have their own special additional rules:
    // TODO: determine precedent order for portraits

    // 1. File in the filesystem - either in wizardrypath/subfolder or
    //    wizardrypath/ParallelWorlds/world/subfolder


    QDir         cwd = s_wizardryPath;
    QStringList  entries;

    bool         skip_mod = force_base;

    if (s_parallelWorlds)
    {
        if (s_world.isEmpty())
        {
            // intentional edge case for it _not_ to match any files in any world -
            // used to perform a basic init before the initial Parallel World selection
            // dialog, so that interface elements can be loaded from DATA.SLF to render
            // that dialog.
            skip_mod = true;
        }
        else
        {
            // = operator is apparently deprecated, and setPath() preferred, but = invokes
            // swap() which is very fast, never fails and avoids repeating a lot of operations
            // we're managing ourselves. Stick with it; it's better. But shutup the compile warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            cwd = s_worldPath;
#pragma GCC diagnostic pop
        }
    }

    // Find the m_subfolder subfolder ("DATA" by default) - which we don't know the casing of yet

    filter.clear();
    filter << m_subfolder;

    // If we're in parallel worlds configuration but no parallel world is set yet, then
    // skip the search through an as yet unknown world.
    // Also skip if we have been explicitly asked to in the constructor.

    if (!skip_mod)
    {
        entries = cwd.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

        if (entries.size() == 1)
        {
            cwd.cd( entries.at(0) );

            QDirIterator it( cwd, QDirIterator::Subdirectories);

            while (it.hasNext())
            {
                QString file = it.next();

                if (file.compare( cwd.absoluteFilePath( m_filename ), Qt::CaseInsensitive ) == 0)
                {
//                    qDebug() << "Found" << m_filename << "in filesystem file" << file;
                    m_storage = new QFile(file);
                    m_in_slf = false;
                    m_in_patch = false;
                    return;
                }
            }
        }
    }

    // 2) inside patch file - only if not parallel worlds

    if (! s_parallelWorlds)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        cwd = s_wizardryPath;
#pragma GCC diagnostic pop
        // Find the PATCHES subfolder - which we don't know the casing of yet

        filter.clear();
        filter << "PATCHES";

        entries = cwd.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
        if (entries.size() == 1)
        {
            cwd.cd( entries.at(0) );

            filter.clear();
            filter << "PATCH.*";

            // We sort in reversed order so that later numbered patches can override earlier ones
            entries = cwd.entryList(filter, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase | QDir::Reversed );

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
                    QFile *probe = new QFile( cwd.absoluteFilePath( entries.at(k) ) );

                    if (isSlf(*probe) && containsFile(*probe, m_filename))
                    {
    //                    qDebug() << "Found" << m_filename << "in SLF file" << probe->fileName();
                        m_storage = probe;
                        m_in_slf = true;
                        m_in_patch = true;
                        return;
                    }
                    delete probe;
                }
            }
        }
    }

    // 3) inside the given SLF file (DATA.SLF by default)
    //    If parallelworlds this will be in the base folder
    //    otherwise it is in subfolder

    bool    search      = false;
    QDir    slf_homedir = s_wizardryPath;

    if (s_parallelWorlds)
    {
        search = true;
    }
    else
    {
        filter.clear();
        filter << m_subfolder;

        entries = slf_homedir.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

        if (entries.size() == 1)
        {
            search = true;
            slf_homedir.cd( entries.at(0) );
        }
    }

    if (search)
    {
        QDirIterator it( slf_homedir );
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
                    m_in_patch = false;
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

bool SLFFile::isFromPatch()
{
    return m_in_patch;
}

bool SLFFile::open(QFile::OpenMode flags)
{
    if (!m_storage)
        return false;

    // close it in case we're opening in another mode (although we should be readonly always)
    if (m_storage->isOpen())
        m_storage->close();

    if (m_storage->open(flags))
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

void SLFFile::readAll( QByteArray &buffer )
{
    qint64 to_read = (m_dataLen - (m_storage->pos() - m_dataOffset));

    // Preallocate the bytearray and read into it directly, rather than
    // copy a returned bytearray, which uses twice the memory.
    buffer.resize( to_read );
    if (buffer.size() < to_read)
    {
        throw SLFFileException();
    }

    m_storage->read( buffer.data(), to_read );
}

QByteArray SLFFile::readAll()
{
    QByteArray qb;
    readAll( qb );

    return qb;
}

QByteArray SLFFile::read(qint64 bytes)
{
    if (bytes > m_dataLen - (m_storage->pos() - m_dataOffset) )
        bytes = m_dataLen - (m_storage->pos() - m_dataOffset);

    if (bytes == 0)
    {
        throw SLFFileException();
    }

    QByteArray qb = m_storage->read( bytes );
    if (qb.size() == 0)
    {
        throw SLFFileException();
    }

    return qb;
}

qint64 SLFFile::pos()
{
    return m_storage->pos() - m_dataOffset;
}

qint64 SLFFile::read(char *buf, qint64 bytes)
{
    if (bytes > m_dataLen - (m_storage->pos() - m_dataOffset) )
        bytes = m_dataLen - (m_storage->pos() - m_dataOffset);

    if (bytes == 0)
    {
        throw SLFFileException();
    }

    qint64 r = m_storage->read( buf, bytes );
    if (r == -1)
    {
        throw SLFFileException();
    }

    return r;
}

qint8 SLFFile::readByte()
{
    qint8 b;

    if (-1 == m_storage->read( (char *)&b, 1 ))
    {
        throw SLFFileException();
    }

    return b;
}

quint8 SLFFile::readUByte()
{
    quint8 b;

    if (-1 == m_storage->read( (char *)&b, 1 ))
    {
        throw SLFFileException();
    }

    return b;
}

qint16 SLFFile::readLEShort()
{
    quint8 buf[2];

    if (-1 == m_storage->read( (char *)buf, 2 ))
    {
        throw SLFFileException();
    }

    return FORMAT_LE16(buf);
}

quint16 SLFFile::readLEUShort()
{
    quint8 buf[2];

    if (-1 == m_storage->read( (char *)buf, 2 ))
    {
        throw SLFFileException();
    }

    return FORMAT_LE16(buf);
}

qint32 SLFFile::readLELong()
{
    quint8 buf[4];

    if (-1 == m_storage->read( (char *)buf, 4 ))
    {
        throw SLFFileException();
    }

    return FORMAT_LE32(buf);
}

quint32 SLFFile::readLEULong()
{
    quint8 buf[4];

    if (-1 == m_storage->read( (char *)buf, 4 ))
    {
        throw SLFFileException();
    }

    return FORMAT_LE32(buf);
}

float SLFFile::readFloat()
{
    quint8   buf[4];
    quint32  v;
    float   *f;

    if (-1 == m_storage->read( (char *)buf, 4 ))
    {
        throw SLFFileException();
    }

    v = FORMAT_LE32(buf);
    f = (float *) &v;

    return *f;
}

QByteArray SLFFile::readLine()
{
    return m_storage->readLine();
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
        for (int k=(int)num_files; k > 0; k--)
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
    seekToFile( m_filename );
}

void SLFFile::seekToFile(QString filename)
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

    for (int k=(int)num_files; k > 0; k--)
    {
        m_storage->seek( m_storage->size() - 280 * k);

        m_storage->read((char *)buf, 256);
        buf[256] = 0;

        QString archiveFile = QString::fromLatin1((char *)buf).replace("\\", "/");

        if (archiveFile.compare( filename, Qt::CaseInsensitive ) == 0)
        {
            m_storage->read((char *)buf, 8);
            m_dataOffset = FORMAT_LE32(buf);
            m_dataLen    = (qint64) FORMAT_LE32(buf+4);

            m_storage->seek( m_dataOffset );
            break;
        }
    }
}

