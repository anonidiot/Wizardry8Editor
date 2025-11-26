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

#include <QDebug>
#include <QByteArray>
#include <QMouseEvent>
#include <QPainter>
#include "WStatBar.h"

#include "SLFFile.h"
#include "STI.h"

// Keep these around globally to save constantly loading them, because they're
// used a lot
QPixmap *s_redBar  = NULL;
QPixmap *s_blueBar = NULL;
QPixmap *s_cyanBar = NULL;

WStatBar::WStatBar(QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_showCurrent(false),
    m_mouseInLabel(false)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    if (!s_redBar ||
        !s_blueBar ||
        !s_cyanBar)
    {
        loadPixmaps();
    }
}

WStatBar::WStatBar(bool showCurrent, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_showCurrent(showCurrent),
    m_fontSize(pointSize),
    m_fontWeight(weight),
    m_alignment(alignment),
    m_mouseInLabel(false)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    if (!s_redBar ||
        !s_blueBar ||
        !s_cyanBar)
    {
        loadPixmaps();
    }
}

WStatBar::~WStatBar()
{
}

void WStatBar::setFontSize(int pointSize, int weight)
{
    m_fontSize   = pointSize;
    m_fontWeight = weight;
}

void WStatBar::loadPixmaps()
{
    SLFFile imgs( "REVIEW/REVIEWSLIDERBAR.STI" );
    if (imgs.open(QFile::ReadOnly))
    {
        QByteArray  stiData;

        imgs.readAll( stiData );
        STI stiImages( stiData );

        s_blueBar = new QPixmap( QPixmap::fromImage( stiImages.getImage( 0 )) );
        s_cyanBar = new QPixmap( QPixmap::fromImage( stiImages.getImage( 1 )) );
        s_redBar  = new QPixmap( QPixmap::fromImage( stiImages.getImage( 2 )) );

        imgs.close();
    }
}

void WStatBar::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use it to
        // reposition _and_ resize the widget
        move(  (double)m_rect.x()     * scale, (double)m_rect.y()      * scale );
        resize((double)m_rect.width() * scale, (double)m_rect.height() * scale );
    }

    Wizardry8Scalable::setScale(scale);
}

void WStatBar::setValue( int base, int current, int range )
{
    m_base    = base;
    m_current = current;
    m_range   = range;

    repaint();
}

void WStatBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Make the upscaling look nicer
    painter.setRenderHint( QPainter::Antialiasing,          true );
    painter.setRenderHint( QPainter::TextAntialiasing,      true );
    painter.setRenderHint( QPainter::SmoothPixmapTransform, true );

    double units = (double)rect().width() / (double)m_range;

    if (m_base > m_current)
    {
        // Cyan bar won't be visible, blue bar sits on red
        // Red bar shows base
        // Blue bar shows current
        double red_length  = units * m_base;
        double blue_length = units * m_current;

        if ((int)red_length)
        {
            QRectF r = rect();
            r.setWidth( red_length );
            r.moveTop( 2*m_scale );
            r.setHeight( rect().height() - 4*m_scale );
            painter.drawPixmap( r, *s_redBar,  s_redBar->rect() );
        }
        if ((int)blue_length)
        {
            QRectF r = rect();
            r.setWidth( blue_length );
            r.moveTop( 2*m_scale );
            r.setHeight( rect().height() - 4*m_scale );
            painter.drawPixmap( r, *s_blueBar, s_blueBar->rect() );
        }
    }
    else
    {
        // Red bar won't be visible, blue bar sits on cyan
        // Blue bar shows base
        // Cyan bar shows current
        double blue_length = units * m_base;
        double cyan_length = units * m_current;

        if ((int)cyan_length)
        {
            QRectF r = rect();
            r.setWidth( cyan_length );
            r.moveTop( 2*m_scale );
            r.setHeight( rect().height() - 4*m_scale );
            painter.drawPixmap( r, *s_cyanBar, s_cyanBar->rect() );
        }
        if ((int)blue_length)
        {
            QRectF r = rect();
            r.setWidth( blue_length );
            r.moveTop( 2*m_scale );
            r.setHeight( rect().height() - 4*m_scale );
            painter.drawPixmap( r, *s_blueBar, s_blueBar->rect() );
        }
    }

    if (m_showCurrent)
    {
        painter.setPen( QColor::fromRgb( 0xee, 0xee, 0xee ) );
        painter.setFont(QFont("Wizardry", m_fontSize * m_scale, m_fontWeight));
        painter.drawText( rect(), m_alignment, QString::number( m_current ) );
    }
}

void WStatBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked( true );
    }
}

void WStatBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked( false );
    }
}

void WStatBar::enterEvent(QEvent *)
{
    if (m_mouseInLabel != true)
    {
        m_mouseInLabel = true;
        emit mouseOver( m_mouseInLabel );
    }
}

void WStatBar::leaveEvent(QEvent *)
{
    if (m_mouseInLabel != false)
    {
        m_mouseInLabel = false;
        emit mouseOver( m_mouseInLabel );
    }
}
