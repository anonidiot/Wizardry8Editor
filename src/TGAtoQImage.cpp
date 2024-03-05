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

#include <TGAtoQImage.h>
#include "common.h"

#include <QDebug>

// This doesn't decode every TGA format you will come across -
// just the bare minimum needed to load the variant used in the
// Wizardry 8 game
TGAtoQImage::TGAtoQImage( QByteArray tga ) :
    m_img_data()
{
    // No magic header
    parseTGA( tga );
}

// callback function for QImage to release the buffer containing the byte data
// -- this isn't currently needed because we use an image subclass to hold this
//    data and only generate the QImage when getImage() is called from it. So
//    long as the call to getImage() is followed by a shove into a pixmap BEFORE
//    the TGAtoQImage instance is destroyed, everything works
#if 0
static void bufferRelease(void *info)
{
    QByteArray *buffer = static_cast<QByteArray *>(info);

    delete buffer;
}
#endif

// Implemented based on information found at the following site:
// http://www.paulbourke.net/dataformats/tga/
void TGAtoQImage::parseTGA( QByteArray tga )
{
    const quint8 *data_ptr = (const quint8 *)tga.constData();

    quint8   idStrLen     = FORMAT_8(   data_ptr +  0);
    quint8   picTypeCode  = FORMAT_8(   data_ptr +  2);
    quint16  paletteStart = FORMAT_LE16(data_ptr +  3);
    quint16  paletteEnd   = FORMAT_LE16(data_ptr +  5);
    quint16  x_offset     = FORMAT_LE16(data_ptr +  8);
    quint16  y_offset     = FORMAT_LE16(data_ptr + 10);
    quint16  width        = FORMAT_LE16(data_ptr + 12);
    quint16  height       = FORMAT_LE16(data_ptr + 14);
    quint8   depth        = FORMAT_8(   data_ptr + 16);
    bool     flipped      = ! (FORMAT_8(   data_ptr + 17) & 0x20);

    quint32  offset = 18 + idStrLen + (paletteEnd - paletteStart);

    // It isn't too hard to adapt the RL encoding in STI to
    // work here for the RL formats too, but I just don't need it.
    switch (picTypeCode)
    {
        case  0: // no image data
        case  1: // uncompressed color-mapped image
        case  3: // uncompressed B&W image
        case  9: // runlength encoded color-mapped image
        case 10: // runlength encoded RGB image
        case 11: // compressed B&W image
        case 32: // compressed color mapped, using Huffman, Delta and RL
        case 33: // compressed color mapped, using Huffman, Delta and RL. 4-pass quadtree-type process
            qWarning() << "TGA Format unsupported";
            return;

        case  2: // uncompressed RGB image
            m_img_data   = tga.mid( offset );
            m_x_offset   = x_offset;
            m_y_offset   = y_offset;
            m_width      = width;
            m_height     = height;
            m_depth      = depth;
            m_flipped    = flipped;
            break;
    }
}
