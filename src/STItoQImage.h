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

#ifndef STI_TO_QIMAGE_H__
#define STI_TO_QIMAGE_H__

#include <QByteArray>
#include <QImage>

class image
{
public:
    image() {}

    QImage getImage()
    {
        if (img_format == QImage::Format_RGB16)
        {
            // 16 bit STI images are NOT 32-bit aligned.
            // nonaligned 16 bit width = 2 bytes per pixel in width = width*2
            return QImage( (quint8 *)img_data.data(), width, height, width*2, img_format);//, bufferRelease, img_raw);
        }
        // Palette indexed images get unpacked to ARGB32, which by
        // its nature is 32 bit aligned, so we can use the simpler constructor
        return QImage( (quint8 *)img_data.data(), width, height, img_format);//, bufferRelease, img_raw);
    }

    image(const image &other)
    {
        this->img_data   = other.img_data;
        this->img_format = other.img_format;
        this->x_offset   = other.x_offset;
        this->y_offset   = other.y_offset;
        this->width      = other.width;
        this->height     = other.height;
        this->depth      = other.depth;
    }

    image &operator=(const image &other)
    {
        this->img_data   = other.img_data;
        this->img_format = other.img_format;
        this->x_offset   = other.x_offset;
        this->y_offset   = other.y_offset;
        this->width      = other.width;
        this->height     = other.height;
        this->depth      = other.depth;
        return *this;
    }

public:
    QByteArray         img_data;
    QImage::Format     img_format;
    quint16            x_offset;
    quint16            y_offset;
    quint16            width;
    quint16            height;
    quint8             depth;
};

class STItoQImage
{
public:
    STItoQImage( QByteArray sti );

    int           getNumImages()        { return m_image.size();                                       }
    QSize         getSize(int image)    { return QSize( m_image[image].width, m_image[image].height ); }
    int           getWidth(int image)   { return m_image[image].width;                                 }
    int           getHeight(int image)  { return m_image[image].height;                                }
    int           getDepth(int image)   { return m_image[image].depth;                                 }
    const QImage  getImage(int image, int *x = NULL, int *y = NULL)
    {
        if (image >= m_image.size())
            return QImage();

        if (x)
            *x = m_image[image].x_offset;
        if (y)
            *y = m_image[image].y_offset;

        return m_image[image].getImage();
    }

private:
    void parseSTI( const quint8 *data_ptr, size_t len );
    void parse16bitSTI( quint32 flags, const quint8 *data_ptr, size_t len );
    void parseIndexedSTI( quint32 flags, const quint8 *data_ptr, size_t len );

    QList<image>  m_image;
};

#endif /* STI_TO_QIMAGE_H__ */
