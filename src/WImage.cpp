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

#include <QByteArray>
#include <QMouseEvent>
#include <QPainter>

#include "WImage.h"

#include "SLFFile.h"
#include "STItoQImage.h"
#include "TGAtoQImage.h"

WImage::WImage(QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_xScale(1.0),
    m_yScale(1.0),
    m_mouseInLabel(false),
    m_stiImages(NULL),
    m_crop()
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }
}

WImage::WImage(QString file, int image_idx, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_xScale(1.0),
    m_yScale(1.0),
    m_mouseInLabel(false),
    m_stiImages(NULL),
    m_crop()
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    if (file.endsWith(".PNG", Qt::CaseInsensitive))
    {
        setImageFile( file, "PNG" );
    }
    else if (file.endsWith(".TGA", Qt::CaseInsensitive))
    {
        setTargaFile( file );
    }
    else
    {
        setStiFile( file, image_idx );
    }
}

WImage::WImage(QString file, int image_idx, int crop_left, int crop_top, int crop_width, int crop_height, double extraScale, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_xScale(1.0),
    m_yScale(1.0),
    m_mouseInLabel(false),
    m_stiImages(NULL)
{
    // Base class variables can't be initialized in the defaults bit above
    m_extraScale = extraScale;

    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() * extraScale );
    }

    if (file.endsWith(".PNG", Qt::CaseInsensitive))
    {
        setImageFile( file, "PNG" );
    }
    else if (file.endsWith(".TGA", Qt::CaseInsensitive))
    {
        setTargaFile( file );
    }
    else
    {
        setStiFile( file, image_idx );
    }

    m_crop = QRect( crop_left, crop_top, crop_width, crop_height );
}

WImage::WImage(QPixmap &pix, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_xScale(1.0),
    m_yScale(1.0),
    m_mouseInLabel(false),
    m_stiImages(NULL),
    m_crop()
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    this->setPixmap( pix );
}

WImage::WImage(int width, int height, QColor c, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_xScale(1.0),
    m_yScale(1.0),
    m_mouseInLabel(false),
    m_stiImages(NULL),
    m_crop()
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    this->setFill( width, height, c );
}

WImage::~WImage()
{
    if (m_stiImages)
        delete m_stiImages;
}

void WImage::setFill(int width, int height, QColor c)
{
    QPixmap pixmap = QPixmap( width, height );

    pixmap.fill( c );

    setPixmap(pixmap);
}

void WImage::setStiFile(QString sti_file, int image_idx, bool keep)
{
    SLFFile imgs( sti_file );
    if (imgs.open(QFile::ReadOnly))
    {
        if (m_stiImages)
            delete m_stiImages;

        m_stiData   = imgs.readAll();
        m_stiImages = new STItoQImage( m_stiData );
        m_frameIdx  = image_idx;

        this->setPixmap( QPixmap::fromImage( m_stiImages->getImage( image_idx )) );
        if (! keep)
        {
            delete m_stiImages;
            m_stiImages = NULL;
            m_stiData   = QByteArray();
        }

        imgs.close();
    }
}

void WImage::setTargaFile(QString tga_file)
{
    SLFFile imgs( tga_file );
    if (imgs.open(QFile::ReadOnly))
    {
        if (m_stiImages)
            delete m_stiImages;
        m_stiImages = NULL;
        m_stiData   = QByteArray();
        m_frameIdx  = 0;

        TGAtoQImage tgaImage( imgs.readAll() );

        this->setPixmap( QPixmap::fromImage( tgaImage.getImage()) );

        imgs.close();
    }
}

// Supported filetypes: see - QImageReader::supportedImageFormats()
// (BMP, GIF, JPEG, PNG, PBM, PGM, PPM, XBM, XPM)
void WImage::setImageFile(QString img_file, const char *extension)
{
    SLFFile imgs( img_file );
    if (imgs.open(QFile::ReadOnly))
    {
        QImage     m;

        m.loadFromData( imgs.readAll(), extension );
        this->setPixmap( QPixmap::fromImage( m ) );

        imgs.close();

        if (m_stiImages)
            delete m_stiImages;
        m_stiData = QByteArray();
    }
}

void WImage::animate()
{
    if (m_stiImages)
    {
        int maxFrame = m_stiImages->getNumImages();

        m_frameIdx++;
        if (m_frameIdx >= maxFrame)
            m_frameIdx = 0;

        this->setPixmap( QPixmap::fromImage( m_stiImages->getImage( m_frameIdx )) );
    }
}

void WImage::setPixmap(QPixmap pixmap)
{
    m_pixmap = pixmap;
    resize( m_scale * m_pixmap.width(), m_scale * m_pixmap.height() );
    update();
}

QSize WImage::getPixmapSize() const
{
    return m_pixmap.size();
}

void WImage::setLengthRestrict(Qt::Alignment anchor, double widthPercent, double heightPercent)
{
    m_anchor = anchor;
    m_xScale = widthPercent;
    m_yScale = heightPercent;
}

void WImage::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use the left
        // and top values to set the position relative to the
        // parent widget - width and height always comes from
        // the pixmap.
        move( (double)m_rect.x() * scale, (double)m_rect.y() * scale );
    }
    if (m_crop.isNull())
        resize( scale * m_pixmap.width() * m_extraScale, scale * m_pixmap.height() * m_extraScale );
    else
        resize( scale * m_crop.width() * m_extraScale, scale * m_crop.height() * m_extraScale );

    Wizardry8Scalable::setScale(scale);
}

void WImage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Make the upscaling look nicer
    painter.setRenderHint( QPainter::Antialiasing,          true );
    painter.setRenderHint( QPainter::TextAntialiasing,      true );
    painter.setRenderHint( QPainter::SmoothPixmapTransform, true );

    QPixmap pic = m_pixmap;

    // Grayscale disabled images
    if (! this->isEnabled())
    {
        QImage s = pic.toImage();
        pic = QPixmap::fromImage( s.convertToFormat( QImage::Format_Grayscale8 ) );
    }

    if ((m_xScale == 1.0) && (m_yScale == 1.0) && (m_extraScale == 1.0))
    {
        if (m_crop.isNull())
        {
            painter.drawPixmap( rect(), pic);
        }
        else
        {
            painter.drawPixmap( rect(), pic, m_crop);
        }
    }
    else
    {
        int x_pos = rect().x();
        int y_pos = rect().y();

        if (m_anchor & Qt::AlignHCenter)
            x_pos += rect().width() * (1.0 - m_xScale)/2;

        if (m_anchor & Qt::AlignRight)
            x_pos += rect().width() * (1.0 - m_xScale);

        if (m_anchor & Qt::AlignVCenter)
            y_pos += rect().height() * (1.0 - m_yScale)/2;

        if (m_anchor & Qt::AlignBottom)
            y_pos += rect().height() * (1.0 - m_yScale);

        painter.drawPixmap( x_pos, y_pos, rect().width()*m_xScale * m_extraScale, rect().height()*m_yScale * m_extraScale, pic);
    }
}

void WImage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked( true );
    }
}

void WImage::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked( false );
    }
}

void WImage::enterEvent(QEvent *)
{
    if (m_mouseInLabel != true)
    {
        m_mouseInLabel = true;
        emit mouseOver( m_mouseInLabel );
    }
}

void WImage::leaveEvent(QEvent *)
{
    if (m_mouseInLabel != false)
    {
        m_mouseInLabel = false;
        emit mouseOver( m_mouseInLabel );
    }
}
