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

#include "DialogAbout.h"
#include "SLFFile.h"
#include "STItoQImage.h"
#include "main.h"

#include <QApplication>
#include <QPainter>

#include "WButton.h"
#include "WLabel.h"

#include <QDebug>

#define STRFY(X) #X
#define STR(X) STRFY(X)
#define TITLE \
    "Wizardry Editor " STR(VERSION)

#define COPYRIGHT1 \
    "All code in this application is covered by the BSD 2-clause license:\n\n"                   \
    "Copyright 2022 Anonymous Idiot\n"                                                           \
    "All rights reserved\n\n"                                                                    \
                                                                                                 \
    "The patch files used to create the Wizardry font were created with\n"                       \
    "Colin Percival's BSDiff utility (http://www.daemonology.net/bsdiff/)\n"                     \
    "- also covered by BSD license. The extraction process embedded into\n"                      \
    "this application is based on his companion bspatch utility for applying\n"                  \
    "the patch.\n\n"                                                                             \
    "Copyright 2003-2005 Colin Percival\n"                                                       \
    "All rights reserved\n\n"                                                                    \
                                                                                                 \
    "Redistribution and use in source and binary forms, with or without\n"                       \
    "modification, are permitted providing that the following conditions \n"                     \
    "are met:\n"                                                                                 \
    "1. Redistributions of source code must retain the above copyright\n"                        \
    "   notice, this list of conditions and the following disclaimer.\n"                         \
    "2. Redistributions in binary form must reproduce the above copyright\n"                     \
    "   notice, this list of conditions and the following disclaimer in the\n"                   \
    "   documentation and/or other materials provided with the distribution.\n" 
#define COPYRIGHT2 \
    "THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"                     \
    "IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"                           \
    "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"                       \
    "ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY\n"                          \
    "DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"                       \
    "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE\n"                        \
    "GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS\n"                            \
    "INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"                             \
    "WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"                                \
    "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"                       \
    "SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"                           \
    "Pinata icon taken from Emojipedia:\n  https://emojipedia.org/emojipedia/13.0/pinata/\n"

#define COLOR_STEPS 50

typedef enum
{
    NO_ID,

    SIZE_WIDGET_IDS
} widget_ids;

DialogAbout::DialogAbout(QWidget *parent)
    : Dialog(parent),
    m_pos(-472),
    m_colorSteps(0)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect( 450, 490.0/m_scale,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true,  1.0,  this ),  -1,  SLOT(close()) },
        { NO_ID,              QRect( 10,  495.0/m_scale, 250,  16 ),    new WLabel( tr("Download source code"),        Qt::AlignLeft, 12, QFont::Thin, this ),  -1,  SLOT(downloadSourceCode(bool)) },

    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    QPixmap p = SLFFile::getPixmapFromSlf( "LEVEL LOAD/DEATHSCREEN.STI", 0 );

    m_width   = p.width();
    m_height  = p.height();

    splitYUV( p, m_y1, m_u1, m_v1 );

    QFile f_u2( ":/death.u4" );
    QFile f_v2( ":/death.v4" );

    if (f_u2.open(QFile::ReadOnly))
    {
        m_u2 = f_u2.readAll();
        f_u2.close();
    }
    if (f_v2.open(QFile::ReadOnly))
    {
        m_v2 = f_v2.readAll();
        f_v2.close();
    }

    resize( 640, 530 );
    show();

    connect(&m_animateText, SIGNAL(timeout()), this, SLOT(coloriseImage()));
    m_animateText.start(60);
}

DialogAbout::~DialogAbout()
{
    m_animateText.stop();
}

