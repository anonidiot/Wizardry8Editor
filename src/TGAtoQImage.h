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

#ifndef TGA_TO_QIMAGE_H__
#define TGA_TO_QIMAGE_H__

#include <QByteArray>
#include <QImage>
#include <QDebug>

class TGAtoQImage
{
public:
    TGAtoQImage( QByteArray tga );

    QSize         getSize()    { return QSize( m_width, m_height ); }
    int           getWidth()   { return m_width;                  }
    int           getHeight()  { return m_height;                 }
    int           getDepth()   { return m_depth;                  }
    const QImage  getImage(int *x = NULL, int *y = NULL)
    {
        if (x)
            *x = m_x_offset;
        if (y)
            *y = m_y_offset;

        QImage qImg = QImage();

        if (m_depth == 24)
        {
qWarning() << "UNTESTED 24 bit depth";
            qImg = QImage( (quint8 *)m_img_data.data(), m_width, m_height, m_width*3, QImage::Format_RGB888);
        }
        else if (m_depth == 32)
        {
            qImg = QImage( (quint8 *)m_img_data.data(), m_width, m_height, m_width*4, QImage::Format_ARGB32);
        }

        if (! qImg.isNull() && m_flipped)
        {
            qImg = qImg.mirrored( false, true );
        }

        return qImg;
    }

private:
    void parseTGA( QByteArray tga );

    QByteArray         m_img_data;
    quint16            m_x_offset;
    quint16            m_y_offset;
    quint16            m_width;
    quint16            m_height;
    quint8             m_depth;
    bool               m_flipped;
};

#endif /* TGA_TO_QIMAGE_H__ */
