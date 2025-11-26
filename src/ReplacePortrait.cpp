/*
 * Copyright (C) 2024-2025 Anonymous Idiot
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

#include "ReplacePortrait.h"
#include "SLFFile.h"
#include "STI.h"
#include "main.h"

#include <QDirIterator>
#include <QFile>
#include <QImage>
#include "ScreenCommon.h"
#include "common.h"

#include <QDebug>

#define PATCH_FILE      "PATCH.010"
#define PORTRAIT_WIDTH  180
#define PORTRAIT_HEIGHT 144

class SlfEntry
{
public:
    SlfEntry() {}
    ~SlfEntry() {}

    SlfEntry(const SlfEntry &other)
    {
        filename    = other.filename;
        file_size   = other.file_size;
        file_offset = other.file_offset;
    }

    SlfEntry & operator=(const SlfEntry &other)
    {
        filename    = other.filename;
        file_size   = other.file_size;
        file_offset = other.file_offset;

        return *this;
    }

    bool operator<(const SlfEntry &other) const
    {
        return (filename < other.filename);
    }

    QString filename;
    quint32 file_size;
    quint32 file_offset;
};

void replacePortrait( int portraitId, QString filename )
{
    if (! filename.isEmpty())
    {
        QImage img( filename );

        if ((img.width() == PORTRAIT_WIDTH) && (img.height() == PORTRAIT_HEIGHT))
        {
            rebuildPatchFile( portraitId, img );
        }
        else
        {
            rebuildPatchFile( portraitId, img.scaled( PORTRAIT_WIDTH, PORTRAIT_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
        }
    }
    else
    {
        rebuildPatchFile( portraitId, QImage() );
    }
}

void rebuildPatchFile( int portraitId, const QImage &largeImage )
{
    QString smallPortraitName  = ScreenCommon::getSmallPortraitName( portraitId );
    QString mediumPortraitName = ScreenCommon::getMediumPortraitName( portraitId );
    QString largePortraitName  = ScreenCommon::getLargePortraitName( portraitId );

    QString &wizardryPath = SLFFile::getWizardryPath();
    QDir    patches_subfolder = wizardryPath;

    QFile   *src = NULL;

    QByteArray data;

    // Find the PATCHES subfolder - which we don't know the casing of yet

    QStringList filter;

    filter.clear();
    filter << "PATCHES";

    QStringList entries = patches_subfolder.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    if (entries.size() == 1)
    {
//        qDebug() << "Found " << entries.at(0) << "folder";
        patches_subfolder.cd( entries.at(0) );

        // Look for PATCH_FILE

        QDirIterator it( patches_subfolder, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString file = it.next();

            if (file.compare( patches_subfolder.absoluteFilePath( PATCH_FILE ), Qt::CaseInsensitive ) == 0)
            {
//                qDebug() << "Found " << file;
                // existing file found
                src = new QFile(file);
                break;
            }
        }
    }
    else
    {
        // No patches folder
        qDebug() << "PATCHES folder not found - making it";

        patches_subfolder.mkdir( "PATCHES" );
        patches_subfolder.cd( "PATCHES" );
    }

    if (!src)
    {
        // No file found
        qDebug() << "Creating new" << PATCH_FILE << "file";
        src = new QFile( patches_subfolder.absoluteFilePath( PATCH_FILE ) );
    }
    else
    {
        if (src->open(QFile::ReadOnly))
        {
            data = src->readAll();
            src->close();
            src->resize(0); // truncate the file
        }
    }

    if (src->open(QFile::WriteOnly))
    {
        QList<SlfEntry>  slf_contents;
        QByteArray       slf_output;

        // Populate the header bytes
        slf_output.append( PATCH_FILE, sizeof(PATCH_FILE) );
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        for (int z=0; z<256 - sizeof(PATCH_FILE); z++)
            slf_output.append( '\0' );
#else
        slf_output.append( 256 - sizeof(PATCH_FILE), '\0' );
#endif
        slf_output.append( "Data\\", 5 );
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        for (int z=0; z<256 - 5; z++)
            slf_output.append( '\0' );
#else
        slf_output.append( 256 - 5, '\0' );
#endif

        // These need updating after the archive is built
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        for (int z=0; z<20; z++)
            slf_output.append( '\0' );
#else
        slf_output.append( 20, '\0' );
#endif

        // Step through all the files in the pre-existing patch (if any)
        // and copy them into the output - so long as they don't match
        // the portrait we just modified.

        const quint8  *slf      = (const quint8 *) data.constData();
        const quint32  slf_size = data.size();

        if (slf_size > 516)
        {
            quint32 num_files = FORMAT_LE32(slf+512);

            qDebug() << num_files << "in slf file already.";
            if (slf_size > num_files * 280)
            {
                for (int k=(int)num_files; k > 0; k--)
                {
                    QByteArray filename( (char *)(slf+(slf_size - 280 * k)), 256);

                    filename.append( '\0' );

                    QString archiveFile = QString::fromLatin1((char*)filename.constData()).replace("\\", "/");

                    if (archiveFile.compare( smallPortraitName, Qt::CaseInsensitive ) == 0)
                    {
                        qDebug() << archiveFile << "was in pre-existing" << PATCH_FILE << "file - ignoring it because it matches SMALL version of portrait we are saving";
                    }
                    else if (archiveFile.compare( mediumPortraitName, Qt::CaseInsensitive ) == 0)
                    {
                        qDebug() << archiveFile << "was in pre-existing" << PATCH_FILE << "file - ignoring it because it matches MEDIUM version of portrait we are saving";
                    }
                    else if (archiveFile.compare( largePortraitName, Qt::CaseInsensitive ) == 0)
                    {
                        qDebug() << archiveFile << "was in pre-existing" << PATCH_FILE << "file - ignoring it because it matches LARGE version of portrait we are saving";
                    }
                    else
                    {
                        quint32 file_offset = FORMAT_LE32( slf+(slf_size - 280 * k)+256 );
                        quint32 file_size   = FORMAT_LE32( slf+(slf_size - 280 * k)+260 );

                        qDebug() << archiveFile << "was in pre-existing" << PATCH_FILE << "file - size " << file_size << " at offset " << file_offset;

                        SlfEntry s;

                        s.filename    = archiveFile;
                        s.file_offset = slf_output.size();
                        s.file_size   = file_size;

                        slf_output.append( (char *)(slf+file_offset), file_size );
                        slf_contents.append( s );
                    }
                }
            }
        }

        // if largeImage is a null pixmap, we're performing the 'reset' function -
        // restoring the portrait back to default by removing any mods
        // of it from the patch file
        if (! largeImage.isNull())
        {
            QByteArray largeImageSTI = STI::makeSTI( largeImage );

            qDebug() << "Size of LARGE STI image:" << largeImageSTI.size();

            QImage mediumImage = quantise( largeImage.scaledToWidth( 90, Qt::SmoothTransformation ), 255 );

            QByteArray mediumImageSTI = STI::makeSTI( mediumImage, 10, false );

            qDebug() << "Size of MEDIUM STI image:" << mediumImageSTI.size();

            QImage smallImage = largeImage.scaledToWidth( 45, Qt::SmoothTransformation );

            QByteArray smallImageSTI = STI::makeSTI( smallImage );

            qDebug() << "Size of SMALL STI image:" << smallImageSTI.size();


            // Now push our STI file replacements to the list too
            SlfEntry s;

            s.filename    = largePortraitName;
            s.file_offset = slf_output.size();
            s.file_size   = largeImageSTI.size();

            slf_output.append( largeImageSTI );
            slf_contents.append( s );

            s.filename    = mediumPortraitName;
            s.file_offset = slf_output.size();
            s.file_size   = mediumImageSTI.size();

            slf_output.append( mediumImageSTI );
            slf_contents.append( s );

            s.filename    = smallPortraitName;
            s.file_offset = slf_output.size();
            s.file_size   = smallImageSTI.size();

            slf_output.append( smallImageSTI );
            slf_contents.append( s );
        }

        // update header fields deferred till now
        quint8 *d = (quint8 *) slf_output.data();

        d[512] = ( (slf_contents.size() >>  0) & 0xff );
        d[513] = ( (slf_contents.size() >>  8) & 0xff );
        d[514] = ( (slf_contents.size() >> 16) & 0xff );
        d[515] = ( (slf_contents.size() >> 24) & 0xff );

        d[516] = ( (slf_contents.size() >>  0) & 0xff );
        d[517] = ( (slf_contents.size() >>  8) & 0xff );
        d[518] = ( (slf_contents.size() >> 16) & 0xff );
        d[519] = ( (slf_contents.size() >> 24) & 0xff );

        d[520] = 0xff;
        d[521] = 0xff;
        d[522] = 0x00;
        d[523] = 0x02;

        d[524] = 0x01;

        d[525] = 0x00;
        d[526] = 0x00;
        d[527] = 0x00;
        d[528] = 0x00;
        d[529] = 0x00;
        d[530] = 0x00;
        d[531] = 0x00;


        // Create the SLF catalogue at the tail of the file.
        // This has to be sorted alphabetically in order to work.
        // Fortunately the order of the data it references isn't
        // important.

        std::sort(slf_contents.begin(), slf_contents.end());

        for (int k=0; k<slf_contents.size(); k++)
        {
            SlfEntry s = slf_contents.at(k);

            s.filename.replace("/", "\\"); // Needs to be Windows format dir slash

            qDebug() << "File" << s.filename << s.file_size;
            slf_output.append( s.filename.toLatin1(), s.filename.size() );
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
            for (int z=0; z<256 - s.filename.size(); z++)
                slf_output.append( '\0' );
#else
            slf_output.append( 256 - s.filename.size(), '\0' );
#endif
            slf_output.append( (char)(s.file_offset >>  0) & 0xff );
            slf_output.append( (char)(s.file_offset >>  8) & 0xff );
            slf_output.append( (char)(s.file_offset >> 16) & 0xff );
            slf_output.append( (char)(s.file_offset >> 24) & 0xff );
            slf_output.append( (char)(s.file_size   >>  0) & 0xff );
            slf_output.append( (char)(s.file_size   >>  8) & 0xff );
            slf_output.append( (char)(s.file_size   >> 16) & 0xff );
            slf_output.append( (char)(s.file_size   >> 24) & 0xff );
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
            for (int z=0; z<16; z++)
                slf_output.append( '\0' );
#else
            slf_output.append( 16, '\0' );
#endif
        }

        // Write the buffer into the file

        qDebug() << "Writing new" << PATCH_FILE << "file of size" << slf_output.size();
        src->write( slf_output );
        src->close();

        // Portraits have been updated so remove the pre-existing notions of where to 
        // find the file data for these from the SLF cache

        SLFFile::flushFromCache( smallPortraitName );
        SLFFile::flushFromCache( mediumPortraitName );
        SLFFile::flushFromCache( largePortraitName );
    }

    delete src;
}

// The technique of quantisation here comes from
// https://rosettacode.org/wiki/Color_quantization/C
// I couldn't find a copyright for the source code, but the site itself is covered by
// the GNU Free Document License 1.3
// It has been minimally modified to work with this class

#define ON_INHEAP   1

typedef struct oct_node_t oct_node_t, *oct_node;
struct oct_node_t
{
    int64_t r, g, b; /* sum of all child node colors */
    int count, heap_idx;
    unsigned char n_kids, kid_idx, flags, depth;
    oct_node kids[8], parent;
};

