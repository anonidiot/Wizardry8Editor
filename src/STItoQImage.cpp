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

#include <QtEndian>

#include <STItoQImage.h>
#include "common.h"

#include <QDebug>

// Flags
#define STCI_TRANSPARENT  (1 << 0)
#define STCI_RGB          (1 << 2)
#define STCI_INDEXED      (1 << 3)
#define STCI_ZLIB_COMP    (1 << 4)
#define STCI_ETRLE_COMP   (1 << 5)

STItoQImage::STItoQImage( QByteArray sti )
{
    const quint8 *data_ptr = (const quint8 *)sti.constData();

    if ((data_ptr[0] == 'S') &&
        (data_ptr[1] == 'T') &&
        (data_ptr[2] == 'C') &&
        (data_ptr[3] == 'I'))
    {
        parseSTI( data_ptr, sti.length() );
    }
}

// callback function for QImage to release the buffer containing the byte data
// -- this isn't currently needed because we use an image subclass to hold this
//    data and only generate the QImage when getImage() is called from it. So
//    long as the call to getImage() is followed by a shove into a pixmap BEFORE
//    the STItoQImage instance is destroyed, everything works
#if 0
static void bufferRelease(void *info)
{
    QByteArray *buffer = static_cast<QByteArray *>(info);

    delete buffer;
}
#endif

// Many thanks to Anonymous individual who documented this format
// http://ja2v113.pbworks.com/w/page/4218367/STCI%20%28STI%29%20format%20description

void STItoQImage::parseSTI( const quint8 *data_ptr, size_t len )
{
    quint32 flags;

    if (len < 0x40)
    {
        qWarning() << "STI data too small for even a header";
        return;
    }

    flags = FORMAT_LE32(data_ptr + 0x10);

    if (flags & STCI_RGB) // 16 bit truecolour image
    {
        if (flags & STCI_ETRLE_COMP)
        {
            qWarning() << "STCI_ETRLE_COMP doesn't make sense with 16 bit";
            return;
        }

        parse16bitSTI( flags, data_ptr, len );
    }
    else if (flags & STCI_INDEXED)
    {
        parseIndexedSTI( flags, data_ptr, len );
    }
    else
    {
        // No idea
        qWarning() << "Neither a 16bit nor an indexed image";
    }
}