void DialogAbout::splitYUV( QPixmap sti_pix, QByteArray &y, QByteArray &u, QByteArray &v )
{
    QImage sti = sti_pix.toImage();

    int width  = sti.width();
    int height = sti.height();

    y = QByteArray( width * height, 0 );
    u = QByteArray( width * height, 0 );
    v = QByteArray( width * height, 0 );

    for (int h = 0; h < height; h++)
    {
        QRgb *line = (QRgb *) sti.scanLine(h);

        for (int x = 0; x< width; x++)
        {
           double r = qRed(   line[x] );
           double g = qGreen( line[x] );
           double b = qBlue(  line[x] );

           y[h*width + x] =   0.257 * r + 0.504 * g + 0.098 * b +  16;
           u[h*width + x] = -0.148 * r - 0.291 * g + 0.439 * b + 128;
           v[h*width + x] =  0.439 * r - 0.368 * g - 0.071 * b + 128;
        }
    }
}

void DialogAbout::downloadSourceCode(bool down)
{
    if (down)
    {
        QString save_file = ::getSaveFileName( this,
                                               TITLE,
                                               QDir::homePath() + QDir::separator() + TITLE + ".zip",
                                               tr("Zip Archives (*.zip)"));
        if (! save_file.isEmpty())
        {
            QFile::copy( ":/Wizardry8Editor.zip", save_file );
        }
    }
}

void DialogAbout::coloriseImage()
{
    QRgb rgb[m_width * m_height];

    for (int h=0; h<m_height; h++)
    {
        for (int x=0; x<m_width; x++)
        {
            int pos = h*m_width + x;

            int y  = (quint8)(m_y1[pos]);
            int u1 = (quint8)(m_u1[pos]) - 128;
            int v1 = (quint8)(m_v1[pos]) - 128;

            int u2 = (quint8)(m_u2[h/2*m_width/2 + x/2]) - 128;
            int v2 = (quint8)(m_v2[h/2*m_width/2 + x/2]) - 128;

            int u  = (1.0 - (double)m_colorSteps / COLOR_STEPS) * u1 +
                     (      (double)m_colorSteps / COLOR_STEPS) * u2;
            int v  = (1.0 - (double)m_colorSteps / COLOR_STEPS) * v1 +
                     (      (double)m_colorSteps / COLOR_STEPS) * v2;

            rgb[pos] = qRgb( (int)(y + 1.370705 * v),
                             (int)(y - 0.337633 * u - 0.698001 * v),
                             (int)(y + 1.732446 * u) );

            // get rid of the bright red pixels
            if ((qRed(   rgb[pos] ) > 0xf0) &&
                (qGreen( rgb[pos] ) < 0x20) &&
                (qBlue(  rgb[pos] ) < 0x20))
            {
                rgb[pos] = 0;
            }
        }
    }
    QImage im( (quint8 *)rgb, m_width, m_height, QImage::Format_ARGB32 );

    m_Image = QPixmap::fromImage( im );

    repaint();

    m_colorSteps++;
    if (m_colorSteps > COLOR_STEPS)
    {
        disconnect(&m_animateText, SIGNAL(timeout()), this, SLOT(coloriseImage()));
        connect(&m_animateText, SIGNAL(timeout()), this, SLOT(scrollText()));
    }
}

void DialogAbout::scrollText()
{
    m_pos += 3;
    if (m_pos > 640)
        m_pos = -460;
    repaint();
}

void DialogAbout::paintEvent(QPaintEvent *evt)
{
    QPainter p(this);

    (void)evt;

    p.drawPixmap( 0, 0, m_Image );

    p.translate( 0, -m_pos );
    p.setClipRect( QRect( 10, 10+m_pos, 620, 460 ) );
    p.setClipping( true );
    p.setFont(QFont("Wizardry", 16 * m_scale, QFont::Bold));
    p.setPen( QColor( 255, 255, 255 ) );
    p.drawText( QRect( 10,  10, 620, 400), Qt::AlignLeft, tr(TITLE) );
    p.setFont(QFont("Wizardry", 10 * m_scale, QFont::Thin));
    p.drawText( QRect( 10,  45, 620, 400), Qt::AlignLeft, tr(COPYRIGHT1) );
    p.drawText( QRect( 10, 455, 620, 400), Qt::AlignLeft, tr(COPYRIGHT2) );
    p.setClipping( false );
}