typedef struct
{
    int alloc, n;
    oct_node* buf;
} node_heap;

static int cmp_node(oct_node a, oct_node b)
{
    if (a->n_kids < b->n_kids)
        return -1;
    if (a->n_kids > b->n_kids)
        return 1;

    int ac = a->count >> a->depth;
    int bc = b->count >> b->depth;
    return ac < bc ? -1 : ac > bc;
}

static void down_heap(node_heap *h, oct_node p)
{
    int n = p->heap_idx, m;
    while (1)
    {
        m = n * 2;
        if (m >= h->n)
            break;
        if (m + 1 < h->n && cmp_node(h->buf[m], h->buf[m + 1]) > 0)
            m++;

        if (cmp_node(p, h->buf[m]) <= 0)
            break;

        h->buf[n] = h->buf[m];
        h->buf[n]->heap_idx = n;
        n = m;
    }
    h->buf[n] = p;
    p->heap_idx = n;
}

static void up_heap(node_heap *h, oct_node p)
{
    int n = p->heap_idx;
    oct_node prev;

    while (n > 1)
    {
        prev = h->buf[n / 2];
        if (cmp_node(p, prev) >= 0)
            break;

        h->buf[n] = prev;
        prev->heap_idx = n;
        n /= 2;
    }
    h->buf[n] = p;
    p->heap_idx = n;
}

