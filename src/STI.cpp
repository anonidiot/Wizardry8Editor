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

#include <QtEndian>

#include <STI.h>
#include "common.h"

#include <QDebug>

// Flags
#define STCI_TRANSPARENT  (1 << 0)
#define STCI_RGB          (1 << 2)
#define STCI_INDEXED      (1 << 3)
#define STCI_ZLIB_COMP    (1 << 4)
#define STCI_ETRLE_COMP   (1 << 5)

STI::STI( QByteArray sti )
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
//    the STI instance is destroyed, everything works
#if 0
static void bufferRelease(void *info)
{
    QByteArray *buffer = static_cast<QByteArray *>(info);

    delete buffer;
}
#endif

// Many thanks to Anonymous individual who documented this format
// http://ja2v113.pbworks.com/w/page/4218367/STCI%20%28STI%29%20format%20description

void STI::parseSTI( const quint8 *data_ptr, size_t len )
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

void STI::parseIndexedSTI( quint32 flags, const quint8 *data_ptr, size_t len )
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
                    qWarning() << "ROW TOO LONG (" << row_width << ">" << width << ") on image" << j;
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

void STI::parse16bitSTI( quint32 flags, const quint8 *data_ptr, size_t len )
{
    quint8  depth    = FORMAT_8(   data_ptr + 0x2C);
    quint16 height   = FORMAT_LE16(data_ptr + 0x14);
    quint16 width    = FORMAT_LE16(data_ptr + 0x16);

    quint32 red_bits = FORMAT_LE32(data_ptr + 0x18);
    quint32 grn_bits = FORMAT_LE32(data_ptr + 0x1C);
    quint32 blu_bits = FORMAT_LE32(data_ptr + 0x20);
    quint32 alp_bits = FORMAT_LE32(data_ptr + 0x24);

    // HACK to fix bug in the portraits created using Portrait Editor which
    // stores an incorrect value in the grn_bits
    if (grn_bits == 0x00000701)
        grn_bits =  0x000007e0;

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

QByteArray STI::makeSTI( QImage image, int num_images, bool true256 )
{
    switch (image.format())
    {
        case QImage::Format_Indexed8: // 8 bit palette image
            return make8BitSTI( image, num_images, true256 );

        default:
        case QImage::Format_ARGB32:   // 0xAARRGGBB
        case QImage::Format_RGB32:    // 0xffRRGGBB
            image = image.convertToFormat( QImage::Format_RGB16 );
            /* Fall through */
        case QImage::Format_RGB16:    // RGB 5-6-5
            return make16BitSTI( image );
    }
}

QByteArray STI::make16BitSTI( QImage image )
{
    QByteArray sti;

    int         width   = image.width();
    int         height  = image.height();

    // sti header - 64 bytes

    sti.append( 'S' );
    sti.append( 'T' );
    sti.append( 'C' );
    sti.append( 'I' );

    sti.append( (char)(((height*width*2) >>  0) & 0xff) ); // uncompressed pixels size
    sti.append( (char)(((height*width*2) >>  8) & 0xff) );
    sti.append( (char)(((height*width*2) >> 16) & 0xff) );
    sti.append( (char)(((height*width*2) >> 24) & 0xff) );
    sti.append( (char)(((height*width*2) >>  0) & 0xff) ); // compressed pixels size (same, ie. no compression)
    sti.append( (char)(((height*width*2) >>  8) & 0xff) );
    sti.append( (char)(((height*width*2) >> 16) & 0xff) );
    sti.append( (char)(((height*width*2) >> 24) & 0xff) );

    sti.append( 4, '\0' );

    sti.append( (char)0x04 );                       // flags - set for 16 bit no compression
    sti.append( (char)0x00 );
    sti.append( (char)0x00 );
    sti.append( (char)0x00 );

    sti.append( (char)((height >>  0) & 0xff) ); // height
    sti.append( (char)((height >>  8) & 0xff) );
    sti.append( (char)((width  >>  0) & 0xff) ); // width
    sti.append( (char)((width  >>  8) & 0xff) );

    sti.append( (char)((0xf800 >>  0) & 0xff) ); // Red mask: 00000000 00000000 11111000 00000000
    sti.append( (char)((0xf800 >>  8) & 0xff) );
    sti.append( (char)((0xf800 >> 16) & 0xff) );
    sti.append( (char)((0xf800 >> 24) & 0xff) );
    sti.append( (char)((0x07e0 >>  0) & 0xff) ); // Grn mask: 00000000 00000000 00000111 11100000
    sti.append( (char)((0x07e0 >>  8) & 0xff) );
    sti.append( (char)((0x07e0 >> 16) & 0xff) );
    sti.append( (char)((0x07e0 >> 24) & 0xff) );
    sti.append( (char)((0x001f >>  0) & 0xff) ); // Blu mask: 00000000 00000000 00000000 00011111
    sti.append( (char)((0x001f >>  8) & 0xff) );
    sti.append( (char)((0x001f >> 16) & 0xff) );
    sti.append( (char)((0x001f >> 24) & 0xff) );
    sti.append( 4, '\0' );                 // Alpha mask
    sti.append( (char)0x05 );              // Red depth
    sti.append( (char)0x06 );              // Grn depth
    sti.append( (char)0x05 );              // Blu depth
    sti.append( (char)0x00 );              // Alpha depth

    sti.append( (char)16 );                // 16 bit image
    sti.append( (char)((16 >>  0) & 0xff) );    // num_images (1) * 16
    sti.append( (char)((16 >>  8) & 0xff) );
    sti.append( (char)((16 >> 16) & 0xff) );
    sti.append( (char)((16 >> 24) & 0xff) );
    sti.append( 15, '\0' );

    if (image.width() % 2 == 0)
    {
        sti.append( (char *)image.bits(), image.sizeInBytes() );
    }
    else
    {
        // QImage screws up the conversion to 16 bit if the width of the image
        // is an odd number. It adds an unnecessary pixel to the end of every
        // line that screws up the raw dump of the content. It's almost like
        // they assumed there was 2 pixels per 1 byte, instead of 1 pixel per 2 bytes

        // We have to do it line by line now.
        int pitch = (image.width() + 1)*2;
        for (int y=0; y<image.height(); y++)
        {
            sti.append( (char *)image.bits() + y*pitch, image.width()*2 );
        }
    }

    return sti;
}

// If true256 is true then we expect a 256 colour image that has
// transparency in the 0 slot already, and correctly uses it.
// if true256 is false we expect a 255 colour image that we will
// offset by one index to create transparency in the first slot
QByteArray STI::make8BitSTI( QImage image, int num_images, bool true256 )
{
    QByteArray  sti;

    QVector<QRgb> palette = image.colorTable();
    int           width   = image.width();
    int           height  = image.height();

    // sti header - 64 bytes

    sti.append( 'S' );
    sti.append( 'T' );
    sti.append( 'C' );
    sti.append( 'I' );

    sti.append( 12, '\0' );

    sti.append( (char)0x28 );                       // flags - set for 8 bit ETRLE compression
    sti.append( (char)0x00 );
    sti.append( (char)0x00 );
    sti.append( (char)0x00 );

    sti.append( (char)((height >>  0) & 0xff) );     // height
    sti.append( (char)((height >>  8) & 0xff) );
    sti.append( (char)((width  >>  0) & 0xff) );     // width
    sti.append( (char)((width  >>  8) & 0xff) );

    sti.append( (char)((256 >>  0) & 0xff) );        // colours in palette
    sti.append( (char)((256 >>  8) & 0xff) );
    sti.append( (char)((256 >> 16) & 0xff) );
    sti.append( (char)((256 >> 24) & 0xff) );
    sti.append( (char)((num_images >>  0) & 0xff) ); // images in file
    sti.append( (char)((num_images >>  8) & 0xff) );
    sti.append( 3, (char)0x08 );

    sti.append( 11, '\0' );
    sti.append( (char)8 );                           // 8 bit image
    sti.append( 19, '\0' );

    // install the palette into the buffer - 0x300 bytes

    // Transparent colour has to be first in the image.
    // If true256 is false then we set this up ourselves,
    // and expect the image to only have 255 actual colours.
    // If true256 is true then we expect the image to already
    // have been setup correctly with transparency in first index.

    int num_cols = 256;
    if (true256 == false)
    {
        sti.append( (char)0xff );
        sti.append( (char)0xff );
        sti.append( (char)0x00 );
        num_cols = 255;
    }
    for (int k=0; k<num_cols; k++)
    {
        if (k < palette.size())
        {
            // Avoid pure black - Wizardry maps it through to transparent
            // for some broken reason too. It doesn't matter what colour
            // we have assigned for the transparent colour in position 0.
            if ((qRed(   palette[k] ) == 0) &&
                (qGreen( palette[k] ) == 0) &&
                (qBlue(  palette[k] ) == 0))
            {
                sti.append( 3, '\1' );
            }
            else
            {
                sti.append( (char)qRed(   palette[k] ) );
                sti.append( (char)qGreen( palette[k] ) );
                sti.append( (char)qBlue(  palette[k] ) );
            }
        }
        else
        {
            sti.append( (char)0xff );
            sti.append( (char)0xff );
            sti.append( (char)0xff );
        }
    }

    // image headers - 16 * num_images bytes

    sti.append( 16 * num_images, '\0' );

    // actual image - img_size bytes
    int sz_before = sti.size();
    for (int y=0; y<height; y++)
    {
        int          transp_run = 0;
        QByteArray   opaque_run;

        for (int x=0; x<width; x++)
        {
            int clr = image.pixelIndex( x, y);

            // Offset to account for the transparent (unused) colour
            // if we are processing a 255 colour image instead of 256
            if (true256 == false)
            {
                clr++;
                Q_ASSERT(clr != 0);
            }
            Q_ASSERT( clr >= 0);
            Q_ASSERT( clr < 256);

            // Have to deal with transparent pixels if true256
            // clr == 0 should be unused if true256 is true
            if (clr == 0)
            {
                if (opaque_run.size() > 0)
                {
                    sti.append( (char)opaque_run.size() );
                    sti.append( opaque_run );
                    opaque_run = QByteArray();
                }
                transp_run++;
                if (transp_run == 127)
                {
                    sti.append( (char)(0x80 | transp_run) );
                    transp_run = 0;
                }
            }
            else
            {
                if (transp_run > 0)
                {
                    sti.append( (char)(0x80 | transp_run) );
                    transp_run = 0;
                }
                // If we aren't  in true256 all the colours need offseting
                // by 1 to account for the transparent colour inserted at
                // the top of the palette
                opaque_run.append( (char)clr );
                if (opaque_run.size() == 127)
                {
                    sti.append( (char)opaque_run.size() );
                    sti.append( opaque_run );
                    opaque_run = QByteArray();
                }
            }
        }
        if (transp_run > 0)
        {
            sti.append( (char)(0x80 | transp_run) );
            transp_run = 0;
        }
        if (opaque_run.size() > 0)
        {
            sti.append( (char)opaque_run.size() );
            sti.append( opaque_run );
            opaque_run = QByteArray();
        }
        sti.append( '\0' );
    }
    int img_size = sti.size() - sz_before;

    // dummy transparent image that we'll point to n times
    sz_before = sti.size();
    for (int y=0; y<height; y++)
    {
        int x = width;

        while (x > 127)
        {
            sti.append( (char)(0x80 | 127) );
            x -= 127;
        }
        if (x > 0)
        {
            sti.append( (char)(0x80 | x) );
        }
        sti.append( '\0' );
    }
    int transp_img_size = sti.size() - sz_before;

    // fill in bytes we had to skip over previously

    quint8 *stiData = (quint8 *) sti.data();

    stiData[ 4] = (((width*height) >>  0) & 0xff);
    stiData[ 5] = (((width*height) >>  8) & 0xff);
    stiData[ 6] = (((width*height) >> 16) & 0xff);
    stiData[ 7] = (((width*height) >> 24) & 0xff);
    // Ordinarily there'd be (num_items-1)*transp_img_size
    // but we only put the one copy in
    stiData[ 8] = (((img_size + transp_img_size) >>  0) & 0xff);
    stiData[ 9] = (((img_size + transp_img_size) >>  8) & 0xff);
    stiData[10] = (((img_size + transp_img_size) >> 16) & 0xff);
    stiData[11] = (((img_size + transp_img_size) >> 24) & 0xff);

    stiData[0x340+ 0] = 0;                          // Offset of frame 0 "Real picture"
    stiData[0x340+ 1] = 0;
    stiData[0x340+ 2] = 0;
    stiData[0x340+ 3] = 0;

    stiData[0x340+ 4] = ((img_size >>  0) & 0xff);  // size of frame 0
    stiData[0x340+ 5] = ((img_size >>  8) & 0xff);
    stiData[0x340+ 6] = ((img_size >> 16) & 0xff);
    stiData[0x340+ 7] = ((img_size >> 24) & 0xff);

    stiData[0x340+ 8] = 0;
    stiData[0x340+ 9] = 0;
    stiData[0x340+10] = 0;
    stiData[0x340+11] = 0;

    stiData[0x340+12] = ((height >>  0) & 0xff);
    stiData[0x340+13] = ((height >>  8) & 0xff);
    stiData[0x340+14] = ((width  >>  0) & 0xff);
    stiData[0x340+15] = ((width  >>  8) & 0xff);

    for (int k=0; k<num_images-1; k++)
    {
        // Point at the same transparent byte data for all of these images
        stiData[0x350+ k*16+ 0] = ((img_size >>  0) & 0xff);         // offset of all other frames = same
        stiData[0x350+ k*16+ 1] = ((img_size >>  8) & 0xff);
        stiData[0x350+ k*16+ 2] = ((img_size >> 16) & 0xff);
        stiData[0x350+ k*16+ 3] = ((img_size >> 24) & 0xff);

        stiData[0x350+ k*16+ 4] = ((transp_img_size >>  0) & 0xff);  // size
        stiData[0x350+ k*16+ 5] = ((transp_img_size >>  8) & 0xff);
        stiData[0x350+ k*16+ 6] = ((transp_img_size >> 16) & 0xff);
        stiData[0x350+ k*16+ 7] = ((transp_img_size >> 24) & 0xff);

        stiData[0x350+ k*16+ 8] = 0;
        stiData[0x350+ k*16+ 9] = 0;
        stiData[0x350+ k*16+10] = 0;
        stiData[0x350+ k*16+11] = 0;

        stiData[0x350+ k*16+12] = ((height >>  0) & 0xff);
        stiData[0x350+ k*16+13] = ((height >>  8) & 0xff);
        stiData[0x350+ k*16+14] = ((width  >>  0) & 0xff);
        stiData[0x350+ k*16+15] = ((width  >>  8) & 0xff);
    }
    return sti;
}
