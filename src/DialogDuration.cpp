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

#include "DialogDuration.h"
#include "SLFFile.h"
#include "STItoQImage.h"
#include "main.h"

#include <QApplication>
#include <QPainter>
#include <QSlider>
#include <QtMath>

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"

#include <QDebug>

#define USE_LOGARITHMIC_SCALE

typedef enum
{
    NO_ID,

    LBL_DURATION,

    SLDR_DURATION,

    SIZE_WIDGET_IDS
} widget_ids;

DialogDuration::DialogDuration(QString sickness, int duration, QWidget *parent)
    : Dialog(parent)
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    setWindowTitle( sickness + " " + ::getStringTable()->getString( StringList::Duration ) );

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { NO_ID,              QRect(  10,  16, 140,  12 ),    new WLabel(    StringList::Duration + StringList::APPEND_COLON, Qt::AlignLeft,  10, QFont::Thin, this ),  -1,  NULL },
        { LBL_DURATION,       QRect( 150,  16,  90,  12 ),    new WLabel(    "",                                              Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },

        { SLDR_DURATION,      QRect(  10,  40, 220,  15 ),    new QSlider( Qt::Orientation::Horizontal,                                      this ),  -1,  NULL },

        { NO_ID,              QRect( 180,  68,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(accept()) },
        { NO_ID,              QRect( 212,  68,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(reject()) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    if (QSlider *q = qobject_cast<QSlider *>(m_widgets[ SLDR_DURATION ]))
    {
        if (duration > 9999)
            duration = 9999;
        if (duration < 0)
            duration = 0;

#ifdef USE_LOGARITHMIC_SCALE
        q->setRange( 0, 133 );

        // We have to calculate an infernal Lambert expression to truly
        // reverse the equation below. NOT GOING TO HAPPEN. To avoid
        // taking the completely brute force approach of just iterating
        // through the array I'm using a polynomial approximation, which
        // is reasonably accurate over the range except the extremities, but
        // those are dealt with as exceptions (loss of accuracy is exacerbated
        // by the (int) floor operation we performed at the othe end also)

        if (duration <= 10)
        {
            q->setValue( duration );
        }
        else
        {
            double ln2 = qLn(duration) / qLn(2);

            double poly =    0.000030922386797 * ln2 * ln2 * ln2 * ln2 * ln2 * ln2 * ln2 * ln2
                          -  0.001733664724928 * ln2 * ln2 * ln2 * ln2 * ln2 * ln2 * ln2
                          +  0.039350983824911 * ln2 * ln2 * ln2 * ln2 * ln2 * ln2
                          -  0.459840959831874 * ln2 * ln2 * ln2 * ln2 * ln2
                          +  2.900637553346980 * ln2 * ln2 * ln2 * ln2
                          -  9.510494099658780 * ln2 * ln2 * ln2
                          + 15.697572686753400 * ln2 * ln2
                          -  9.162412046191100 * ln2
                          +  1.794398028352130;
            // simple round to nearest whole number (for positive integers)
            int d = (int)((poly -(int)poly > 0.5) ? (poly + 1) : (poly));

            if (d > 9999.0)
                d = 9999.0;

            q->setValue( d );
        }
#else
        q->setRange( 0, 9999 );
        q->setValue( duration );
#endif

        connect( q, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)) );
    }
    sliderChanged( duration );

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

DialogDuration::~DialogDuration()
{
}

QPixmap DialogDuration::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we just use the standard background and crop it
    QPixmap bgImg;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( bgImg.width(), 100 ) );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(), 200 );
    p.end();

    return customImage;
}

void DialogDuration::sliderChanged(int duration)
{
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ LBL_DURATION ]))
    {
#ifdef USE_LOGARITHMIC_SCALE
        duration = getDuration(); // adjust linear to logarithmic scale
#endif
        if (duration == 0)
            q->setText( tr("Off") );
        else if (duration >= 9999)
            q->setText( tr("Permanent") );
        else
        {
            // FIXME: I don't know that this is reliable.
            // 1) I'm not sure if the units change at certain values (like the spells do)
            // 2) Combat changes everything and it tends to be turn based there
            q->setText( QString( tr("%1 seconds") ).arg( duration * 10 ) );
        }
    }
}

int DialogDuration::getDuration()
{

    if (QSlider *q = qobject_cast<QSlider *>(m_widgets[ SLDR_DURATION ]))
    {
#ifdef USE_LOGARITHMIC_SCALE
        // -1 because our range is [0..9999] and can't raise 2 to any
        // power and get 0 as a result
        double d = qPow(2, ((double)q->value())/10.0) + q->value() - 1;

        if (d > 9999.0)
            d = 9999.0;

        return (int)d;
#else
        return q->value();
#endif
    }
    return -1;
}