static void heap_add(node_heap *h, oct_node p)
{
    if ((p->flags & ON_INHEAP))
    {
        down_heap(h, p);
        up_heap(h, p);
        return;
    }

    p->flags |= ON_INHEAP;
    if (!h->n) h->n = 1;
    if (h->n >= h->alloc)
    {
        while (h->n >= h->alloc)
            h->alloc += 1024;
        h->buf = (oct_node_t **) realloc(h->buf, sizeof(oct_node) * h->alloc);
    }

    p->heap_idx = h->n;
    h->buf[h->n++] = p;
    up_heap(h, p);
}

static oct_node pop_heap(node_heap *h)
{
    if (h->n <= 1)
        return 0;

    oct_node ret = h->buf[1];
    h->buf[1] = h->buf[--h->n];

    h->buf[h->n] = 0;

    h->buf[1]->heap_idx = 1;
    down_heap(h, h->buf[1]);

    return ret;
}

static oct_node node_new(oct_node *pool, int *len, unsigned char idx, unsigned char depth, oct_node p)
{
    if (*len <= 1)
    {
        oct_node p = (oct_node) calloc(2048, sizeof(oct_node_t));
        p->parent = *pool;
        *pool = p;
        *len = 2047;
    }

    oct_node x = *pool + (*len)--;
    x->kid_idx = idx;
    x->depth = depth;
    x->parent = p;
    if (p)
        p->n_kids++;
    return x;
}

