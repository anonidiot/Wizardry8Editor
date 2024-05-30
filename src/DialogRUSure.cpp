/*
 * Copyright (C) 2024 Anonymous Idiot
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

#include "DialogRUSure.h"

#include "SLFFile.h"
#include "common.h"
#include "main.h"

#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QDebug>

#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"

typedef enum
{
    NO_ID,

    CB_SUPPRESS_WARN,

    SIZE_WIDGET_IDS
} widget_ids;

DialogRUSure::DialogRUSure(QString message, QString name, int width, int height, QWidget *parent)
    : Dialog(parent),
    m_name(name)
{
    if (width > 420)  width  = 420;
    if (width <  20)  width  =  20;

    if (height > 270) height = 270;
    if (height <  20) height =  20;

    QPixmap bgImg     = makeDialogForm( width, height);
    QSize   bgImgSize = bgImg.size();


    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                         this ),  -1,  NULL },

        { CB_SUPPRESS_WARN,   QRect(  32, height - 47,  width - 114,  13 ),  new WCheckBox( StringList::HideThisMessage,                                   this ),  -1,  NULL           },

        { NO_ID,              QRect( width - 82, height - 47,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",              0,  true, 1.0,  this ),  -1,  SLOT(accept()) },
        { NO_ID,              QRect( width - 50, height - 47,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",              4,  true, 1.0,  this ),  -1,  SLOT(reject()) },

        { NO_ID,              QRect(  32,  32, width - 64, height - 96 ),    new WLabel(    message,  Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
    };
    // TODO: A checkbox on dialog to allow the warning to be suppressed in future

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

DialogRUSure::~DialogRUSure()
{
    if (result() == QDialog::Accepted)
    {
        bool hide_warning = qobject_cast<QAbstractButton *>(m_widgets[ CB_SUPPRESS_WARN ])->isChecked();

        // We only set if the cb is true, not false - because if the
        // setting was already true to begin with the dialog isn't even
        // shown at all and will be at the incorrect initialised state of
        // false

        if (hide_warning)
        {
            QSettings settings;

            settings.setValue( m_name, true );
        }
    }
}

int DialogRUSure::exec()
{
    QSettings settings;

    QVariant warn = settings.value( "SuppressWarningAll" );

    if (warn.isNull() || (warn.toBool() == false))
    {
        // Warnings not disabled globally but check if this warning has been individually disabled
        if (! m_name.isEmpty())
        {
            QVariant warn = settings.value( m_name );

            if (warn.isNull() || (warn.toBool() == false))
            {
                return QDialog::exec();
            }
        }
        else
        {
            return QDialog::exec();
        }
    }
    return QDialog::Accepted;
}

QPixmap DialogRUSure::makeDialogForm(int width, int height)
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg, bgFrame, bgDdl;


    // images used in dialog
    bgFrame = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_ITEMINFO.STI", 0 );
    bgImg   = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( width, height ) );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, bgImg,                      0,                     0, bgImg.width(), 145 );
    p.drawPixmap( 240,   0, bgImg,    bgImg.width() - 180,                     0,           180, 145 );
    p.drawPixmap(   0, 125, bgImg,                      0,                     0, bgImg.width(), 145 );
    p.drawPixmap( 240, 125, bgImg,    bgImg.width() - 180,                     0,           180, 145 );

    int offset_x;
    int offset_y;

#define FRAME_WIDTH 10

    if (width > bgFrame.width() - FRAME_WIDTH)
    {
        offset_x = width - (bgFrame.width() - FRAME_WIDTH);
    }
    else
    {
        offset_x = width - 2 * FRAME_WIDTH;
    }
    if (height > bgFrame.height() - FRAME_WIDTH)
    {
        offset_y = height - (bgFrame.height() - FRAME_WIDTH);
    }
    else
    {
        offset_y = height - 2 * FRAME_WIDTH;
    }

    p.drawPixmap(                   0,                    0, bgFrame,                                     0,                                      0, offset_x + FRAME_WIDTH,             FRAME_WIDTH );
    p.drawPixmap(            offset_x,                    0, bgFrame,  bgFrame.width() - (width - offset_x),                                      0,       width - offset_x,             FRAME_WIDTH );

    p.drawPixmap(                   0,                    0, bgFrame,                                     0,                                      0,            FRAME_WIDTH,  offset_y + FRAME_WIDTH );
    p.drawPixmap(                   0,             offset_y, bgFrame,                                     0, bgFrame.height() - (height - offset_y),            FRAME_WIDTH,       height - offset_y );

    p.drawPixmap(                   0, height - FRAME_WIDTH, bgFrame,                                     0,         bgFrame.height() - FRAME_WIDTH, offset_x + FRAME_WIDTH,             FRAME_WIDTH );
    p.drawPixmap(            offset_x, height - FRAME_WIDTH, bgFrame,  bgFrame.width() - (width - offset_x),         bgFrame.height() - FRAME_WIDTH,       width - offset_x,             FRAME_WIDTH );

    p.drawPixmap( width - FRAME_WIDTH,                    0, bgFrame,         bgFrame.width() - FRAME_WIDTH,                                      0,            FRAME_WIDTH,  offset_y + FRAME_WIDTH );
    p.drawPixmap( width - FRAME_WIDTH,             offset_y, bgFrame,         bgFrame.width() - FRAME_WIDTH, bgFrame.height() - (height - offset_y),            FRAME_WIDTH,       height - offset_y );

    p.end();
    return customImage;
}
