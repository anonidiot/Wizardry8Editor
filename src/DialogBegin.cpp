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

#include "DialogBegin.h"
#include "SLFFile.h"
#include "STItoQImage.h"
#include "main.h"

#include <QApplication>
#include <QButtonGroup>
#include <QPainter>
#include <QPixmap>

#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    MODE_START,
    CB_NEW_FILE = MODE_START,
    CB_OPEN_FILE,
    MODE_END,

    SIZE_WIDGET_IDS
} widget_ids;


DialogBegin::DialogBegin(QWidget *parent)
    : Dialog(parent)
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { CB_NEW_FILE,        QRect(  10,  16, 230,  13 ),    new WCheckBox( StringList::NewSaveFile,                                        this ),  -1,  NULL },
        { CB_OPEN_FILE,       QRect(  10,  33, 230,  13 ),    new WCheckBox( StringList::OpenExistingSave,                                   this ),  -1,  NULL },

        { NO_ID,              QRect( 180,  68,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(accept()) },
        { NO_ID,              QRect( 212,  68,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(reject()) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    m_action_cb_group = new QButtonGroup(this);
    for (int k=MODE_START; k < MODE_END; k++)
    {
        m_action_cb_group->addButton( qobject_cast<QAbstractButton *>(m_widgets[ k ]), k );
    }
    // Set Open Existing as default
    if (QCheckBox *q = qobject_cast<QCheckBox *>(m_widgets[ CB_OPEN_FILE ]))
    {
        q->setCheckState( Qt::Checked );
    }

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

DialogBegin::~DialogBegin()
{
    delete m_action_cb_group;
}

QPixmap DialogBegin::makeDialogForm()
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

int DialogBegin::getAction()
{
    switch (m_action_cb_group->checkedId())
    {
        case CB_NEW_FILE:  return Action::NewFile;
        case CB_OPEN_FILE: return Action::OpenFile;
    }
    return -1;
}