static void node_free( oct_node pool )
{
    oct_node p;
    while (pool)
    {
        p = pool->parent;
        free(pool);
        pool = p;
    }
}

static oct_node node_insert(oct_node *pool, int *len, oct_node root, QRgb pix)
{
    unsigned char i, bit, depth = 0;

    for (bit = 1 << 7; ++depth < 8; bit >>= 1)
    {
        i = !!(qGreen( pix ) & bit) * 4 + !!(qRed( pix ) & bit) * 2 + !!(qBlue( pix ) & bit);
        if (!root->kids[i])
            root->kids[i] = node_new(pool, len, i, depth, root);

        root = root->kids[i];
    }

    root->r += qRed( pix );
    root->g += qGreen( pix );
    root->b += qBlue( pix );
    root->count++;
    return root;
}

static oct_node node_fold(oct_node p)
{
    if (p->n_kids)
        abort();
    oct_node q = p->parent;
    q->count += p->count;

    q->r += p->r;
    q->g += p->g;
    q->b += p->b;
    q->n_kids --;
    q->kids[p->kid_idx] = 0;
    return q;
}

static QRgb color_replace(oct_node root, QRgb pix)
{
    unsigned char i, bit;

    for (bit = 1 << 7; bit; bit >>= 1)
    {
        i = !!(qGreen(pix) & bit) * 4 + !!(qRed(pix) & bit) * 2 + !!(qBlue(pix) & bit);
        if (!root->kids[i])
            break;
        root = root->kids[i];
    }

    return (QRgb) (((int)(root->r) << 16) |
                   ((int)(root->g) <<  8) |
                   ((int)(root->b) <<  0));
}

static void irritatingQImageCleanup(void *info)
{
    QByteArray *qb = (QByteArray *)info;

    delete qb;
}

QImage quantise( QImage src, int n_colors )
{
    QImage dest;

    oct_node     pool = 0;
    int          len = 0;

    node_heap    heap = { 0, 0, 0 };
    oct_node     root = node_new(&pool, &len, 0, 0, 0);
    oct_node     got;

    for (int y=0; y<src.height(); y++)
    {
        for (int x=0; x<src.width(); x++)
        {
            heap_add( &heap, node_insert( &pool, &len, root, src.pixelColor( x, y ).rgba() ));
        }
    }

    while (heap.n > n_colors + 1)
    {
        heap_add( &heap, node_fold( pop_heap( &heap ) ));
    }

    double c;
    for (int i = 1; i < heap.n; i++)
    {
        got = heap.buf[i];
        c = got->count;
        got->r = got->r / c + .5;
        got->g = got->g / c + .5;
        got->b = got->b / c + .5;
    }

    QVector<QRgb>  palette;
    QByteArray    *img = new QByteArray();
    for (int y=0; y<src.height(); y++)
    {
        for (int x=0; x<src.width(); x++)
        {
            QRgb new_colr = color_replace(root, src.pixelColor( x, y ).rgba() );

            if (! palette.contains( new_colr ))
            {
                palette.append( new_colr );
            }
            img->append( palette.indexOf( new_colr ) );
        }
    }
    Q_ASSERT( palette.size() <= n_colors );
    dest = QImage( (const uchar *)img->constData(), src.width(), src.height(), src.width(), QImage::Format_Indexed8, & irritatingQImageCleanup, (void *)img );
    dest.setColorTable( palette );

    node_free( pool );
    free(heap.buf);

    return dest;
}