void STItoQImage::parseIndexedSTI( quint32 flags, const quint8 *data_ptr, size_t len )
{
    quint8  depth    = FORMAT_8(   data_ptr + 0x2C);
    quint32 num_cols = FORMAT_LE32(data_ptr + 0x18);
    quint16 num_imgs = FORMAT_LE16(data_ptr + 0x1C);
    quint8  red_bits = FORMAT_8(   data_ptr + 0x1e);
    quint8  grn_bits = FORMAT_8(   data_ptr + 0x1f);
    quint8  blu_bits = FORMAT_8(   data_ptr + 0x20);

    //quint32 app_data_size = FORMAT_LE32(data_ptr + 0x2D);

    // palette consists of num_cols * (red_bits + grn_bits + blu_bits)/8 bytes
    quint32 *palette  = new quint32[ num_cols ];
    int      col_bits = red_bits + grn_bits + blu_bits;

    data_ptr += 0x40;
    len -= 0x40;
    for (unsigned int k=0; k<num_cols; k++)
    {
#if 1
        quint32 red = 0;
        quint32 grn = 0;
        quint32 blu = 0;

        for (int b=0; b<col_bits; b++)
        {
            int byte = b / 8;
            int bit  = b % 8;
            int c    = (data_ptr[byte] & (1 << (7-bit))) ? 1 : 0;

            if (b < red_bits)
            {
                red <<= 1;
                red |= c;
            }
            else if (b < red_bits + grn_bits)
            {
                grn <<= 1;
                grn |= c;
            }
            else if (b < red_bits + grn_bits + blu_bits)
            {
                blu <<= 1;
                blu |= c;
            }
        }
        data_ptr += (col_bits+7) / 8;  // round up
        len -= (col_bits+7) / 8;

        red = 255 * red / ((1 << red_bits) - 1);
        grn = 255 * grn / ((1 << grn_bits) - 1);
        blu = 255 * blu / ((1 << blu_bits) - 1);

        palette[k] = (red << 24) | (grn << 16) | (blu << 8) | 0xff /*alpha*/;
#else
        // Simple version if we assume it is always 8,8,8

        palette[k] = ((quint32)(data_ptr[0]) << 24) |
                     ((quint32)(data_ptr[1]) << 16) |
                     ((quint32)(data_ptr[2]) <<  8) | 0xff /*alpha*/;
        data_ptr += 3;
        len -= 3;
#endif
    }

    if (! (flags & STCI_ETRLE_COMP))
    {
        qWarning() << "Don't know the structure for an uncompressed palette-based image";
        // all possible values used for colours so:
        //  * no transparency maybe?
        //  * no early terminating lines
        // could just interpret it as a width*height array of palette indexes
        // but not encountered any images like this to actually confirm with
        delete[] palette;
        return;
    }

    for (int j=0; j<num_imgs; j++)
    {
        quint32 img_st   = FORMAT_LE32(data_ptr + j*16 + 0x00);
        quint32 img_len  = FORMAT_LE32(data_ptr + j*16 + 0x04);
        quint16 x_offset = FORMAT_LE16(data_ptr + j*16 + 0x08);
        quint16 y_offset = FORMAT_LE16(data_ptr + j*16 + 0x0A);
        quint16 height   = FORMAT_LE16(data_ptr + j*16 + 0x0C);
        quint16 width    = FORMAT_LE16(data_ptr + j*16 + 0x0E);

        const quint8 *img_ptr  = data_ptr + 16*num_imgs + img_st;

        QByteArray img_raw;

        int row_width = 0;
        while (img_len > 0)
        {
            int run_width = *img_ptr & 0x7f;

            row_width += run_width;

            if (*img_ptr == 0)
            {
                // row_end
                if (row_width > width)
                {
                    qWarning() << "ROW TOO LONG" << row_width;
                }
                else if (row_width < width)
                {
                    for (int k=row_width; k < width; k++)
                    {
                        img_raw.append( (char)0x00 ); // ARGB fully transparent
                        img_raw.append( (char)0x00 );
                        img_raw.append( (char)0x00 );
                        img_raw.append( (char)0x00 );
                    }
                }

                // reset for next row
                row_width = 0;
            }
            else if (*img_ptr & 0x80)
            {
                // transparent bytes
                for (int k=0; k<run_width; k++)
                {
                    img_raw.append( (char)0x00 ); // ARGB fully transparent
                    img_raw.append( (char)0x00 );
                    img_raw.append( (char)0x00 );
                    img_raw.append( (char)0x00 );
                }
            }
            else
            {
                // opaque bytes
                for (int k=0; k<run_width; k++)
                {
                    img_ptr++;
                    img_len--;
                    // For some reason QT still wants the byte order in BGRA despite the fact we tell it ARGB
                    // This is the opposite of what the QT documentation is indicating - this is supposed to
                    // be the architecture independent format
                    img_raw.append( (char)((palette[*img_ptr] & 0x0000ff00) >>  8) );
                    img_raw.append( (char)((palette[*img_ptr] & 0x00ff0000) >> 16) );
                    img_raw.append( (char)((palette[*img_ptr] & 0xff000000) >> 24) );
                    img_raw.append( (char)((palette[*img_ptr] & 0x000000ff)      ) );
                }
            }
            img_ptr++;
            img_len--;
        }

        image i;

        i.x_offset   = x_offset;
        i.y_offset   = y_offset;
        i.width      = width;
        i.height     = height;
        i.depth      = depth;
        i.img_data   = img_raw;
        i.img_format = QImage::Format_ARGB32;

        m_image.append(i);
    }
    delete[] palette;
}

void STItoQImage::parse16bitSTI( quint32 flags, const quint8 *data_ptr, size_t len )
{
    quint8  depth    = FORMAT_8(   data_ptr + 0x2C);
    quint16 height   = FORMAT_LE16(data_ptr + 0x14);
    quint16 width    = FORMAT_LE16(data_ptr + 0x16);

    quint32 red_bits = FORMAT_LE32(data_ptr + 0x18);
    quint32 grn_bits = FORMAT_LE32(data_ptr + 0x1C);
    quint32 blu_bits = FORMAT_LE32(data_ptr + 0x20);
    quint32 alp_bits = FORMAT_LE32(data_ptr + 0x24);

    if ((red_bits == 0x0000f800) /* 11111000 00000000 */ &&
        (grn_bits == 0x000007e0) /* 00000111 11100000 */ &&
        (blu_bits == 0x0000001f) /* 00000000 00011111 */ &&
        (alp_bits == 0x00000000))
    {
        // It's RGB565 - QT knows how to parse this
        //  but QImage requires the buffer to be available for the life of the image, and data_ptr
        //  won't be, so copy it
        QByteArray img_raw( (char *)data_ptr + 0x40, len - 0x40 );

        if (flags & STCI_ZLIB_COMP)
        {
            qWarning() << "===UNTESTED=== STCI_ZLIB_COMP flag code";
            // qUncompress needs a 4 byte header saying the size of the output in BigEndian -
            // use the uncompressed size from the STCI header for this.

            quint32 uncompressed_size  = FORMAT_LE32(data_ptr + 0x04);
            quint32 be_uncompress_size = qToBigEndian( uncompressed_size ); // 32 bit

            img_raw.prepend( (char *)(&be_uncompress_size), 4 );

            img_raw = qUncompress( img_raw );
        }

        image i;

        i.x_offset   = 0;
        i.y_offset   = 0;
        i.width      = width;
        i.height     = height;
        i.depth      = depth;
        i.img_data   = img_raw;
        i.img_format = QImage::Format_RGB16;

        m_image.append(i);

        return;
    }

    // We're going to have to expand the data ourselves.
    // Left unimplemented since don't know what sort of structure
    // to expect, and haven't found any images using it.

    qWarning() << "Unhandled image format";
}
